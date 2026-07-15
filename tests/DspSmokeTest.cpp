#include "../src/dsp/AcidEngine.h"
#include "../src/dsp/EngineDescriptor.h"
#include "../src/dsp/Modulation.h"
#include "../src/model/PatternBank.h"

#include <juce_core/juce_core.h>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

juce::File findPresetDirectory()
{
    auto dir = juce::File::getCurrentWorkingDirectory();
    for (int i = 0; i < 8; ++i)
    {
        auto candidate = dir.getChildFile ("presets").getChildFile ("factory");
        if (candidate.exists())
            return candidate;
        dir = dir.getParentDirectory();
    }

    return {};
}

bool validateFactoryPresets()
{
    const auto dir = findPresetDirectory();
    if (! dir.exists())
    {
        std::cerr << "Factory preset directory not found\n";
        return false;
    }

    juce::Array<juce::File> files;
    dir.findChildFiles (files, juce::File::findFiles, false, "*.acidpreset.json");
    if (files.size() < 16)
    {
        std::cerr << "Expected at least 16 factory presets\n";
        return false;
    }

    for (const auto& file : files)
    {
        const auto parsed = juce::JSON::parse (file);
        auto* root = parsed.getDynamicObject();
        if (root == nullptr || root->getProperty ("format").toString() != "AcidLab303Preset")
        {
            std::cerr << "Invalid preset format: " << file.getFileName().toStdString() << "\n";
            return false;
        }

        if (root->getProperty ("meta").getDynamicObject() == nullptr
            || root->getProperty ("parameters").getDynamicObject() == nullptr)
        {
            std::cerr << "Preset missing meta or parameters: " << file.getFileName().toStdString() << "\n";
            return false;
        }
    }

    return true;
}

bool validatePatternBankRoundTrip()
{
    acidlab::PatternBank bank;
    bank.initializeFactory();
    acidlab::PatternBankMetadata metadata;
    metadata.name = "Smoke";
    metadata.author = "Test";
    metadata.description = "Round trip";
    const auto root = bank.toPatternBankVar (metadata);

    acidlab::PatternBank restored;
    if (! restored.fromPatternBankVar (root))
    {
        std::cerr << "Pattern bank round trip failed to parse\n";
        return false;
    }

    const auto step = restored.getStep (0, 0);
    if (step.note != bank.getStep (0, 0).note || step.gate != bank.getStep (0, 0).gate)
    {
        std::cerr << "Pattern bank round trip changed step data\n";
        return false;
    }

    return true;
}

