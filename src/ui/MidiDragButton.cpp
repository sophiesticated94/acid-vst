#include "MidiDragButton.h"

#include "PluginEditor.h"

MidiDragButton::MidiDragButton (AcidLab303AudioProcessorEditor& owner, const juce::String& text)
    : juce::TextButton (text),
      editor (owner)
{
}

void MidiDragButton::mouseDrag (const juce::MouseEvent& event)
{
    if (event.getDistanceFromDragStart() < 8)
        return;

    const auto file = editor.processor.createTemporaryMidiDragFile();
    if (file.existsAsFile())
        performExternalDragDropOfFiles ({ file.getFullPathName() }, true, this);
}
