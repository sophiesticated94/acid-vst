#include "PluginEditor.h"

namespace
{
using namespace acidlab::ui;

void styleButton (juce::Button& button)
{
    button.setColour (juce::TextButton::buttonColourId, panelBright);
    button.setColour (juce::TextButton::buttonOnColourId, defaultAcid.withAlpha (0.7f));
    button.setColour (juce::TextButton::textColourOffId, text);
    button.setColour (juce::TextButton::textColourOnId, background);
}

void styleTabButton (juce::TextButton& button)
{
    styleButton (button);
    button.setClickingTogglesState (false);
}
} // namespace

AcidLab303AudioProcessorEditor::AcidLab303AudioProcessorEditor (AcidLab303AudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      stepGrid (*this),
      energyStrip (*this),
      exportButton (*this, "MIDI")
{
    lookAndFeel.setColour (juce::Slider::rotarySliderFillColourId, defaultAcid);
    lookAndFeel.setColour (juce::Slider::rotarySliderOutlineColourId, muted);
    lookAndFeel.setColour (juce::Slider::thumbColourId, amber);
    lookAndFeel.setColour (juce::ComboBox::backgroundColourId, panelBright);
    lookAndFeel.setColour (juce::ComboBox::textColourId, text);
    lookAndFeel.setColour (juce::Label::textColourId, text);
    setLookAndFeel (&lookAndFeel);

    auto& state = processor.getValueTreeState();

    addCombo (waveformBox, waveformLabel, "WAVE");
    waveformBox.addItem ("SAW", 1);
    waveformBox.addItem ("PULSE", 2);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "waveform", waveformBox));

    addCombo (lengthBox, lengthLabel, "LEN");
    lengthBox.addItem ("16", 1);
    lengthBox.addItem ("32", 2);
    lengthBox.addItem ("64", 3);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "patternLength", lengthBox));

    addCombo (distModeBox, distModeLabel, "DIST");
    distModeBox.addItem ("WARM", 1);
    distModeBox.addItem ("BITE", 2);
    distModeBox.addItem ("FOLD", 3);
    distModeBox.addItem ("FUZZ", 4);
    distModeBox.addItem ("ZAP", 5);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "distMode", distModeBox));

    addCombo (filterModelBox, filterModelLabel, "FILTER");
    filterModelBox.addItem ("CLEAN", 1);
    filterModelBox.addItem ("ACID", 2);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "filterModel", filterModelBox));

    addCombo (oversamplingBox, oversamplingLabel, "OS");
    oversamplingBox.addItem ("OFF", 1);
    oversamplingBox.addItem ("2X", 2);
    oversamplingBox.addItem ("4X", 3);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "oversampling", oversamplingBox));

    addCombo (seqRateBox, seqRateLabel, "RATE");
    seqRateBox.addItem ("1/8", 1);
    seqRateBox.addItem ("1/16", 2);
    seqRateBox.addItem ("1/32", 3);
    seqRateBox.addItem ("TRI", 4);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "seqRate", seqRateBox));

    addCombo (scaleBox, scaleLabel, "SCALE");
    scaleBox.addItem ("CHR", 1);
    scaleBox.addItem ("MIN", 2);
    scaleBox.addItem ("MAJ", 3);
    scaleBox.addItem ("PHR", 4);
    scaleBox.addItem ("ACID", 5);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "scale", scaleBox));

    addCombo (patternSlotBox, patternSlotLabel, "PATT");
    for (int i = 0; i < acidlab::patternSlots; ++i)
        patternSlotBox.addItem (juce::String (static_cast<char> ('A' + i)), i + 1);
    comboAttachments.push_back (std::make_unique<ComboAttachment> (state, "activePatternSlot", patternSlotBox));
    patternSlotBox.onChange = [this]
    {
        markDirty();
        refreshStepControls();
        stepGrid.repaint();
    };

    addCombo (presetCategoryBox, presetCategoryLabel, "CATEGORY");
    presetCategoryBox.addItem ("ALL", 1);
    presetCategoryBox.addItem ("ACID BASS", 2);
    presetCategoryBox.addItem ("DISTORTED", 3);
    presetCategoryBox.addItem ("SEQUENCED", 4);
    presetCategoryBox.addItem ("FX", 5);
    presetCategoryBox.addItem ("INIT", 6);
    presetCategoryBox.onChange = [this] { refreshPresetList(); };

    addCombo (presetSourceBox, presetSourceLabel, "SOURCE");
    presetSourceBox.addItem ("ALL", 1);
    presetSourceBox.addItem ("FACTORY", 2);
    presetSourceBox.addItem ("USER", 3);
    presetSourceBox.setSelectedId (1, juce::dontSendNotification);
    presetSourceBox.onChange = [this] { refreshPresetList(); };

    addCombo (presetBox, presetLabel, "PRESET");
    presetBox.onChange = [this] { loadSelectedPreset(); };
    presetSearchBox.setTextToShowWhenEmpty ("search", muted);
    presetSearchBox.setColour (juce::TextEditor::backgroundColourId, panelBright);
    presetSearchBox.setColour (juce::TextEditor::textColourId, text);
    presetSearchBox.setColour (juce::TextEditor::outlineColourId, muted.withAlpha (0.35f));
    presetSearchBox.onTextChange = [this] { refreshPresetList(); };
    addAndMakeVisible (presetSearchBox);
    presetMetaLabel.setJustificationType (juce::Justification::topLeft);
    presetMetaLabel.setColour (juce::Label::textColourId, muted);
    presetMetaLabel.setText ("No preset loaded", juce::dontSendNotification);
    addAndMakeVisible (presetMetaLabel);

    addKnob (tuneSlider, tuneLabel, "TUNE");
    addKnob (pulseWidthSlider, pulseWidthLabel, "PW");
    addKnob (cutoffSlider, cutoffLabel, "CUTOFF");
    addKnob (resonanceSlider, resonanceLabel, "RES");
    addKnob (envModSlider, envModLabel, "ENV");
    addKnob (decaySlider, decayLabel, "DECAY");
    addKnob (accentSlider, accentLabel, "ACCENT");
    addKnob (slideSlider, slideLabel, "SLIDE");
    addKnob (driveSlider, driveLabel, "DRIVE");
    addKnob (toneSlider, toneLabel, "TONE");
    addKnob (mixSlider, mixLabel, "MIX");
    addKnob (filterTrackingSlider, filterTrackingLabel, "TRACK");
    addKnob (bassBoostSlider, bassBoostLabel, "BASS");
    addKnob (accentAttackSlider, accentAttackLabel, "ATTACK");
    addKnob (clipLevelSlider, clipLevelLabel, "CLIP");
    addKnob (fxDelayMixSlider, fxDelayMixLabel, "DLY MIX");
    addKnob (fxDelayTimeSlider, fxDelayTimeLabel, "DLY TIME");
    addKnob (fxModDepthSlider, fxModDepthLabel, "MOD");
    addKnob (fxReverbMixSlider, fxReverbMixLabel, "RVB");
    addKnob (fxToneSlider, fxToneLabel, "FX TONE");
    addKnob (mutationSlider, mutationLabel, "MUTATE");
    addKnob (subSlider, subLabel, "SUB");
    addKnob (noiseSlider, noiseLabel, "NOISE");
    addKnob (driftSlider, driftLabel, "DRIFT");
    addKnob (volumeSlider, volumeLabel, "VOL");

    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "tune", tuneSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "pulseWidth", pulseWidthSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "cutoff", cutoffSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "resonance", resonanceSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "envMod", envModSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "decay", decaySlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "accentAmount", accentSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "slideTime", slideSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "drive", driveSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "distTone", toneSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "distMix", mixSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "filterTracking", filterTrackingSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "bassBoost", bassBoostSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "accentAttack", accentAttackSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "clipLevel", clipLevelSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "fxDelayMix", fxDelayMixSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "fxDelayTime", fxDelayTimeSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "fxModDepth", fxModDepthSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "fxReverbMix", fxReverbMixSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "fxTone", fxToneSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "mutationAmount", mutationSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "subLevel", subSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "noise", noiseSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "drift", driftSlider));
    sliderAttachments.push_back (std::make_unique<SliderAttachment> (state, "volume", volumeSlider));

    addAndMakeVisible (stepGrid);
    addAndMakeVisible (energyStrip);

    styleButton (sequencerButton);
    addAndMakeVisible (sequencerButton);
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "sequencerEnabled", sequencerButton));

    addToggle (fxDelayButton);
    addToggle (fxModButton);
    addToggle (fxReverbButton);
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "fxDelayOn", fxDelayButton));
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "fxModOn", fxModButton));
    buttonAttachments.push_back (std::make_unique<ButtonAttachment> (state, "fxReverbOn", fxReverbButton));

    styleButton (initButton);
    styleButton (randomButton);
    styleButton (mutateButton);
    styleButton (copyButton);
    styleButton (pasteButton);
    styleButton (clearButton);
    styleButton (exportButton);
    styleButton (prevPresetButton);
    styleButton (nextPresetButton);
    styleButton (loadPresetButton);
    styleButton (savePresetButton);
    styleButton (saveAsPresetButton);
    styleButton (patternLoadButton);
    styleButton (patternSaveButton);
    for (auto* button : { &synthTabButton, &sequencerTabButton, &modTabButton, &effectsTabButton, &analogTabButton })
    {
        styleTabButton (*button);
        addAndMakeVisible (*button);
    }
    addAndMakeVisible (initButton);
    addAndMakeVisible (randomButton);
    addAndMakeVisible (mutateButton);
    addAndMakeVisible (copyButton);
    addAndMakeVisible (pasteButton);
    addAndMakeVisible (clearButton);
    addAndMakeVisible (exportButton);
    addAndMakeVisible (prevPresetButton);
    addAndMakeVisible (nextPresetButton);
    addAndMakeVisible (loadPresetButton);
    addAndMakeVisible (savePresetButton);
    addAndMakeVisible (saveAsPresetButton);
    addAndMakeVisible (patternLoadButton);
    addAndMakeVisible (patternSaveButton);
    initButton.onClick = [this]
    {
        processor.initializePattern();
        markDirty();
        refreshStepControls();
        stepGrid.repaint();
    };
    randomButton.onClick = [this]
    {
        processor.randomizePattern();
        markDirty();
        refreshStepControls();
        stepGrid.repaint();
    };
    mutateButton.onClick = [this]
    {
        processor.mutatePattern();
        markDirty();
        refreshStepControls();
        stepGrid.repaint();
    };
    copyButton.onClick = [this] { processor.copyPattern(); };
    pasteButton.onClick = [this]
    {
        processor.pastePattern();
        markDirty();
        refreshStepControls();
        stepGrid.repaint();
    };
    clearButton.onClick = [this]
    {
        processor.clearPattern();
        markDirty();
        refreshStepControls();
        stepGrid.repaint();
    };
    exportButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Export pattern as MIDI",
                                                           juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("AcidLab303.mid"),
                                                           "*.mid");
        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                                  [this] (const juce::FileChooser& chooser)
                                  {
                                      auto file = chooser.getResult();
                                      if (file == juce::File{})
                                          return;
                                      if (! file.hasFileExtension (".mid"))
                                          file = file.withFileExtension (".mid");
                                      processor.exportActivePatternToMidi (file);
                                  });
    };
    prevPresetButton.onClick = [this]
    {
        if (presetBox.getNumItems() == 0)
            return;
        const auto nextId = presetBox.getSelectedId() <= 1 ? presetBox.getNumItems() : presetBox.getSelectedId() - 1;
        presetBox.setSelectedId (nextId, juce::sendNotification);
    };
    nextPresetButton.onClick = [this]
    {
        if (presetBox.getNumItems() == 0)
            return;
        const auto nextId = presetBox.getSelectedId() >= presetBox.getNumItems() ? 1 : presetBox.getSelectedId() + 1;
        presetBox.setSelectedId (nextId, juce::sendNotification);
    };
    loadPresetButton.onClick = [this] { loadSelectedPreset(); };
    savePresetButton.onClick = [this] { saveCurrentPreset(); };
    saveAsPresetButton.onClick = [this] { savePresetAs(); };
    synthTabButton.onClick = [this] { selectTab (0); };
    sequencerTabButton.onClick = [this] { selectTab (1); };
    modTabButton.onClick = [this] { selectTab (2); };
    effectsTabButton.onClick = [this] { selectTab (3); };
    analogTabButton.onClick = [this] { selectTab (4); };

    patternLoadButton.onClick = [this] { loadPatternBank(); };
    patternSaveButton.onClick = [this] { savePatternBank(); };

    addAndMakeVisible (stepEditLabel);
    stepEditLabel.setJustificationType (juce::Justification::centredLeft);
    stepEditLabel.setText ("STEP", juce::dontSendNotification);

    stepNoteSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    stepNoteSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 20);
    stepNoteSlider.setRange (0, 11, 1);
    stepNoteSlider.onValueChange = [this] { commitSelectedStep(); };
    addAndMakeVisible (stepNoteSlider);

    stepOctaveSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    stepOctaveSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 20);
    stepOctaveSlider.setRange (-2, 2, 1);
    stepOctaveSlider.onValueChange = [this] { commitSelectedStep(); };
    addAndMakeVisible (stepOctaveSlider);

    for (auto* button : { &stepAccentButton, &stepSlideButton, &stepGateButton, &stepRestButton })
    {
        styleButton (*button);
        button->onClick = [this] { commitSelectedStep(); };
        addAndMakeVisible (*button);
    }

    setResizable (true, true);
    setResizeLimits (980, 720, 1600, 1186);
    getConstrainer()->setFixedAspectRatio (1160.0 / 860.0);
    setSize (1160, 860);
    refreshPresetList();
    updateTabButtons();
    updateVisibleTab();
    updatePresetMetaLabel();
    selectStep (0);
    startTimerHz (30);
}

