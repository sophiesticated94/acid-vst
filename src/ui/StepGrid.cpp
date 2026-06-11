#include "StepGrid.h"

#include "AcidLookAndFeel.h"
#include "PluginEditor.h"

StepGrid::StepGrid (AcidLab303AudioProcessorEditor& owner)
    : editor (owner)
{
}

void StepGrid::paint (juce::Graphics& g)
{
    g.fillAll (acidlab::ui::panel);
    const auto bounds = getLocalBounds().reduced (8);
    const auto cellW = bounds.getWidth() / 16;
    const auto cellH = bounds.getHeight() / 4;
    const auto current = editor.processor.getCurrentStep();

    for (int i = 0; i < acidlab::maxSteps; ++i)
    {
        const auto row = i / 16;
        const auto col = i % 16;
        auto cell = juce::Rectangle<int> (bounds.getX() + col * cellW,
                                          bounds.getY() + row * cellH,
                                          cellW,
                                          cellH).reduced (3);
        const auto step = editor.processor.getStep (i);
        auto colour = step.rest ? acidlab::ui::muted.darker (0.45f) : acidlab::ui::panelBright;
        if (step.accent)
            colour = colour.interpolatedWith (acidlab::ui::amber, 0.45f);
        if (step.slide)
            colour = colour.interpolatedWith (editor.getThemeColour(), 0.25f);
        if (i == current)
            colour = editor.getThemeColour();

        g.setColour (colour);
        g.fillRoundedRectangle (cell.toFloat(), 4.0f);
        g.setColour (i == current ? acidlab::ui::text : (i == editor.selectedStep ? acidlab::ui::amber : acidlab::ui::background.withAlpha (0.7f)));
        g.drawRoundedRectangle (cell.toFloat(), 4.0f, i == current ? 3.0f : (i == editor.selectedStep ? 2.0f : 1.0f));

        g.setColour (i == current ? acidlab::ui::background : acidlab::ui::text);
        g.setFont (11.0f);
        g.drawFittedText (juce::String (i + 1), cell.removeFromTop (cell.getHeight() / 2), juce::Justification::centred, 1);

        juce::String symbols;
        if (step.rest)
            symbols << "R";
        else
        {
            if (step.accent) symbols << "A";
            if (step.slide) symbols << "S";
        }
        g.setFont (10.0f);
        g.drawFittedText (symbols, cell, juce::Justification::centred, 1);
    }
}

void StepGrid::mouseDown (const juce::MouseEvent& event)
{
    const auto bounds = getLocalBounds().reduced (8);
    const auto cellW = bounds.getWidth() / 16;
    const auto cellH = bounds.getHeight() / 4;
    if (cellW <= 0 || cellH <= 0)
        return;

    const auto col = juce::jlimit (0, 15, (event.x - bounds.getX()) / cellW);
    const auto row = juce::jlimit (0, 3, (event.y - bounds.getY()) / cellH);
    editor.selectStep (row * 16 + col);
}
