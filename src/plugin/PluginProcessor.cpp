#include "PluginProcessor.h"
#include "../ui/PluginEditor.h"
#include "../model/MidiPatternExporter.h"

#include <cmath>

namespace
{
constexpr auto stateId = "AcidLab303State";
constexpr auto patternId = "Patterns";

std::unique_ptr<juce::AudioParameterFloat> makeFloat (const juce::String& id,
                                                      const juce::String& name,
                                                      float min,
                                                      float max,
                                                      float defaultValue,
                                                      const juce::String& suffix = {})
{
    return std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id, 1 },
        name,
        juce::NormalisableRange<float> { min, max },
        defaultValue,
        juce::AudioParameterFloatAttributes().withLabel (suffix));
}
} // namespace

AcidLab303AudioProcessor::AcidLab303AudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, stateId, createParameterLayout())
{
    initializePattern();
}

juce::AudioProcessorValueTreeState::ParameterLayout AcidLab303AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "waveform", 1 },
        "Waveform",
        juce::StringArray { "Saw", "Pulse" },
        0));
    params.add (makeFloat ("tune", "Tune", -24.0f, 24.0f, 0.0f, "st"));
    params.add (makeFloat ("pulseWidth", "Pulse Width", 0.08f, 0.92f, 0.5f));
    params.add (makeFloat ("cutoff", "Cutoff", 35.0f, 9000.0f, 850.0f, "Hz"));
    params.add (makeFloat ("resonance", "Resonance", 0.0f, 0.98f, 0.48f));
    params.add (makeFloat ("envMod", "Env Mod", 0.0f, 1.0f, 0.62f));
    params.add (makeFloat ("decay", "Decay", 0.035f, 1.8f, 0.28f, "s"));
    params.add (makeFloat ("accentAmount", "Accent", 0.0f, 1.0f, 0.62f));
    params.add (makeFloat ("slideTime", "Slide", 0.005f, 0.45f, 0.08f, "s"));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "filterModel", 1 },
        "Filter Model",
        juce::StringArray { "Clean", "Acid" },
        1));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "oversampling", 1 },
        "Oversampling",
        juce::StringArray { "Off", "2x", "4x" },
        1));
    params.add (makeFloat ("filterTracking", "Filter Tracking", 0.0f, 1.0f, 0.25f));
    params.add (makeFloat ("bassBoost", "Bass Boost", 0.0f, 1.0f, 0.28f));
    params.add (makeFloat ("accentAttack", "Accent Attack", 0.0f, 1.0f, 0.38f));
    params.add (makeFloat ("clipLevel", "Clip Level", 0.25f, 1.0f, 0.85f));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "distMode", 1 },
        "Distortion Mode",
        juce::StringArray { "Warm", "Bite", "Fold", "Fuzz", "Zap" },
        0));
    params.add (makeFloat ("drive", "Drive", 0.0f, 1.0f, 0.42f));
    params.add (makeFloat ("distTone", "Dist Tone", 0.0f, 1.0f, 0.55f));
    params.add (makeFloat ("distMix", "Dist Mix", 0.0f, 1.0f, 0.48f));
    params.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "fxDelayOn", 1 },
        "Delay",
        false));
    params.add (makeFloat ("fxDelayMix", "Delay Mix", 0.0f, 0.8f, 0.0f));
    params.add (makeFloat ("fxDelayTime", "Delay Time", 0.04f, 0.75f, 0.25f, "s"));
    params.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "fxModOn", 1 },
        "Mod",
        false));
    params.add (makeFloat ("fxModDepth", "Mod Depth", 0.0f, 1.0f, 0.0f));
    params.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "fxReverbOn", 1 },
        "Reverb",
        false));
    params.add (makeFloat ("fxReverbMix", "Reverb Mix", 0.0f, 0.75f, 0.0f));
    params.add (makeFloat ("fxTone", "FX Tone", 0.0f, 1.0f, 0.5f));
    params.add (makeFloat ("subLevel", "Sub", 0.0f, 1.0f, 0.22f));
    params.add (makeFloat ("noise", "Circuit Noise", 0.0f, 0.08f, 0.012f));
    params.add (makeFloat ("drift", "Drift", 0.0f, 1.0f, 0.08f));
    params.add (makeFloat ("volume", "Volume", 0.0f, 1.0f, 0.78f));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "patternLength", 1 },
        "Pattern Length",
        juce::StringArray { "16", "32", "64" },
        0));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "seqRate", 1 },
        "Sequence Rate",
        juce::StringArray { "1/8", "1/16", "1/32", "Triplet" },
        1));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "scale", 1 },
        "Scale",
        juce::StringArray { "Chromatic", "Minor", "Major", "Phrygian", "Acid" },
        4));
    params.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "activePatternSlot", 1 },
        "Pattern Slot",
        juce::StringArray { "A", "B", "C", "D", "E", "F", "G", "H" },
        0));
    params.add (makeFloat ("mutationAmount", "Mutation", 0.0f, 1.0f, 0.35f));
    params.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "sequencerEnabled", 1 },
        "Sequencer",
        true));

    return params;
}

