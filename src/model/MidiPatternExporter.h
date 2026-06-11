#pragma once

#include "PatternBank.h"
#include "../dsp/AcidEngine.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

namespace acidlab
{
class MidiPatternExporter
{
public:
    static bool writePatternToStream (juce::OutputStream& stream,
                                      const PatternBank::Pattern& pattern,
                                      const Parameters& parameters);

    static bool writePatternToFile (const juce::File& file,
                                    const PatternBank::Pattern& pattern,
                                    const Parameters& parameters);

    static int ticksPerStepForRate (int seqRate) noexcept;
};
} // namespace acidlab
