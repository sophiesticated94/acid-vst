#include "AcidLookAndFeel.h"

void AcidLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                        int x,
                                        int y,
                                        int width,
                                        int height,
                                        float sliderPos,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider&)
{
    const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y), static_cast<float> (width), static_cast<float> (height)).reduced (7.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto fill = findColour (juce::Slider::rotarySliderFillColourId);

    g.setColour (acidlab::ui::panelBright);
    g.fillEllipse (bounds.withSizeKeepingCentre (radius * 2.0f, radius * 2.0f));
    g.setColour (acidlab::ui::background);
    g.fillEllipse (bounds.withSizeKeepingCentre (radius * 1.58f, radius * 1.58f));

    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, radius * 0.86f, radius * 0.86f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (fill);
    g.strokePath (arc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.addRoundedRectangle (-2.0f, -radius * 0.74f, 4.0f, radius * 0.38f, 2.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (acidlab::ui::amber);
    g.fillPath (pointer);

    g.setColour (acidlab::ui::muted.withAlpha (0.65f));
    g.drawEllipse (bounds.withSizeKeepingCentre (radius * 2.0f, radius * 2.0f), 1.0f);
}