AcidLab303AudioProcessorEditor::~AcidLab303AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AcidLab303AudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto theme = getThemeColour();
    g.fillAll (background.interpolatedWith (theme, 0.035f));
    auto bounds = getLocalBounds().reduced (18);

    auto header = bounds.removeFromTop (54);
    g.setColour (theme);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("AcidLab303", header.removeFromLeft (220), juce::Justification::centredLeft);

    g.setColour (muted);
    g.setFont (13.0f);
    g.drawText ("original analog-style acid bassline instrument", header, juce::Justification::centredLeft);

    auto content = bounds;
    auto sidebar = content.removeFromLeft (264);
    content.removeFromLeft (14);

    auto drawPanel = [&g] (juce::Rectangle<int> r, const juce::String& label, juce::Colour accent)
    {
        g.setColour (panel.withAlpha (0.78f));
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (accent.withAlpha (0.8f));
        g.fillRoundedRectangle (r.withHeight (2).toFloat(), 1.0f);
        g.setColour (text.withAlpha (0.88f));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (label, r.reduced (12, 8).removeFromTop (18), juce::Justification::centredLeft);
    };

    drawPanel (sidebar, "PRESETS", theme);

    auto engine = content.removeFromTop (58);
    drawPanel (engine, "ENGINE / THEME", theme);
    content.removeFromTop (10);
    auto tabs = content.removeFromTop (42);
    g.setColour (panel.withAlpha (0.75f));
    g.fillRoundedRectangle (tabs.toFloat(), 8.0f);
    content.removeFromTop (10);

    juce::String tabName = "SYNTH";
    if (currentTab == 1) tabName = "SEQUENCER";
    if (currentTab == 2) tabName = "MODULATION";
    if (currentTab == 3) tabName = "EFFECTS";
    if (currentTab == 4) tabName = "ANALOG";
    drawPanel (content, tabName, theme);
}