void AcidLab303AudioProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
}

void AcidLab303AudioProcessor::releaseResources() {}

bool AcidLab303AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AcidLab303AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    auto parameters = readParameters();

    if (auto* hostPlayHead = getPlayHead())
    {
        if (auto position = hostPlayHead->getPosition())
        {
            if (auto bpm = position->getBpm())
                parameters.bpm = *bpm;
        }
    }

    auto hostPlaying = false;
    if (auto* hostPlayHead = getPlayHead())
        if (auto position = hostPlayHead->getPosition())
            hostPlaying = position->getIsPlaying();

    if (! parameters.sequencerEnabled || ! hostPlaying)
    {
        for (const auto metadata : midiMessages)
        {
            const auto message = metadata.getMessage();
            if (message.isNoteOn())
                engine.noteOn (message.getNoteNumber(), message.getFloatVelocity(), false, message.getVelocity() > 96);
            else if (message.isNoteOff())
                engine.noteOff();
        }
    }

    std::array<acidlab::Step, acidlab::maxSteps> patternCopy;
    {
        const juce::ScopedLock lock (patternLock);
        patternCopy = patternBank.getPattern (getActivePatternSlot());
    }

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;
    engine.render (left, right, buffer.getNumSamples(), parameters, patternCopy, hostPlaying);
    currentStep.store (engine.getCurrentStep());
    outputLevel.store (engine.getOutputLevel());
    accentLevel.store (engine.getAccentLevel());
}

acidlab::Parameters AcidLab303AudioProcessor::readParameters() const
{
    acidlab::Parameters p;
    p.waveform = apvts.getRawParameterValue ("waveform")->load();
    p.tune = apvts.getRawParameterValue ("tune")->load();
    p.pulseWidth = apvts.getRawParameterValue ("pulseWidth")->load();
    p.cutoff = apvts.getRawParameterValue ("cutoff")->load();
    p.resonance = apvts.getRawParameterValue ("resonance")->load();
    p.envMod = apvts.getRawParameterValue ("envMod")->load();
    p.decay = apvts.getRawParameterValue ("decay")->load();
    p.accentAmount = apvts.getRawParameterValue ("accentAmount")->load();
    p.slideTime = apvts.getRawParameterValue ("slideTime")->load();
    p.filterModel = static_cast<int> (apvts.getRawParameterValue ("filterModel")->load());
    p.oversampling = static_cast<int> (apvts.getRawParameterValue ("oversampling")->load());
    p.filterTracking = apvts.getRawParameterValue ("filterTracking")->load();
    p.bassBoost = apvts.getRawParameterValue ("bassBoost")->load();
    p.accentAttack = apvts.getRawParameterValue ("accentAttack")->load();
    p.clipLevel = apvts.getRawParameterValue ("clipLevel")->load();
    p.distMode = static_cast<int> (apvts.getRawParameterValue ("distMode")->load());
    p.drive = apvts.getRawParameterValue ("drive")->load();
    p.distTone = apvts.getRawParameterValue ("distTone")->load();
    p.distMix = apvts.getRawParameterValue ("distMix")->load();
    p.fxDelayOn = apvts.getRawParameterValue ("fxDelayOn")->load() >= 0.5f;
    p.fxDelayMix = apvts.getRawParameterValue ("fxDelayMix")->load();
    p.fxDelayTime = apvts.getRawParameterValue ("fxDelayTime")->load();
    p.fxModOn = apvts.getRawParameterValue ("fxModOn")->load() >= 0.5f;
    p.fxModDepth = apvts.getRawParameterValue ("fxModDepth")->load();
    p.fxReverbOn = apvts.getRawParameterValue ("fxReverbOn")->load() >= 0.5f;
    p.fxReverbMix = apvts.getRawParameterValue ("fxReverbMix")->load();
    p.fxTone = apvts.getRawParameterValue ("fxTone")->load();
    p.subLevel = apvts.getRawParameterValue ("subLevel")->load();
    p.noise = apvts.getRawParameterValue ("noise")->load();
    p.drift = apvts.getRawParameterValue ("drift")->load();
    p.volume = apvts.getRawParameterValue ("volume")->load();
    const auto lengthChoice = static_cast<int> (apvts.getRawParameterValue ("patternLength")->load());
    p.patternLength = lengthChoice == 2 ? 64 : (lengthChoice == 1 ? 32 : 16);
    p.seqRate = static_cast<int> (apvts.getRawParameterValue ("seqRate")->load());
    p.sequencerEnabled = apvts.getRawParameterValue ("sequencerEnabled")->load() >= 0.5f;
    return p;
}

