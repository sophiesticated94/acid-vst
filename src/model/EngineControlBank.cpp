#include "EngineControlBank.h"

namespace acidlab
{
EngineControlBank::EngineControlBank (juce::AudioProcessorValueTreeState& state) noexcept
    : apvts (state)
{
}

EngineControls EngineControlBank::getControls (int engineMode) const noexcept
{
    const auto safeMode = juce::jlimit (0, engineCount - 1, engineMode);
    const auto defaults = getEngineDescriptor (safeMode).defaults;
    EngineControls controls = defaults;
    if (auto* value = apvts.getRawParameterValue (parameterId (safeMode, 0))) controls.a = value->load();
    if (auto* value = apvts.getRawParameterValue (parameterId (safeMode, 1))) controls.b = value->load();
    if (auto* value = apvts.getRawParameterValue (parameterId (safeMode, 2))) controls.c = value->load();
    return controls;
}

void EngineControlBank::setControl (int engineMode, int controlIndex, float value)
{
    const auto safeMode = juce::jlimit (0, engineCount - 1, engineMode);
    const auto safeControl = juce::jlimit (0, engineControlCount - 1, controlIndex);
    if (auto* parameter = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (parameterId (safeMode, safeControl))))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, value));
        parameter->endChangeGesture();
    }
}

juce::String EngineControlBank::parameterId (int engineMode, int controlIndex)
{
    const auto safeMode = juce::jlimit (0, engineCount - 1, engineMode);
    const auto suffix = static_cast<char> ('A' + juce::jlimit (0, engineControlCount - 1, controlIndex));
    return "engine" + juce::String (getEngineDescriptor (safeMode).id) + juce::String::charToString (suffix);
}
}
