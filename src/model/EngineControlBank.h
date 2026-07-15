#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/EngineDescriptor.h"

namespace acidlab
{
class EngineControlBank
{
public:
    explicit EngineControlBank (juce::AudioProcessorValueTreeState& state) noexcept;

    EngineControls getControls (int engineMode) const noexcept;
    void setControl (int engineMode, int controlIndex, float value);
    static juce::String parameterId (int engineMode, int controlIndex);

private:
    juce::AudioProcessorValueTreeState& apvts;
};
}
