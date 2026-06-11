#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AcidLab303AudioProcessorEditor;

class MidiDragButton final : public juce::TextButton,
                             public juce::DragAndDropContainer
{
public:
    MidiDragButton (AcidLab303AudioProcessorEditor& owner, const juce::String& text);
    void mouseDrag (const juce::MouseEvent& event) override;

private:
    AcidLab303AudioProcessorEditor& editor;
};
