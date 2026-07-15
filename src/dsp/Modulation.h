#pragma once

#include <array>

namespace acidlab
{
constexpr int lfoCount = 3;
constexpr int envelopeCount = 2;
constexpr int modulationSlotCount = 6;
constexpr int modulationSourceCount = lfoCount + envelopeCount;

enum class ModulationTarget
{
    cutoff, resonance, envMod, decay, accent, slide, pulseWidth, drive, distTone, distMix,
    delayMix, delayTime, fxModDepth, reverbMix, fxTone, noise, drift, volume,
    engineCharacterA, engineCharacterB, engineCharacterC, count
};

constexpr int modulationTargetCount = static_cast<int> (ModulationTarget::count);

struct LfoParameters
{
    float rateHz = 1.0f;
    bool sync = false;
    int division = 8;
    int shape = 0;
    float phase = 0.0f;
    float depth = 0.0f;
};

struct AdsrParameters
{
    float attack = 0.02f;
    float decay = 0.20f;
    float sustain = 0.70f;
    float release = 0.30f;
    float amount = 0.0f;
};

struct ModulationSlot
{
    int source = 0;
    ModulationTarget target = ModulationTarget::cutoff;
    float amount = 0.0f;
};

struct ModulationParameters
{
    std::array<LfoParameters, lfoCount> lfos {};
    std::array<AdsrParameters, envelopeCount> envelopes {};
    std::array<ModulationSlot, modulationSlotCount> slots {};
};

class ModulationEngine
{
public:
    void prepare (double newSampleRate);
    void reset();
    void noteOn();
    void noteOff();
    std::array<float, modulationSourceCount> next (const ModulationParameters&, double bpm);

private:
    struct EnvelopeState
    {
        enum class Stage { idle, attack, decay, sustain, release };
        float value = 0.0f;
        Stage stage = Stage::idle;
    };

    float nextLfo (int index, const LfoParameters&, double bpm);
    float nextEnvelope (int index, const AdsrParameters&);
    static float syncRateHz (int division, double bpm) noexcept;

    double sampleRate = 44100.0;
    std::array<float, lfoCount> phases {};
    std::array<float, lfoCount> randomValues {};
    std::array<EnvelopeState, envelopeCount> envelopes {};
    unsigned int randomState = 0x2f6e2b1u;
};
}