juce::AudioProcessorEditor* AcidLab303AudioProcessor::createEditor()
{
    return new AcidLab303AudioProcessorEditor (*this);
}

void AcidLab303AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    writePatternToState();
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AcidLab303AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (stateId))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            readPatternFromState();
        }
    }
}

acidlab::Step AcidLab303AudioProcessor::getStep (int index) const
{
    const juce::ScopedLock lock (patternLock);
    return patternBank.getStep (getActivePatternSlot(), index);
}

void AcidLab303AudioProcessor::setStep (int index, acidlab::Step step)
{
    const juce::ScopedLock lock (patternLock);
    patternBank.setStep (getActivePatternSlot(), index, step);
}

void AcidLab303AudioProcessor::initializePattern()
{
    const juce::ScopedLock lock (patternLock);
    patternBank.initializeFactory();
}

void AcidLab303AudioProcessor::randomizePattern()
{
    const auto scale = static_cast<int> (apvts.getRawParameterValue ("scale")->load());
    const juce::ScopedLock lock (patternLock);
    patternBank.randomize (getActivePatternSlot(), scale);
}

void AcidLab303AudioProcessor::mutatePattern()
{
    const auto amount = apvts.getRawParameterValue ("mutationAmount")->load();
    const auto scale = static_cast<int> (apvts.getRawParameterValue ("scale")->load());
    const juce::ScopedLock lock (patternLock);
    patternBank.mutate (getActivePatternSlot(), scale, amount);
}

void AcidLab303AudioProcessor::clearPattern()
{
    const juce::ScopedLock lock (patternLock);
    patternBank.clear (getActivePatternSlot());
}

void AcidLab303AudioProcessor::copyPattern()
{
    const juce::ScopedLock lock (patternLock);
    patternBank.copy (getActivePatternSlot());
}

void AcidLab303AudioProcessor::pastePattern()
{
    const juce::ScopedLock lock (patternLock);
    patternBank.paste (getActivePatternSlot());
}

bool AcidLab303AudioProcessor::exportActivePatternToMidi (const juce::File& file) const
{
    const auto params = readParameters();
    const juce::ScopedLock lock (patternLock);
    return acidlab::MidiPatternExporter::writePatternToFile (file, patternBank.getPattern (getActivePatternSlot()), params);
}

bool AcidLab303AudioProcessor::writeActivePatternMidiToStream (juce::OutputStream& stream) const
{
    const auto params = readParameters();
    const juce::ScopedLock lock (patternLock);
    return acidlab::MidiPatternExporter::writePatternToStream (stream, patternBank.getPattern (getActivePatternSlot()), params);
}

juce::File AcidLab303AudioProcessor::createTemporaryMidiDragFile() const
{
    auto file = juce::File::getSpecialLocation (juce::File::tempDirectory)
                    .getChildFile ("AcidLab303-" + juce::String (juce::Time::currentTimeMillis()) + ".mid");
    exportActivePatternToMidi (file);
    return file;
}

bool AcidLab303AudioProcessor::savePatternBankToFile (const juce::File& file,
                                                      const juce::String& name,
                                                      const juce::String& author,
                                                      const juce::String& description) const
{
    const juce::ScopedLock lock (patternLock);
    return patternBank.saveToFile (file, createPatternBankMetadata (name, author, description));
}