void AcidLab303AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (18);
    area.removeFromTop (54);

    auto sidebar = area.removeFromLeft (264).reduced (12, 34);
    area.removeFromLeft (14);

    auto placeCombo = [] (juce::Rectangle<int>& column, juce::Label& label, juce::ComboBox& combo)
    {
        label.setBounds (column.removeFromTop (18));
        combo.setBounds (column.removeFromTop (28));
        column.removeFromTop (9);
    };

    presetSearchBox.setBounds (sidebar.removeFromTop (32));
    sidebar.removeFromTop (8);
    placeCombo (sidebar, presetCategoryLabel, presetCategoryBox);
    placeCombo (sidebar, presetSourceLabel, presetSourceBox);
    placeCombo (sidebar, presetLabel, presetBox);

    auto presetNav = sidebar.removeFromTop (34);
    prevPresetButton.setBounds (presetNav.removeFromLeft (42).reduced (3));
    nextPresetButton.setBounds (presetNav.removeFromLeft (42).reduced (3));
    loadPresetButton.setBounds (presetNav.removeFromLeft (72).reduced (3));
    savePresetButton.setBounds (presetNav.removeFromLeft (64).reduced (3));
    sidebar.removeFromTop (6);
    saveAsPresetButton.setBounds (sidebar.removeFromTop (34).reduced (3));
    sidebar.removeFromTop (8);
    presetMetaLabel.setBounds (sidebar);

    auto engine = area.removeFromTop (58).reduced (12, 10);
    distModeLabel.setBounds (engine.removeFromLeft (62));
    distModeBox.setBounds (engine.removeFromLeft (132).reduced (3));
    engine.removeFromLeft (18);
    energyStrip.setBounds (engine);
    area.removeFromTop (10);

    auto tabs = area.removeFromTop (42).reduced (8, 7);
    const auto tabW = tabs.getWidth() / 5;
    synthTabButton.setBounds (tabs.removeFromLeft (tabW).reduced (3));
    sequencerTabButton.setBounds (tabs.removeFromLeft (tabW).reduced (3));
    modTabButton.setBounds (tabs.removeFromLeft (tabW).reduced (3));
    effectsTabButton.setBounds (tabs.removeFromLeft (tabW).reduced (3));
    analogTabButton.setBounds (tabs.reduced (3));
    area.removeFromTop (10);

    auto panelArea = area.reduced (14, 34);
    auto placeKnob = [] (juce::Rectangle<int>& row, int width, juce::Slider& slider, juce::Label& label)
    {
        auto cell = row.removeFromLeft (width).reduced (5);
        label.setBounds (cell.removeFromTop (18));
        slider.setBounds (cell);
    };
    auto placeKnobRow = [&placeKnob] (juce::Rectangle<int>& panelBounds, std::initializer_list<std::pair<juce::Slider*, juce::Label*>> controls)
    {
        auto row = panelBounds.removeFromTop (132);
        const auto width = row.getWidth() / static_cast<int> (controls.size());
        for (auto control : controls)
            placeKnob (row, width, *control.first, *control.second);
        panelBounds.removeFromTop (10);
    };

    auto hideBounds = juce::Rectangle<int> (-4000, -4000, 10, 10);
    auto hideControl = [&hideBounds] (juce::Component& c) { c.setBounds (hideBounds); };
    juce::Component* tabComponents[] = {
        &waveformBox, &waveformLabel, &filterModelBox, &filterModelLabel,
        &tuneSlider, &tuneLabel, &pulseWidthSlider, &pulseWidthLabel, &cutoffSlider, &cutoffLabel,
        &resonanceSlider, &resonanceLabel, &envModSlider, &envModLabel, &decaySlider, &decayLabel,
        &accentSlider, &accentLabel, &slideSlider, &slideLabel, &subSlider, &subLabel, &volumeSlider, &volumeLabel,
        &stepGrid, &patternSlotBox, &patternSlotLabel, &lengthBox, &lengthLabel, &seqRateBox, &seqRateLabel,
        &scaleBox, &scaleLabel, &sequencerButton, &stepEditLabel, &stepNoteSlider, &stepOctaveSlider,
        &stepAccentButton, &stepSlideButton, &stepGateButton, &stepRestButton, &initButton, &randomButton,
        &mutateButton, &copyButton, &pasteButton, &clearButton, &exportButton, &patternLoadButton,
        &patternSaveButton, &driveSlider, &driveLabel, &toneSlider, &toneLabel, &mixSlider, &mixLabel,
        &fxDelayButton, &fxModButton, &fxReverbButton, &fxDelayMixSlider, &fxDelayMixLabel,
        &fxDelayTimeSlider, &fxDelayTimeLabel, &fxModDepthSlider, &fxModDepthLabel,
        &fxReverbMixSlider, &fxReverbMixLabel, &fxToneSlider, &fxToneLabel, &mutationSlider,
        &mutationLabel, &oversamplingBox, &oversamplingLabel, &filterTrackingSlider, &filterTrackingLabel,
        &bassBoostSlider, &bassBoostLabel, &accentAttackSlider, &accentAttackLabel, &clipLevelSlider,
        &clipLevelLabel, &noiseSlider, &noiseLabel, &driftSlider, &driftLabel
    };
    for (auto* c : tabComponents)
        hideControl (*c);

    if (currentTab == 0)
    {
        auto combos = panelArea.removeFromTop (56);
        auto comboW = combos.getWidth() / 2;
        auto left = combos.removeFromLeft (comboW).reduced (4, 0);
        auto right = combos.reduced (4, 0);
        waveformLabel.setBounds (left.removeFromTop (18));
        waveformBox.setBounds (left.removeFromTop (30));
        filterModelLabel.setBounds (right.removeFromTop (18));
        filterModelBox.setBounds (right.removeFromTop (30));
        panelArea.removeFromTop (20);
        placeKnobRow (panelArea, { { &tuneSlider, &tuneLabel }, { &pulseWidthSlider, &pulseWidthLabel }, { &cutoffSlider, &cutoffLabel }, { &resonanceSlider, &resonanceLabel } });
        placeKnobRow (panelArea, { { &envModSlider, &envModLabel }, { &decaySlider, &decayLabel }, { &accentSlider, &accentLabel }, { &slideSlider, &slideLabel } });
        placeKnobRow (panelArea, { { &subSlider, &subLabel }, { &volumeSlider, &volumeLabel } });
    }
    else if (currentTab == 1)
    {
        auto seqTop = panelArea.removeFromTop (54);
        patternSlotLabel.setBounds (seqTop.removeFromLeft (66));
        patternSlotBox.setBounds (seqTop.removeFromLeft (70).reduced (4));
        lengthLabel.setBounds (seqTop.removeFromLeft (44));
        lengthBox.setBounds (seqTop.removeFromLeft (74).reduced (4));
        seqRateLabel.setBounds (seqTop.removeFromLeft (50));
        seqRateBox.setBounds (seqTop.removeFromLeft (88).reduced (4));
        scaleLabel.setBounds (seqTop.removeFromLeft (54));
        scaleBox.setBounds (seqTop.removeFromLeft (96).reduced (4));
        sequencerButton.setBounds (seqTop.removeFromLeft (62).reduced (4));
        patternLoadButton.setBounds (seqTop.removeFromRight (90).reduced (4));
        patternSaveButton.setBounds (seqTop.removeFromRight (90).reduced (4));
        panelArea.removeFromTop (8);
        stepGrid.setBounds (panelArea.removeFromTop (250));
        panelArea.removeFromTop (10);
        auto edit = panelArea.removeFromTop (50);
        stepEditLabel.setBounds (edit.removeFromLeft (92));
        stepNoteSlider.setBounds (edit.removeFromLeft (170).reduced (4));
        stepOctaveSlider.setBounds (edit.removeFromLeft (140).reduced (4));
        stepAccentButton.setBounds (edit.removeFromLeft (68).reduced (4));
        stepSlideButton.setBounds (edit.removeFromLeft (68).reduced (4));
        stepGateButton.setBounds (edit.removeFromLeft (78).reduced (4));
        stepRestButton.setBounds (edit.removeFromLeft (78).reduced (4));
        exportButton.setBounds (edit.removeFromRight (72).reduced (4));
        clearButton.setBounds (edit.removeFromRight (62).reduced (4));
        pasteButton.setBounds (edit.removeFromRight (62).reduced (4));
        copyButton.setBounds (edit.removeFromRight (62).reduced (4));
        mutateButton.setBounds (edit.removeFromRight (62).reduced (4));
        randomButton.setBounds (edit.removeFromRight (62).reduced (4));
        initButton.setBounds (edit.removeFromRight (62).reduced (4));
    }
    else if (currentTab == 2)
    {
        placeKnobRow (panelArea, { { &mutationSlider, &mutationLabel }, { &envModSlider, &envModLabel }, { &accentSlider, &accentLabel }, { &slideSlider, &slideLabel } });
        placeKnobRow (panelArea, { { &decaySlider, &decayLabel }, { &accentAttackSlider, &accentAttackLabel } });
    }
    else if (currentTab == 3)
    {
        placeKnobRow (panelArea, { { &driveSlider, &driveLabel }, { &toneSlider, &toneLabel }, { &mixSlider, &mixLabel }, { &fxToneSlider, &fxToneLabel } });
        auto toggles = panelArea.removeFromTop (42);
        fxDelayButton.setBounds (toggles.removeFromLeft (76).reduced (4));
        fxModButton.setBounds (toggles.removeFromLeft (76).reduced (4));
        fxReverbButton.setBounds (toggles.removeFromLeft (76).reduced (4));
        panelArea.removeFromTop (14);
        placeKnobRow (panelArea, { { &fxDelayMixSlider, &fxDelayMixLabel }, { &fxDelayTimeSlider, &fxDelayTimeLabel }, { &fxModDepthSlider, &fxModDepthLabel }, { &fxReverbMixSlider, &fxReverbMixLabel } });
    }
    else
    {
        auto osRow = panelArea.removeFromTop (56);
        oversamplingLabel.setBounds (osRow.removeFromLeft (92));
        oversamplingBox.setBounds (osRow.removeFromLeft (120).reduced (4));
        panelArea.removeFromTop (20);
        placeKnobRow (panelArea, { { &filterTrackingSlider, &filterTrackingLabel }, { &bassBoostSlider, &bassBoostLabel }, { &accentAttackSlider, &accentAttackLabel }, { &clipLevelSlider, &clipLevelLabel } });
        placeKnobRow (panelArea, { { &noiseSlider, &noiseLabel }, { &driftSlider, &driftLabel } });
    }
}

