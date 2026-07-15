#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/AcidEngine.h"
#include "../dsp/EngineDescriptor.h"
#include "../model/PatternBank.h"
#include "../model/EngineControlBank.h"

#include <array>

struct AcidLabPresetInfo
{
    juce::String name;
    juce::String category;
    juce::String author;
    juce::String description;
    juce::String source = "Factory";
    juce::File file;
};

class AcidLab303AudioProcessor final : public juce::AudioProcessor
{
public:
    AcidLab303AudioProcessor();
    ~AcidLab303AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return apvts; }
    acidlab::EngineControls getEngineControls (int mode) const noexcept;
    void setEngineControl (int mode, int control, float value);

    acidlab::Step getStep (int index) const;
    void setStep (int index, acidlab::Step step);
    void randomizePattern();
    void mutatePattern();
    void clearPattern();
    void copyPattern();
    void pastePattern();
    bool exportActivePatternToMidi (const juce::File& file) const;
    bool writeActivePatternMidiToStream (juce::OutputStream& stream) const;
    juce::File createTemporaryMidiDragFile() const;
    bool savePatternBankToFile (const juce::File& file,
                                const juce::String& name,
                                const juce::String& author,
                                const juce::String& description) const;
    bool loadPatternBankFromFile (const juce::File& file);
    juce::String loadPresetFromFile (const juce::File& file);
    bool savePresetToFile (const juce::File& file,
                           const juce::String& name,
                           const juce::String& category,
                           const juce::String& author,
                           const juce::String& description);
    juce::Array<AcidLabPresetInfo> scanFactoryPresets() const;
    juce::Array<AcidLabPresetInfo> scanUserPresets() const;
    juce::Array<AcidLabPresetInfo> scanAllPresets (const juce::String& search, const juce::String& category) const;
    juce::String loadFactoryPreset (int index);
    int getLoadedPresetIndex() const noexcept { return loadedPresetIndex; }
    void initializePattern();
    int getCurrentStep() const noexcept { return currentStep.load(); }
    float getOutputLevel() const noexcept { return outputLevel.load(); }
    float getAccentLevel() const noexcept { return accentLevel.load(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    acidlab::Parameters readParameters() const;
    int getActivePatternSlot() const;
    juce::var createPresetVar (const juce::String& name,
                               const juce::String& category,
                               const juce::String& author,
                               const juce::String& description) const;
    bool applyPresetVar (const juce::var& preset);
    void migrateLegacyState (juce::XmlElement& xml) const;
    juce::var createPatternSlotsVar() const;
    void applyPatternSlotsVar (const juce::var& slots);
    juce::File getFactoryPresetDirectory() const;
    juce::File getUserPresetDirectory() const;
    acidlab::PatternBankMetadata createPatternBankMetadata (const juce::String& name,
                                                            const juce::String& author,
                                                            const juce::String& description) const;
    void writePatternToState();
    void readPatternFromState();

    juce::AudioProcessorValueTreeState apvts;
    acidlab::EngineControlBank engineControlBank;
    mutable juce::CriticalSection patternLock;
    acidlab::PatternBank patternBank;
    int loadedPresetIndex = -1;
    acidlab::AcidEngine engine;
    std::atomic<int> currentStep { -1 };
    std::atomic<float> outputLevel { 0.0f };
    std::atomic<float> accentLevel { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidLab303AudioProcessor)
};