bool AcidLab303AudioProcessor::loadPatternBankFromFile (const juce::File& file)
{
    const juce::ScopedLock lock (patternLock);
    return patternBank.loadFromFile (file);
}

juce::String AcidLab303AudioProcessor::loadPresetFromFile (const juce::File& file)
{
    auto parsed = juce::JSON::parse (file);
    if (parsed.isVoid())
        return "Could not parse preset: " + file.getFileName();

    if (! applyPresetVar (parsed))
        return "Preset is missing required AcidLab303 data: " + file.getFileName();

    loadedPresetIndex = -1;
    return {};
}

bool AcidLab303AudioProcessor::savePresetToFile (const juce::File& file,
                                                 const juce::String& name,
                                                 const juce::String& category,
                                                 const juce::String& author,
                                                 const juce::String& description)
{
    auto preset = createPresetVar (name, category, author, description);
    file.getParentDirectory().createDirectory();
    return file.replaceWithText (juce::JSON::toString (preset, true));
}

juce::Array<AcidLabPresetInfo> AcidLab303AudioProcessor::scanFactoryPresets() const
{
    juce::Array<AcidLabPresetInfo> result;
    const auto dir = getFactoryPresetDirectory();
    juce::Array<juce::File> files;
    dir.findChildFiles (files, juce::File::findFiles, false, "*.acidpreset.json");
    files.sort();

    for (const auto& file : files)
    {
        auto parsed = juce::JSON::parse (file);
        if (auto* object = parsed.getDynamicObject())
        {
            auto* meta = object->getProperty ("meta").getDynamicObject();
            AcidLabPresetInfo info;
            info.file = file;
            info.name = meta != nullptr ? meta->getProperty ("name").toString() : file.getFileNameWithoutExtension();
            info.category = meta != nullptr ? meta->getProperty ("category").toString() : "Factory";
            info.author = meta != nullptr ? meta->getProperty ("author").toString() : "Zosia Audio";
            info.description = meta != nullptr ? meta->getProperty ("description").toString() : juce::String {};
            info.source = "Factory";
            result.add (info);
        }
    }

    return result;
}

juce::Array<AcidLabPresetInfo> AcidLab303AudioProcessor::scanUserPresets() const
{
    juce::Array<AcidLabPresetInfo> result;
    const auto dir = getUserPresetDirectory();
    juce::Array<juce::File> files;
    dir.findChildFiles (files, juce::File::findFiles, false, "*.acidpreset.json");
    files.sort();

    for (const auto& file : files)
    {
        auto parsed = juce::JSON::parse (file);
        if (auto* object = parsed.getDynamicObject())
        {
            auto* meta = object->getProperty ("meta").getDynamicObject();
            AcidLabPresetInfo info;
            info.file = file;
            info.name = meta != nullptr ? meta->getProperty ("name").toString() : file.getFileNameWithoutExtension();
            info.category = meta != nullptr ? meta->getProperty ("category").toString() : "User";
            info.author = meta != nullptr ? meta->getProperty ("author").toString() : "User";
            info.description = meta != nullptr ? meta->getProperty ("description").toString() : juce::String {};
            info.source = "User";
            result.add (info);
        }
    }

    return result;
}

juce::Array<AcidLabPresetInfo> AcidLab303AudioProcessor::scanAllPresets (const juce::String& search, const juce::String& category) const
{
    auto result = scanFactoryPresets();
    result.addArray (scanUserPresets());

    juce::Array<AcidLabPresetInfo> filtered;
    const auto needle = search.trim();
    for (const auto& preset : result)
    {
        const auto categoryMatches = category.isEmpty() || category == "ALL" || preset.category.equalsIgnoreCase (category);
        const auto searchMatches = needle.isEmpty()
            || preset.name.containsIgnoreCase (needle)
            || preset.category.containsIgnoreCase (needle)
            || preset.description.containsIgnoreCase (needle)
            || preset.source.containsIgnoreCase (needle);

        if (categoryMatches && searchMatches)
            filtered.add (preset);
    }

    return filtered;
}

