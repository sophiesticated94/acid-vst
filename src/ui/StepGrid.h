#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AcidLab303AudioProcessorEditor;

class StepGrid final : public juce::Component
{
public:
    explicit StepGrid (AcidLab303AudioProcessorEditor& owner);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    AcidLab303AudioProcessorEditor& editor;
};
