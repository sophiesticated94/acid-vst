#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "../plugin/PluginProcessor.h"
#include "AcidLookAndFeel.h"
#include "EnergyStrip.h"
#include "MidiDragButton.h"
#include "StepGrid.h"

class AcidLab303AudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit AcidLab303AudioProcessorEditor (AcidLab303AudioProcessor&);
    ~AcidLab303AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    friend class StepGrid;
    friend class EnergyStrip;
    friend class MidiDragButton;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void timerCallback() override;
    void addKnob (juce::Slider& slider, juce::Label& label, const juce::String& text);
    void addCombo (juce::ComboBox& combo, juce::Label& label, const juce::String& text);
    void addToggle (juce::ToggleButton& button);
    int getDistortionMode() const;
    juce::Colour getThemeColour() const;
    juce::String getThemeName() const;
    void selectStep (int index);
    void refreshStepControls();
    void commitSelectedStep();
    void refreshPresetList();
    void loadSelectedPreset();
    void saveCurrentPreset();
    void savePresetAs();
    void savePresetToUserFile (juce::File file);
    void markDirty();
    void setDirty (bool shouldBeDirty);
    void updatePresetMetaLabel();
    void savePatternBank();
    void loadPatternBank();
    void selectTab (int index);
    void updateTabButtons();
    void updateVisibleTab();

    AcidLab303AudioProcessor& processor;
    AcidLookAndFeel lookAndFeel;

    StepGrid stepGrid;
    EnergyStrip energyStrip;
    int selectedStep = 0;
    bool updatingStepControls = false;

    juce::ComboBox waveformBox;
    juce::Label waveformLabel;
    juce::ComboBox lengthBox;
    juce::Label lengthLabel;
    juce::ComboBox distModeBox;
    juce::Label distModeLabel;
    juce::ComboBox filterModelBox;
    juce::Label filterModelLabel;
    juce::ComboBox oversamplingBox;
    juce::Label oversamplingLabel;
    juce::ComboBox seqRateBox;
    juce::Label seqRateLabel;
    juce::ComboBox scaleBox;
    juce::Label scaleLabel;
    juce::ComboBox patternSlotBox;
    juce::Label patternSlotLabel;
    juce::ComboBox presetCategoryBox;
    juce::Label presetCategoryLabel;
    juce::ComboBox presetSourceBox;
    juce::Label presetSourceLabel;
    juce::ComboBox presetBox;
    juce::Label presetLabel;
    juce::TextEditor presetSearchBox;
    juce::Label presetMetaLabel;

    juce::Slider tuneSlider;
    juce::Slider pulseWidthSlider;
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider envModSlider;
    juce::Slider decaySlider;
    juce::Slider accentSlider;
    juce::Slider slideSlider;
    juce::Slider driveSlider;
    juce::Slider toneSlider;
    juce::Slider mixSlider;
    juce::Slider filterTrackingSlider;
    juce::Slider bassBoostSlider;
    juce::Slider accentAttackSlider;
    juce::Slider clipLevelSlider;
    juce::Slider fxDelayMixSlider;
    juce::Slider fxDelayTimeSlider;
    juce::Slider fxModDepthSlider;
    juce::Slider fxReverbMixSlider;
    juce::Slider fxToneSlider;
    juce::Slider mutationSlider;
    juce::Slider subSlider;
    juce::Slider noiseSlider;
    juce::Slider driftSlider;
    juce::Slider volumeSlider;

    juce::Label tuneLabel;
    juce::Label pulseWidthLabel;
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label envModLabel;
    juce::Label decayLabel;
    juce::Label accentLabel;
    juce::Label slideLabel;
    juce::Label driveLabel;
    juce::Label toneLabel;
    juce::Label mixLabel;
    juce::Label filterTrackingLabel;
    juce::Label bassBoostLabel;
    juce::Label accentAttackLabel;
    juce::Label clipLevelLabel;
    juce::Label fxDelayMixLabel;
    juce::Label fxDelayTimeLabel;
    juce::Label fxModDepthLabel;
    juce::Label fxReverbMixLabel;
    juce::Label fxToneLabel;
    juce::Label mutationLabel;
    juce::Label subLabel;
    juce::Label noiseLabel;
    juce::Label driftLabel;
    juce::Label volumeLabel;

    juce::ToggleButton sequencerButton { "SEQ" };
    juce::ToggleButton fxDelayButton { "DLY" };
    juce::ToggleButton fxModButton { "MOD" };
    juce::ToggleButton fxReverbButton { "RVB" };
    juce::TextButton initButton { "INIT" };
    juce::TextButton randomButton { "RND" };
    juce::TextButton mutateButton { "MUT" };
    juce::TextButton copyButton { "CPY" };
    juce::TextButton pasteButton { "PST" };
    juce::TextButton clearButton { "CLR" };
    MidiDragButton exportButton;
    juce::TextButton prevPresetButton { "<" };
    juce::TextButton nextPresetButton { ">" };
    juce::TextButton loadPresetButton { "LOAD" };
    juce::TextButton savePresetButton { "SAVE" };
    juce::TextButton saveAsPresetButton { "SAVE AS" };
    juce::TextButton patternLoadButton { "PAT LOAD" };
    juce::TextButton patternSaveButton { "PAT SAVE" };
    juce::TextButton synthTabButton { "SYNTH" };
    juce::TextButton sequencerTabButton { "SEQUENCER" };
    juce::TextButton modTabButton { "MOD" };
    juce::TextButton effectsTabButton { "EFFECTS" };
    juce::TextButton analogTabButton { "ANALOG" };

    juce::Slider stepNoteSlider;
    juce::Slider stepOctaveSlider;
    juce::ToggleButton stepAccentButton { "ACC" };
    juce::ToggleButton stepSlideButton { "SLD" };
    juce::ToggleButton stepGateButton { "GATE" };
    juce::ToggleButton stepRestButton { "REST" };
    juce::Label stepEditLabel;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Array<AcidLabPresetInfo> allPresets;
    juce::Array<int> visiblePresetIndices;
    AcidLabPresetInfo currentPreset;
    bool hasCurrentPreset = false;
    bool presetDirty = false;
    int currentTab = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidLab303AudioProcessorEditor)
};