juce::String AcidLab303AudioProcessor::loadFactoryPreset (int index)
{
    const auto presets = scanFactoryPresets();
    if (presets.isEmpty())
        return "No factory presets found.";

    const auto safeIndex = juce::jlimit (0, presets.size() - 1, index);
    const auto error = loadPresetFromFile (presets.getReference (safeIndex).file);
    if (error.isEmpty())
        loadedPresetIndex = safeIndex;
    return error;
}

int AcidLab303AudioProcessor::getActivePatternSlot() const
{
    return juce::jlimit (0, acidlab::patternSlots - 1, static_cast<int> (apvts.getRawParameterValue ("activePatternSlot")->load()));
}

juce::var AcidLab303AudioProcessor::createPresetVar (const juce::String& name,
                                                     const juce::String& category,
                                                     const juce::String& author,
                                                     const juce::String& description) const
{
    auto root = new juce::DynamicObject();
    root->setProperty ("format", "AcidLab303Preset");
    root->setProperty ("version", 1);

    auto meta = new juce::DynamicObject();
    meta->setProperty ("name", name);
    meta->setProperty ("category", category);
    meta->setProperty ("author", author);
    meta->setProperty ("description", description);
    root->setProperty ("meta", juce::var (meta));

    auto params = new juce::DynamicObject();
    for (auto* parameter : getParameters())
    {
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (parameter))
            params->setProperty (ranged->paramID, ranged->getValue());
    }
    root->setProperty ("parameters", juce::var (params));
    root->setProperty ("patterns", createPatternSlotsVar());

    return juce::var (root);
}

bool AcidLab303AudioProcessor::applyPresetVar (const juce::var& preset)
{
    auto* root = preset.getDynamicObject();
    if (root == nullptr || root->getProperty ("format").toString() != "AcidLab303Preset")
        return false;

    if (auto* params = root->getProperty ("parameters").getDynamicObject())
    {
        for (auto* parameter : getParameters())
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (parameter))
            {
                const auto value = params->getProperty (ranged->paramID);
                if (! value.isVoid())
                {
                    ranged->beginChangeGesture();
                    ranged->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, static_cast<float> (static_cast<double> (value))));
                    ranged->endChangeGesture();
                }
            }
        }
    }

    applyPatternSlotsVar (root->getProperty ("patterns"));
    return true;
}

juce::var AcidLab303AudioProcessor::createPatternSlotsVar() const
{
    const juce::ScopedLock lock (patternLock);
    return patternBank.toVar();
}

void AcidLab303AudioProcessor::applyPatternSlotsVar (const juce::var& slots)
{
    const juce::ScopedLock lock (patternLock);
    patternBank.fromVar (slots);
}

juce::File AcidLab303AudioProcessor::getFactoryPresetDirectory() const
{
    auto dir = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    for (int i = 0; i < 8; ++i)
    {
        auto candidate = dir.getChildFile ("presets").getChildFile ("factory");
        if (candidate.exists())
            return candidate;
        dir = dir.getParentDirectory();
    }

    auto cwd = juce::File::getCurrentWorkingDirectory().getChildFile ("presets").getChildFile ("factory");
    return cwd;
}

juce::File AcidLab303AudioProcessor::getUserPresetDirectory() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("AcidLab303")
        .getChildFile ("Presets");
}

acidlab::PatternBankMetadata AcidLab303AudioProcessor::createPatternBankMetadata (const juce::String& name,
                                                                                 const juce::String& author,
                                                                                 const juce::String& description) const
{
    acidlab::PatternBankMetadata metadata;
    metadata.name = name;
    metadata.author = author;
    metadata.description = description;
    metadata.scale = static_cast<int> (apvts.getRawParameterValue ("scale")->load());
    metadata.lengthChoice = static_cast<int> (apvts.getRawParameterValue ("patternLength")->load());
    metadata.rateChoice = static_cast<int> (apvts.getRawParameterValue ("seqRate")->load());
    return metadata;
}

