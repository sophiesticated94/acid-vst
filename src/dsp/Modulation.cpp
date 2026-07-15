#include "Modulation.h"

#include <algorithm>
#include <cmath>

namespace acidlab
{
namespace
{
constexpr float pi = 3.14159265358979323846f;

float clamp (float value, float low, float high) noexcept
{
    return std::max (low, std::min (value, high));
}
}

void ModulationEngine::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
}

void ModulationEngine::reset()
{
    phases.fill (0.0f);
    randomValues.fill (0.0f);
    envelopes.fill ({ });
}

void ModulationEngine::noteOn()
{
    for (auto& envelope : envelopes)
        envelope.stage = EnvelopeState::Stage::attack;
}

void ModulationEngine::noteOff()
{
    for (auto& envelope : envelopes)
        envelope.stage = EnvelopeState::Stage::release;
}

std::array<float, modulationSourceCount> ModulationEngine::next (const ModulationParameters& parameters, double bpm)
{
    std::array<float, modulationSourceCount> values {};
    for (int index = 0; index < lfoCount; ++index)
        values[static_cast<size_t> (index)] = nextLfo (index, parameters.lfos[static_cast<size_t> (index)], bpm);
    for (int index = 0; index < envelopeCount; ++index)
        values[static_cast<size_t> (lfoCount + index)] = nextEnvelope (index, parameters.envelopes[static_cast<size_t> (index)]);
    return values;
}

float ModulationEngine::nextLfo (int index, const LfoParameters& parameters, double bpm)
{
    const auto rate = parameters.sync ? syncRateHz (parameters.division, bpm) : clamp (parameters.rateHz, 0.01f, 30.0f);
    auto& phase = phases[static_cast<size_t> (index)];
    phase += rate / static_cast<float> (sampleRate);
    if (phase >= 1.0f)
    {
        phase -= 1.0f;
        randomState = randomState * 1664525u + 1013904223u;
        randomValues[static_cast<size_t> (index)] = static_cast<float> ((randomState >> 8) & 0xffffu) / 32767.5f - 1.0f;
    }

    auto shapedPhase = phase + parameters.phase;
    shapedPhase -= std::floor (shapedPhase);
    float value = 0.0f;
    switch (std::max (0, std::min (6, parameters.shape)))
    {
        case 1: value = 1.0f - 4.0f * std::abs (shapedPhase - 0.5f); break;
        case 2: value = shapedPhase * 2.0f - 1.0f; break;
        case 3: value = 1.0f - shapedPhase * 2.0f; break;
        case 4: value = shapedPhase < 0.5f ? 1.0f : -1.0f; break;
        case 5: value = randomValues[static_cast<size_t> (index)]; break;
        case 6: value = std::sin (shapedPhase * pi * 2.0f) * 0.65f + randomValues[static_cast<size_t> (index)] * 0.35f; break;
        default: value = std::sin (shapedPhase * pi * 2.0f); break;
    }
    return value * clamp (parameters.depth, 0.0f, 1.0f);
}

float ModulationEngine::nextEnvelope (int index, const AdsrParameters& parameters)
{
    auto& envelope = envelopes[static_cast<size_t> (index)];
    const auto attack = std::max (0.001f, parameters.attack);
    const auto decay = std::max (0.001f, parameters.decay);
    const auto release = std::max (0.001f, parameters.release);
    const auto sustain = clamp (parameters.sustain, 0.0f, 1.0f);

    const auto approach = [this] (float current, float target, float seconds)
    {
        return current + (target - current) * (1.0f - std::exp (-1.0f / (seconds * static_cast<float> (sampleRate))));
    };

    switch (envelope.stage)
    {
        case EnvelopeState::Stage::attack:
            envelope.value = approach (envelope.value, 1.0f, attack);
            if (envelope.value >= 0.999f)
                envelope.stage = EnvelopeState::Stage::decay;
            break;
        case EnvelopeState::Stage::decay:
            envelope.value = approach (envelope.value, sustain, decay);
            if (std::abs (envelope.value - sustain) < 0.001f)
                envelope.stage = EnvelopeState::Stage::sustain;
            break;
        case EnvelopeState::Stage::sustain:
            envelope.value = sustain;
            break;
        case EnvelopeState::Stage::release:
            envelope.value = approach (envelope.value, 0.0f, release);
            if (envelope.value < 0.0001f)
            {
                envelope.value = 0.0f;
                envelope.stage = EnvelopeState::Stage::idle;
            }
            break;
        case EnvelopeState::Stage::idle:
            break;
    }

    return envelope.value * clamp (parameters.amount, 0.0f, 1.0f);
}

float ModulationEngine::syncRateHz (int division, double bpm) noexcept
{
    constexpr std::array<float, 12> beatsPerCycle { 32.0f, 16.0f, 8.0f, 4.0f, 2.0f, 1.0f, 0.5f, 1.0f / 3.0f, 0.25f, 1.0f / 6.0f, 0.125f, 1.0f / 12.0f };
    const auto safe = std::max (0, std::min (division, static_cast<int> (beatsPerCycle.size()) - 1));
    return static_cast<float> (std::max (20.0, bpm) / 60.0) / beatsPerCycle[static_cast<size_t> (safe)];
}
}