void AcidLab303AudioProcessorEditor::timerCallback()
{
    const auto theme = getThemeColour();
    lookAndFeel.setColour (juce::Slider::rotarySliderFillColourId, theme);
    lookAndFeel.setColour (juce::ToggleButton::tickColourId, theme);
    juce::TextButton* tabButtons[] = { &synthTabButton, &sequencerTabButton, &modTabButton, &effectsTabButton, &analogTabButton };
    for (auto* button : tabButtons)
        button->setColour (juce::TextButton::buttonOnColourId, theme.withAlpha (0.9f));
    juce::TextButton* themedButtons[] = { &loadPresetButton, &savePresetButton, &saveAsPresetButton, &initButton, &randomButton, &mutateButton,
                                          &copyButton, &pasteButton, &clearButton, &exportButton, &patternLoadButton, &patternSaveButton };
    for (auto* button : themedButtons)
        button->setColour (juce::TextButton::buttonOnColourId, theme.withAlpha (0.7f));
    stepGrid.repaint();
    energyStrip.repaint();
    repaint();
}

void AcidLab303AudioProcessorEditor::addKnob (juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    slider.onValueChange = [this] { markDirty(); };
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void AcidLab303AudioProcessorEditor::addCombo (juce::ComboBox& combo, juce::Label& label, const juce::String& labelText)
{
    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (label);
    addAndMakeVisible (combo);
    combo.onChange = [this] { markDirty(); };
}

void AcidLab303AudioProcessorEditor::addToggle (juce::ToggleButton& button)
{
    button.setColour (juce::ToggleButton::textColourId, text);
    button.setColour (juce::ToggleButton::tickColourId, getThemeColour());
    addAndMakeVisible (button);
}

int AcidLab303AudioProcessorEditor::getDistortionMode() const
{
    if (auto* value = processor.getValueTreeState().getRawParameterValue ("distMode"))
        return juce::jlimit (0, 4, static_cast<int> (value->load()));

    return 0;
}

juce::Colour AcidLab303AudioProcessorEditor::getThemeColour() const
{
    switch (getDistortionMode())
    {
        case 1: return juce::Colour::fromRGB (255, 124, 62);
        case 2: return juce::Colour::fromRGB (78, 222, 255);
        case 3: return juce::Colour::fromRGB (238, 82, 255);
        case 4: return juce::Colour::fromRGB (255, 235, 76);
        default: return defaultAcid;
    }
}

juce::String AcidLab303AudioProcessorEditor::getThemeName() const
{
    switch (getDistortionMode())
    {
        case 1: return "BITE";
        case 2: return "FOLD";
        case 3: return "FUZZ";
        case 4: return "ZAP";
        default: return "WARM";
    }
}

void AcidLab303AudioProcessorEditor::selectStep (int index)
{
    selectedStep = juce::jlimit (0, acidlab::maxSteps - 1, index);
    refreshStepControls();
    stepGrid.repaint();
}

void AcidLab303AudioProcessorEditor::refreshStepControls()
{
    updatingStepControls = true;
    const auto step = processor.getStep (selectedStep);
    stepEditLabel.setText ("STEP " + juce::String (selectedStep + 1), juce::dontSendNotification);
    stepNoteSlider.setValue (step.note, juce::dontSendNotification);
    stepOctaveSlider.setValue (step.octave, juce::dontSendNotification);
    stepAccentButton.setToggleState (step.accent, juce::dontSendNotification);
    stepSlideButton.setToggleState (step.slide, juce::dontSendNotification);
    stepGateButton.setToggleState (step.gate, juce::dontSendNotification);
    stepRestButton.setToggleState (step.rest, juce::dontSendNotification);
    updatingStepControls = false;
}

void AcidLab303AudioProcessorEditor::commitSelectedStep()
{
    if (updatingStepControls)
        return;

    auto step = processor.getStep (selectedStep);
    step.note = static_cast<int> (stepNoteSlider.getValue());
    step.octave = static_cast<int> (stepOctaveSlider.getValue());
    step.accent = stepAccentButton.getToggleState();
    step.slide = stepSlideButton.getToggleState();
    step.gate = stepGateButton.getToggleState();
    step.rest = stepRestButton.getToggleState();
    if (step.rest)
        step.gate = false;
    processor.setStep (selectedStep, step);
    markDirty();
    refreshStepControls();
    stepGrid.repaint();
}

void AcidLab303AudioProcessorEditor::refreshPresetList()
{
    allPresets = processor.scanAllPresets (presetSearchBox.getText(), presetCategoryBox.getText());
    visiblePresetIndices.clear();
    presetBox.clear (juce::dontSendNotification);

    for (int i = 0; i < allPresets.size(); ++i)
    {
        const auto& preset = allPresets.getReference (i);
        const auto source = presetSourceBox.getText();
        if (! source.isEmpty() && source != "ALL" && ! preset.source.equalsIgnoreCase (source))
            continue;

        visiblePresetIndices.add (i);
        presetBox.addItem (preset.name + "  " + preset.source, visiblePresetIndices.size());
    }

    if (presetBox.getNumItems() > 0)
        presetBox.setSelectedId (1, juce::dontSendNotification);
}

void AcidLab303AudioProcessorEditor::loadSelectedPreset()
{
    const auto selected = presetBox.getSelectedId() - 1;
    if (! juce::isPositiveAndBelow (selected, visiblePresetIndices.size()))
        return;

    const auto presetIndex = visiblePresetIndices.getReference (selected);
    const auto& preset = allPresets.getReference (presetIndex);
    const auto error = processor.loadPresetFromFile (preset.file);
    if (error.isNotEmpty())
        return;

    currentPreset = preset;
    hasCurrentPreset = true;
    setDirty (false);
    refreshStepControls();
    stepGrid.repaint();
}

void AcidLab303AudioProcessorEditor::saveCurrentPreset()
{
    if (hasCurrentPreset && currentPreset.source.equalsIgnoreCase ("User") && currentPreset.file != juce::File{})
    {
        savePresetToUserFile (currentPreset.file);
        return;
    }

    savePresetAs();
}

void AcidLab303AudioProcessorEditor::savePresetAs()
{
    const auto defaultName = hasCurrentPreset ? currentPreset.name : juce::String ("AcidLab303");
    const auto defaultPreset = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                   .getChildFile ("AcidLab303")
                                   .getChildFile ("Presets")
                                   .getChildFile (defaultName + ".acidpreset.json");
    fileChooser = std::make_unique<juce::FileChooser> ("Save AcidLab303 preset",
                                                       defaultPreset,
                                                       "*.acidpreset.json");
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& chooser)
                              {
                                  auto file = chooser.getResult();
                                  if (file == juce::File{})
                                      return;
                                  savePresetToUserFile (file);
                              });
}

