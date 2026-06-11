#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace acidlab::ui
{
inline const auto background = juce::Colour::fromRGB (13, 15, 15);
inline const auto panel = juce::Colour::fromRGB (27, 31, 30);
inline const auto panelBright = juce::Colour::fromRGB (38, 44, 42);
inline const auto amber = juce::Colour::fromRGB (255, 190, 55);
inline const auto text = juce::Colour::fromRGB (218, 226, 210);
inline const auto muted = juce::Colour::fromRGB (115, 126, 116);
inline const auto defaultAcid = juce::Colour::fromRGB (185, 255, 72);
}

class AcidLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;
};
