#include "MidiPatternExporter.h"

namespace acidlab
{
bool MidiPatternExporter::writePatternToStream (juce::OutputStream& stream,
                                                const PatternBank::Pattern& pattern,
                                                const Parameters& parameters)
{
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote (960);
    juce::MidiMessageSequence sequence;

    const auto steps = juce::jlimit (1, maxSteps, parameters.patternLength);
    const auto ticksPerStep = ticksPerStepForRate (parameters.seqRate);
    for (int i = 0; i < steps; ++i)
    {
        const auto& step = pattern[static_cast<size_t> (i)];
        if (step.rest || ! step.gate)
            continue;

        const auto note = juce::jlimit (0, 127, 36 + step.note + step.octave * 12 + static_cast<int> (std::round (parameters.tune)));
        const auto tick = i * ticksPerStep;
        const auto length = step.slide ? ticksPerStep : juce::jmax (60, ticksPerStep - 40);
        const auto velocity = static_cast<juce::uint8> (step.accent ? 118 : 92);
        sequence.addEvent (juce::MidiMessage::noteOn (1, note, velocity), tick);
        sequence.addEvent (juce::MidiMessage::noteOff (1, note), tick + length);
    }

    sequence.updateMatchedPairs();
    midiFile.addTrack (sequence);
    midiFile.writeTo (stream);
    return true;
}

bool MidiPatternExporter::writePatternToFile (const juce::File& file,
                                              const PatternBank::Pattern& pattern,
                                              const Parameters& parameters)
{
    juce::FileOutputStream stream (file);
    if (! stream.openedOk())
        return false;

    return writePatternToStream (stream, pattern, parameters);
}

int MidiPatternExporter::ticksPerStepForRate (int seqRate) noexcept
{
    return seqRate == 0 ? 960 : (seqRate == 2 ? 240 : (seqRate == 3 ? 320 : 480));
}
} // namespace acidlab