void AcidLab303AudioProcessorEditor::savePresetToUserFile (juce::File file)
{
    if (! file.getFileName().endsWithIgnoreCase (".acidpreset.json"))
        file = file.withFileExtension (".acidpreset.json");

    const auto name = file.getFileName().dropLastCharacters (juce::String (".acidpreset.json").length());
    const auto category = hasCurrentPreset && currentPreset.category.isNotEmpty() ? currentPreset.category : juce::String ("User");
    const auto author = hasCurrentPreset && currentPreset.author.isNotEmpty() ? currentPreset.author : juce::String ("User");
    const auto description = hasCurrentPreset && currentPreset.description.isNotEmpty() ? currentPreset.description : juce::String ("Saved from AcidLab303");
    if (! processor.savePresetToFile (file, name, category, author, description))
        return;

    currentPreset.name = name;
    currentPreset.category = category;
    currentPreset.author = author;
    currentPreset.description = description;
    currentPreset.source = "User";
    currentPreset.file = file;
    hasCurrentPreset = true;
    setDirty (false);
    refreshPresetList();
    for (int i = 0; i < visiblePresetIndices.size(); ++i)
    {
        if (allPresets.getReference (visiblePresetIndices.getReference (i)).file == file)
        {
            presetBox.setSelectedId (i + 1, juce::dontSendNotification);
            break;
        }
    }
}

