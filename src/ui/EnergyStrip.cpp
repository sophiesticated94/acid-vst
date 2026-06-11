#include "EnergyStrip.h"

#include "AcidLookAndFeel.h"
#include "PluginEditor.h"

#include <cmath>

EnergyStrip::EnergyStrip (AcidLab303AudioProcessorEditor& owner)
    : editor (owner)
{
}

void EnergyStrip::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    const auto theme = editor.getThemeColour();
    const auto mode = editor.getDistortionMode();
    const auto level = juce::jlimit (0.0f, 1.0f, editor.processor.getOutputLevel() * 3.0f);
    const auto accent = juce::jlimit (0.0f, 1.0f, editor.processor.getAccentLevel());
    const auto intensity = 0.28f + 0.11f * static_cast<float> (mode) + level * 0.55f + accent * 0.5f;
    const auto time = static_cast<float> (juce::Time::getMillisecondCounterHiRes() * 0.001);

    g.setColour (acidlab::ui::panel);
    g.fillRoundedRectangle (bounds, 5.0f);

    juce::ColourGradient glow (theme.withAlpha (0.42f), bounds.getX(), bounds.getCentreY(),
                               acidlab::ui::background.withAlpha (0.0f), bounds.getRight(), bounds.getCentreY(), false);
    g.setGradientFill (glow);
    g.fillRoundedRectangle (bounds.reduced (3.0f), 4.0f);

    juce::Path bolt;
    const auto startX = bounds.getX() + 14.0f;
    const auto midY = bounds.getCentreY();
    bolt.startNewSubPath (startX, midY);

    constexpr int segments = 13;
    for (int i = 1; i <= segments; ++i)
    {
        const auto t = static_cast<float> (i) / static_cast<float> (segments);
        const auto x = startX + t * (bounds.getWidth() - 28.0f);
        const auto wobble = std::sin (time * (2.4f + mode * 0.31f) + t * 31.0f) * bounds.getHeight() * 0.18f;
        const auto jag = std::sin (time * 9.0f + t * 67.0f + mode) * bounds.getHeight() * 0.11f;
        bolt.lineTo (x, midY + wobble + jag);
    }

    g.setColour (theme.withAlpha (0.18f + intensity * 0.2f));
    g.strokePath (bolt, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (theme.withAlpha (0.62f + juce::jlimit (0.0f, 0.32f, intensity * 0.22f)));
    g.strokePath (bolt, juce::PathStrokeType (2.2f + intensity, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (acidlab::ui::text.withAlpha (0.88f));
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText ("DISTORTION  " + editor.getThemeName(), getLocalBounds().reduced (14, 0), juce::Justification::centredLeft);
}