bool validateEngineDescriptors()
{
    const auto& descriptors = acidlab::getEngineDescriptors();
    if (descriptors.size() != acidlab::engineCount)
    {
        std::cerr << "Unexpected engine descriptor count\n";
        return false;
    }

    constexpr std::array<float, 5> legacyValues { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    for (size_t index = 0; index < legacyValues.size(); ++index)
    {
        const auto migrated = acidlab::migrateLegacyEngineValue (legacyValues[index]);
        const auto expected = acidlab::normalisedEngineValue (static_cast<int> (index));
        if (std::abs (migrated - expected) > 0.0001f)
        {
            std::cerr << "Legacy engine migration changed mode " << index << "\n";
            return false;
        }
    }

    return true;
}

bool validateModulationSources()
{
    acidlab::ModulationEngine engine;
    engine.prepare (48000.0);
    acidlab::ModulationParameters parameters;
    parameters.lfos[0].rateHz = 2.0f;
    parameters.lfos[0].depth = 1.0f;
    parameters.lfos[0].shape = 0;
    parameters.lfos[1].sync = true;
    parameters.lfos[1].division = 8;
    parameters.lfos[1].depth = 1.0f;
    parameters.envelopes[0].attack = 0.005f;
    parameters.envelopes[0].decay = 0.030f;
    parameters.envelopes[0].sustain = 0.45f;
    parameters.envelopes[0].release = 0.020f;
    parameters.envelopes[0].amount = 1.0f;

    engine.noteOn();
    float peak = 0.0f;
    for (int i = 0; i < 6000; ++i)
    {
        const auto values = engine.next (parameters, 120.0);
        for (const auto value : values)
        {
            if (! std::isfinite (value) || std::abs (value) > 1.001f)
            {
                std::cerr << "Modulation source out of bounds\n";
                return false;
            }
        }
        peak = std::max (peak, values[3]);
    }

    if (peak < 0.10f)
    {
        std::cerr << "Envelope did not rise after note on\n";
        return false;
    }

    engine.noteOff();
    float released = 1.0f;
    for (int i = 0; i < 6000; ++i)
        released = engine.next (parameters, 120.0)[3];
    if (released > 0.05f)
    {
        std::cerr << "Envelope did not release after note off\n";
        return false;
    }

    return true;
}

double renderEnergy (acidlab::Parameters parameters, const std::array<acidlab::Step, acidlab::maxSteps>& pattern);

bool validateMatrixRouting(const std::array<acidlab::Step, acidlab::maxSteps>& pattern)
{
    acidlab::Parameters parameters;
    parameters.bpm = 126.0;
    parameters.cutoff = 900.0f;
    parameters.resonance = 0.76f;
    parameters.drive = 0.58f;
    parameters.distMode = 7;
    parameters.modulation.lfos[0].rateHz = 3.0f;
    parameters.modulation.lfos[0].depth = 1.0f;
    parameters.modulation.envelopes[0].attack = 0.01f;
    parameters.modulation.envelopes[0].amount = 1.0f;
    parameters.modulation.slots[0] = { 0, acidlab::ModulationTarget::cutoff, 0.8f };
    parameters.modulation.slots[1] = { 3, acidlab::ModulationTarget::engineCharacterA, -0.65f };
    parameters.modulation.slots[2] = { 0, acidlab::ModulationTarget::distMix, 0.5f };

    const auto energy = renderEnergy (parameters, pattern);
    if (energy < 1.0)
    {
        std::cerr << "Matrix routing produced invalid output\n";
        return false;
    }

    return true;
}

double renderEnergy (acidlab::Parameters parameters, const std::array<acidlab::Step, acidlab::maxSteps>& pattern)
{
    acidlab::AcidEngine engine;
    engine.prepare (44100.0);

    constexpr int blockSize = 512;
    constexpr int blocks = 64;
    std::vector<float> left (blockSize, 0.0f);
    std::vector<float> right (blockSize, 0.0f);

    double energy = 0.0;
    for (int block = 0; block < blocks; ++block)
    {
        std::fill (left.begin(), left.end(), 0.0f);
        std::fill (right.begin(), right.end(), 0.0f);
        engine.render (left.data(), right.data(), blockSize, parameters, pattern, true);

        for (int i = 0; i < blockSize; ++i)
        {
            if (! std::isfinite (left[static_cast<size_t> (i)])
                || ! std::isfinite (right[static_cast<size_t> (i)]))
            {
                std::cerr << "Non-finite sample at block " << block << ", sample " << i << "\n";
                return -1.0;
            }

            if (std::abs (left[static_cast<size_t> (i)]) > 1.01f
                || std::abs (right[static_cast<size_t> (i)]) > 1.01f)
            {
                std::cerr << "Sample exceeded expected clipped range\n";
                return -1.0;
            }

            energy += std::abs (left[static_cast<size_t> (i)]);
        }
    }

    return energy;
}

int main()
{
    acidlab::Parameters parameters;
    parameters.bpm = 128.0;
    parameters.cutoff = 1200.0f;
    parameters.resonance = 0.82f;
    parameters.drive = 0.72f;
    parameters.patternLength = 16;
    parameters.fxDelayOn = true;
    parameters.fxDelayMix = 0.18f;
    parameters.fxModOn = true;
    parameters.fxModDepth = 0.25f;
    parameters.fxReverbOn = true;
    parameters.fxReverbMix = 0.12f;

    std::array<acidlab::Step, acidlab::maxSteps> pattern {};
    for (int i = 0; i < acidlab::maxSteps; ++i)
    {
        pattern[static_cast<size_t> (i)].note = i % 12;
        pattern[static_cast<size_t> (i)].octave = i % 5 == 0 ? 1 : 0;
        pattern[static_cast<size_t> (i)].accent = i % 4 == 0;
        pattern[static_cast<size_t> (i)].slide = i % 7 == 0;
        pattern[static_cast<size_t> (i)].gate = true;
        pattern[static_cast<size_t> (i)].rest = i % 11 == 0;
    }

    auto energy = renderEnergy (parameters, pattern);
    if (energy < 1.0)
    {
        std::cerr << "Engine produced near-silent output\n";
        return 1;
    }

    for (int filterModel = 0; filterModel < 2; ++filterModel)
    {
        for (int oversampling = 0; oversampling < 3; ++oversampling)
        {
            for (int mode = 0; mode < acidlab::engineCount; ++mode)
            {
                for (const auto controlValue : { 0.0f, 1.0f })
                {
                    auto p = parameters;
                    p.filterModel = filterModel;
                    p.oversampling = oversampling;
                    p.distMode = mode;
                    p.engineControls[static_cast<size_t> (mode)] = { controlValue, controlValue, controlValue };
                    p.cutoff = 7800.0f;
                    p.resonance = 0.96f;
                    const auto testEnergy = renderEnergy (p, pattern);
                    if (testEnergy < 1.0)
                    {
                        std::cerr << "Invalid energy for filter " << filterModel
                                  << ", oversampling " << oversampling
                                  << ", mode " << mode
                                  << ", control " << controlValue << "\n";
                        return 1;
                    }
                }
            }
        }
    }

    if (! validateFactoryPresets())
        return 1;
    if (! validatePatternBankRoundTrip())
        return 1;
    if (! validateEngineDescriptors())
        return 1;
    if (! validateModulationSources())
        return 1;
    if (! validateMatrixRouting (pattern))
        return 1;

    std::cout << "AcidLab303 DSP smoke test passed\n";
    return 0;
}