void AcidLab303AudioProcessorEditor::markDirty()
{
    if (hasCurrentPreset)
        setDirty (true);
}

void AcidLab303AudioProcessorEditor::setDirty (bool shouldBeDirty)
{
    presetDirty = shouldBeDirty;
    updatePresetMetaLabel();
}

void AcidLab303AudioProcessorEditor::updatePresetMetaLabel()
{
    if (! hasCurrentPreset)
    {
        presetMetaLabel.setText ("No preset loaded\nCustom changes can be saved as a User preset.", juce::dontSendNotification);
        return;
    }

    juce::String metadataText;
    metadataText << currentPreset.name << (presetDirty ? " *" : "") << "\n";
    metadataText << currentPreset.source << " / " << currentPreset.category << "\n";
    if (currentPreset.author.isNotEmpty())
        metadataText << "by " << currentPreset.author << "\n";
    if (currentPreset.description.isNotEmpty())
        metadataText << "\n" << currentPreset.description;
    presetMetaLabel.setText (metadataText, juce::dontSendNotification);
}

void AcidLab303AudioProcessorEditor::selectTab (int index)
{
    currentTab = juce::jlimit (0, 4, index);
    updateTabButtons();
    updateVisibleTab();
}

void AcidLab303AudioProcessorEditor::updateTabButtons()
{
    juce::TextButton* buttons[] = { &synthTabButton, &sequencerTabButton, &modTabButton, &effectsTabButton, &analogTabButton };
    for (int i = 0; i < 5; ++i)
        buttons[i]->setToggleState (i == currentTab, juce::dontSendNotification);
}

