#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AcidLab303AudioProcessorEditor;

class EnergyStrip final : public juce::Component
{
public:
    explicit EnergyStrip (AcidLab303AudioProcessorEditor& owner);
    void paint (juce::Graphics&) override;

private:
    AcidLab303AudioProcessorEditor& editor;
};