void AcidLab303AudioProcessor::writePatternToState()
{
    auto state = apvts.copyState();
    state.removeChild (state.getChildWithName (patternId), nullptr);

    juce::ValueTree patternTree { patternId };
    {
        const juce::ScopedLock lock (patternLock);
        const auto slotsVar = patternBank.toVar();
        auto* slotsArray = slotsVar.getArray();
        for (int slot = 0; slot < acidlab::patternSlots; ++slot)
        {
            juce::ValueTree slotTree { "Slot" };
            slotTree.setProperty ("index", slot, nullptr);
            auto* stepArray = slotsArray != nullptr ? slotsArray->getReference (slot).getArray() : nullptr;
            for (int i = 0; i < acidlab::maxSteps && stepArray != nullptr; ++i)
            {
                auto* object = stepArray->getReference (i).getDynamicObject();
                if (object == nullptr)
                    continue;
                juce::ValueTree stepTree { "Step" };
                stepTree.setProperty ("index", i, nullptr);
                stepTree.setProperty ("note", object->getProperty ("note"), nullptr);
                stepTree.setProperty ("octave", object->getProperty ("octave"), nullptr);
                stepTree.setProperty ("accent", object->getProperty ("accent"), nullptr);
                stepTree.setProperty ("slide", object->getProperty ("slide"), nullptr);
                stepTree.setProperty ("gate", object->getProperty ("gate"), nullptr);
                stepTree.setProperty ("rest", object->getProperty ("rest"), nullptr);
                slotTree.addChild (stepTree, -1, nullptr);
            }
            patternTree.addChild (slotTree, -1, nullptr);
        }
    }

    state.addChild (patternTree, -1, nullptr);
    apvts.replaceState (state);
}

void AcidLab303AudioProcessor::readPatternFromState()
{
    const auto state = apvts.copyState();
    const auto patternTree = state.getChildWithName (patternId);
    if (! patternTree.isValid())
        return;

    const juce::ScopedLock lock (patternLock);
    juce::Array<juce::var> slots;
    for (int slot = 0; slot < acidlab::patternSlots; ++slot)
    {
        juce::Array<juce::var> steps;
        for (int i = 0; i < acidlab::maxSteps; ++i)
        {
            auto object = new juce::DynamicObject();
            object->setProperty ("note", 0);
            object->setProperty ("octave", 0);
            object->setProperty ("accent", false);
            object->setProperty ("slide", false);
            object->setProperty ("gate", false);
            object->setProperty ("rest", true);
            steps.add (juce::var (object));
        }
        slots.add (steps);
    }

    for (int i = 0; i < patternTree.getNumChildren(); ++i)
    {
        const auto slotOrStepTree = patternTree.getChild (i);
        if (slotOrStepTree.hasType ("Slot"))
        {
            const auto slot = juce::jlimit (0, acidlab::patternSlots - 1, static_cast<int> (slotOrStepTree.getProperty ("index", i)));
            auto* slotSteps = slots.getReference (slot).getArray();
            for (int j = 0; j < slotOrStepTree.getNumChildren(); ++j)
            {
                const auto stepTree = slotOrStepTree.getChild (j);
                const auto index = juce::jlimit (0, acidlab::maxSteps - 1, static_cast<int> (stepTree.getProperty ("index", j)));
                auto* object = slotSteps->getReference (index).getDynamicObject();
                object->setProperty ("note", stepTree.getProperty ("note", 0));
                object->setProperty ("octave", stepTree.getProperty ("octave", 0));
                object->setProperty ("accent", stepTree.getProperty ("accent", false));
                object->setProperty ("slide", stepTree.getProperty ("slide", false));
                object->setProperty ("gate", stepTree.getProperty ("gate", true));
                object->setProperty ("rest", stepTree.getProperty ("rest", false));
            }
        }
        else
        {
            const auto index = juce::jlimit (0, acidlab::maxSteps - 1, static_cast<int> (slotOrStepTree.getProperty ("index", i)));
            auto* slotSteps = slots.getReference (0).getArray();
            auto* object = slotSteps->getReference (index).getDynamicObject();
            object->setProperty ("note", slotOrStepTree.getProperty ("note", 0));
            object->setProperty ("octave", slotOrStepTree.getProperty ("octave", 0));
            object->setProperty ("accent", slotOrStepTree.getProperty ("accent", false));
            object->setProperty ("slide", slotOrStepTree.getProperty ("slide", false));
            object->setProperty ("gate", slotOrStepTree.getProperty ("gate", true));
            object->setProperty ("rest", slotOrStepTree.getProperty ("rest", false));
        }
    }

    patternBank.fromVar (slots);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AcidLab303AudioProcessor();
}