void AcidLab303AudioProcessorEditor::updateVisibleTab()
{
    resized();
    repaint();
}

void AcidLab303AudioProcessorEditor::savePatternBank()
{
    const auto defaultPattern = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                    .getChildFile ("AcidLab303")
                                    .getChildFile ("Patterns")
                                    .getChildFile ("AcidLab303.acidpattern.json");
    fileChooser = std::make_unique<juce::FileChooser> ("Save AcidLab303 pattern bank", defaultPattern, "*.acidpattern.json");
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& chooser)
                              {
                                  auto file = chooser.getResult();
                                  if (file == juce::File{})
                                      return;
                                  if (! file.hasFileExtension (".json"))
                                      file = file.withFileExtension (".acidpattern.json");
                                  processor.savePatternBankToFile (file, file.getFileNameWithoutExtension(), "User", "Saved from AcidLab303");
                              });
}

void AcidLab303AudioProcessorEditor::loadPatternBank()
{
    const auto defaultPattern = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                    .getChildFile ("AcidLab303")
                                    .getChildFile ("Patterns");
    fileChooser = std::make_unique<juce::FileChooser> ("Load AcidLab303 pattern bank", defaultPattern, "*.acidpattern.json");
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& chooser)
                              {
                                  const auto file = chooser.getResult();
                                  if (file != juce::File{} && processor.loadPatternBankFromFile (file))
                                  {
                                      markDirty();
                                      refreshStepControls();
                                      stepGrid.repaint();
                                  }
                              });
}
