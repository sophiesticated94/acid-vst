#include "AcidEngine.h"

#include <algorithm>

namespace acidlab
{
namespace
{
constexpr float pi = 3.14159265358979323846f;
}

void AcidEngine::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    filter.prepare (sampleRate);
    delayLine.prepare (sampleRate);
    reverbLineA.prepare (sampleRate);
    reverbLineB.prepare (sampleRate);
    reset();
}

void AcidEngine::reset()
{
    phase = 0.0f;
    subPhase = 0.0f;
    currentFrequency = 110.0f;
    targetFrequency = 110.0f;
    activeVelocity = 0.0f;
    env = 0.0f;
    amp = 0.0f;
    accent = 0.0f;
    accentPulse = 0.0f;
    outputLevel = 0.0f;
    accentMeter = 0.0f;
    gateOpen = false;
    slideActive = false;
    externalHeld = false;
    samplesUntilNextStep = 0.0;
    wasHostPlaying = false;
    currentStep = -1;
    toneFilter.reset();
    bassFilter.reset();
    fxToneFilter.reset();
    modFilter.reset();
    filter.reset();
    acidFilter.reset();
    delayLine.reset();
    reverbLineA.reset();
    reverbLineB.reset();
}

void AcidEngine::noteOn (int midiNote, float velocity, bool slide, bool accented)
{
    targetFrequency = midiNoteToHz (static_cast<float> (midiNote));

    if (! slide || ! gateOpen)
        currentFrequency = targetFrequency;

    activeVelocity = clamp (velocity, 0.0f, 1.0f);
    accent = accented ? 1.0f : 0.0f;
    accentPulse = accent;
    env = 1.0f;
    gateOpen = true;
    slideActive = slide;
    externalHeld = true;
}

void AcidEngine::noteOff()
{
    externalHeld = false;
    gateOpen = false;
}

void AcidEngine::render (float* left,
                         float* right,
                         int numSamples,
                         const Parameters& parameters,
                         const std::array<Step, maxSteps>& pattern,
                         bool hostPlaying)
{
    const auto patternLength = std::max (1, std::min (parameters.patternLength, maxSteps));
    const auto samplesPerStep = getSamplesPerStep (parameters);

    if (parameters.sequencerEnabled && hostPlaying && ! wasHostPlaying)
    {
        currentStep = -1;
        samplesUntilNextStep = 0.0;
    }

    wasHostPlaying = hostPlaying;

    for (int i = 0; i < numSamples; ++i)
    {
        if (parameters.sequencerEnabled && hostPlaying)
        {
            if (samplesUntilNextStep <= 0.0)
            {
                currentStep = (currentStep + 1) % patternLength;
                triggerStep (pattern[static_cast<size_t> (currentStep)], parameters);
                samplesUntilNextStep += samplesPerStep;
            }

            samplesUntilNextStep -= 1.0;
        }

        const auto sample = renderSample (parameters);
        left[i] += sample;
        right[i] += sample;
    }
}

void AcidEngine::triggerStep (const Step& step, const Parameters& parameters)
{
    if (step.rest || ! step.gate)
    {
        gateOpen = false;
        slideActive = false;
        accent = 0.0f;
        return;
    }

    const auto midiNote = 36 + step.note + step.octave * 12 + static_cast<int> (std::round (parameters.tune));
    targetFrequency = midiNoteToHz (static_cast<float> (midiNote));

    if (! step.slide || ! gateOpen)
        currentFrequency = targetFrequency;

    env = 1.0f;
    activeVelocity = 0.9f;
    accent = step.accent ? 1.0f : 0.0f;
    accentPulse = accent;
    gateOpen = true;
    slideActive = step.slide;
}

float AcidEngine::renderSample (const Parameters& parameters)
{
    const auto osFactor = getOversamplingFactor (parameters.oversampling);
    const auto effectiveSampleRate = sampleRate * static_cast<double> (osFactor);

    auto sum = 0.0f;
    for (int i = 0; i < osFactor; ++i)
        sum += renderVoiceSubSample (parameters, effectiveSampleRate);

    auto sample = sum / static_cast<float> (osFactor);
    sample = applyEffects (sample, parameters);
    sample = softClip (sample * parameters.volume * 0.72f);

    outputLevel += 0.0065f * (std::abs (sample) - outputLevel);
    accentMeter += 0.008f * (accentPulse - accentMeter);
    accentPulse *= std::exp (-1.0f / (0.09f * static_cast<float> (sampleRate)));

    return sample;
}

float AcidEngine::renderVoiceSubSample (const Parameters& parameters, double effectiveSampleRate)
{
    const auto glideSeconds = slideActive ? std::max (0.005f, parameters.slideTime) : 0.0015f;
    const auto glideCoefficient = 1.0f - std::exp (-1.0f / (glideSeconds * static_cast<float> (effectiveSampleRate)));
    currentFrequency += glideCoefficient * (targetFrequency - currentFrequency);

    const auto driftDepth = parameters.drift * 0.0015f;
    const auto drift = 1.0f + driftDepth * std::sin (phase * 0.017f + 1.7f);
    const auto frequency = clamp (currentFrequency * drift, 8.0f, 18000.0f);
    const auto phaseInc = clamp (frequency / static_cast<float> (effectiveSampleRate), 0.0f, 0.45f);
    const auto subInc = clamp ((frequency * 0.5f) / static_cast<float> (effectiveSampleRate), 0.0f, 0.45f);

    auto saw = 2.0f * phase - 1.0f;
    saw -= polyBlep (phase, phaseInc);

    const auto pulseWidth = clamp (parameters.pulseWidth, 0.08f, 0.92f);
    auto pulse = phase < pulseWidth ? 1.0f : -1.0f;
    pulse += polyBlep (phase, phaseInc);
    auto phase2 = phase - pulseWidth;
    if (phase2 < 0.0f)
        phase2 += 1.0f;
    pulse -= polyBlep (phase2, phaseInc);

    phase += phaseInc;
    if (phase >= 1.0f)
        phase -= 1.0f;

    const auto sub = subPhase < 0.5f ? 1.0f : -1.0f;
    subPhase += subInc;
    if (subPhase >= 1.0f)
        subPhase -= 1.0f;

    const auto waveMix = clamp (parameters.waveform, 0.0f, 1.0f);
    auto oscillator = saw * (1.0f - waveMix) + pulse * waveMix;
    oscillator += parameters.subLevel * 0.55f * sub;
    oscillator += parameters.noise * nextRandomBipolar();

    const auto decaySeconds = std::max (0.035f, parameters.decay);
    const auto envCoefficient = std::exp (-1.0f / (decaySeconds * static_cast<float> (effectiveSampleRate)));
    env *= envCoefficient;

    const auto accentBoost = 1.0f + accent * parameters.accentAmount * (1.0f + parameters.accentAttack);
    const auto envCutoff = parameters.envMod * accentBoost * env * 4200.0f;
    const auto tracking = parameters.filterTracking * currentFrequency * 1.5f;
    const auto cutoff = clamp (parameters.cutoff + envCutoff + tracking, 32.0f, 18500.0f);
    const auto filterDrive = 1.0f + parameters.drive * 4.0f + accent * 0.8f;
    auto filtered = parameters.filterModel == 0
                        ? filter.process (oscillator, cutoff, parameters.resonance, filterDrive)
                        : acidFilter.process (oscillator, cutoff, parameters.resonance, filterDrive, effectiveSampleRate);

    const auto bass = bassFilter.process (filtered, 0.018f);
    filtered += bass * parameters.bassBoost * 1.5f;

    const auto preDrive = 1.0f + parameters.drive * 18.0f;
    const auto distorted = applyDistortion (filtered * preDrive, parameters.distMode) * driveCompensation (parameters.distMode);
    const auto toneCoefficient = clamp (0.025f + parameters.distTone * 0.45f, 0.025f, 0.475f);
    const auto toned = toneFilter.process (distorted, toneCoefficient);
    filtered = filtered * (1.0f - parameters.distMix) + toned * parameters.distMix;

    const auto ampTarget = gateOpen ? activeVelocity * accentBoost : 0.0f;
    const auto ampCoefficient = gateOpen ? (0.08f + parameters.accentAttack * 0.22f) : 0.0035f;
    amp += ampCoefficient * (ampTarget - amp);

    const auto clipLevel = clamp (parameters.clipLevel, 0.25f, 1.0f);
    return softClip (filtered * amp * 0.8f / clipLevel) * clipLevel;
}

float AcidEngine::midiNoteToHz (float midiNote) noexcept
{
    return 440.0f * std::pow (2.0f, (midiNote - 69.0f) / 12.0f);
}

float AcidEngine::polyBlep (float t, float dt) noexcept
{
    if (dt <= 0.0f)
        return 0.0f;

    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }

    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }

    return 0.0f;
}

float AcidEngine::softClip (float x) noexcept
{
    return std::tanh (x);
}

float AcidEngine::applyDistortion (float x, int mode) noexcept
{
    switch (mode)
    {
        case 1: // Bite: asymmetric diode-ish clipping.
        {
            const auto biased = x + 0.22f;
            return std::tanh (biased * 1.35f) - std::tanh (0.22f * 1.35f);
        }

        case 2: // Fold: energized wavefolder for liquid squelch.
            return std::tanh (fold (x * 0.72f) * 1.65f);

        case 3: // Fuzz: compressed, hairy square-ish edge.
        {
            const auto shaped = x >= 0.0f ? 1.0f - std::exp (-std::abs (x))
                                          : -1.0f + std::exp (-std::abs (x));
            return std::tanh (shaped * 1.4f);
        }

        case 4: // Zap: brighter electrical crackle without random artifacts.
        {
            const auto clipped = std::tanh (x * 1.8f);
            const auto sparkle = std::sin (clipped * 5.0f) * 0.16f;
            return std::tanh (clipped + sparkle);
        }

        default: // Warm: rounded analog overdrive.
            return std::tanh (x);
    }
}

float AcidEngine::driveCompensation (int mode) noexcept
{
    switch (mode)
    {
        case 1: return 0.82f;
        case 2: return 0.76f;
        case 3: return 0.68f;
        case 4: return 0.62f;
        default: return 0.88f;
    }
}

float AcidEngine::fold (float x) noexcept
{
    x = std::fmod (x + 1.0f, 4.0f);
    if (x < 0.0f)
        x += 4.0f;

    return x <= 2.0f ? x - 1.0f : 3.0f - x;
}

float AcidEngine::clamp (float value, float low, float high) noexcept
{
    return std::max (low, std::min (value, high));
}

float AcidEngine::applyEffects (float input, const Parameters& parameters)
{
    auto sample = input;

    const auto toneTarget = parameters.fxTone < 0.5f ? sample * (0.65f + parameters.fxTone * 0.7f)
                                                     : sample + (sample - fxToneFilter.process (sample, 0.12f)) * (parameters.fxTone - 0.5f) * 1.8f;
    sample = toneTarget;

    if (parameters.fxModOn && parameters.fxModDepth > 0.001f)
    {
        const auto modded = modFilter.process (sample, 0.04f + parameters.fxModDepth * 0.12f);
        sample = sample * (1.0f - parameters.fxModDepth * 0.45f) + modded * parameters.fxModDepth * 0.45f;
    }

    if (parameters.fxDelayOn && parameters.fxDelayMix > 0.001f)
    {
        const auto delayed = delayLine.process (sample, parameters.fxDelayTime, 0.32f + parameters.fxDelayMix * 0.28f, parameters.fxTone);
        sample = sample * (1.0f - parameters.fxDelayMix) + delayed * parameters.fxDelayMix;
    }

    if (parameters.fxReverbOn && parameters.fxReverbMix > 0.001f)
    {
        const auto verbA = reverbLineA.process (sample, 0.083f, 0.58f, 0.42f);
        const auto verbB = reverbLineB.process (sample + verbA * 0.35f, 0.127f, 0.54f, 0.38f);
        const auto wet = (verbA + verbB) * 0.5f;
        sample = sample * (1.0f - parameters.fxReverbMix) + wet * parameters.fxReverbMix;
    }

    return sample;
}

int AcidEngine::getOversamplingFactor (int choice) const noexcept
{
    if (choice >= 2)
        return 4;
    if (choice == 1)
        return 2;
    return 1;
}

double AcidEngine::getSamplesPerStep (const Parameters& parameters) const noexcept
{
    const auto bpm = std::max (20.0, parameters.bpm);
    auto divisor = 4.0;
    switch (parameters.seqRate)
    {
        case 0: divisor = 2.0; break;
        case 2: divisor = 8.0; break;
        case 3: divisor = 6.0; break;
        default: divisor = 4.0; break;
    }

    return sampleRate * 60.0 / bpm / divisor;
}

float AcidEngine::nextRandomBipolar() noexcept
{
    randomState = randomState * 1664525u + 1013904223u;
    const auto normalized = static_cast<float> ((randomState >> 8) & 0x00ffffffu) / static_cast<float> (0x00ffffffu);
    return normalized * 2.0f - 1.0f;
}

void AcidEngine::AcidLadder::reset()
{
    stage.fill (0.0f);
}

float AcidEngine::AcidLadder::process (float input, float cutoffHz, float resonance, float drive, double effectiveSampleRate)
{
    const auto cutoff = clamp (cutoffHz, 20.0f, static_cast<float> (effectiveSampleRate * 0.42));
    const auto g = clamp (1.0f - std::exp (-2.0f * pi * cutoff / static_cast<float> (effectiveSampleRate)), 0.001f, 0.92f);
    const auto feedback = clamp (resonance, 0.0f, 0.98f) * 4.15f;
    auto x = std::tanh ((input - stage[3] * feedback) * drive);

    for (auto& s : stage)
    {
        const auto diode = std::tanh ((x - s) * (1.15f + drive * 0.12f));
        s += g * diode;
        x = s;
    }

    return std::tanh ((stage[3] - stage[0] * 0.08f) * 1.28f);
}

void AcidEngine::DelayLine::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    buffer.resize (static_cast<size_t> (std::ceil (sampleRate * 2.0)));
    reset();
}

void AcidEngine::DelayLine::reset()
{
    std::fill (buffer.begin(), buffer.end(), 0.0f);
    writeIndex = 0;
    toneFilter.reset();
}

float AcidEngine::DelayLine::process (float input, float timeSeconds, float feedback, float tone)
{
    if (buffer.empty())
        return 0.0f;

    const auto maxDelay = static_cast<int> (buffer.size()) - 1;
    const auto delaySamples = static_cast<int> (clamp (timeSeconds, 0.01f, 1.5f) * static_cast<float> (sampleRate));
    const auto readIndex = (writeIndex + maxDelay - std::min (delaySamples, maxDelay)) % static_cast<int> (buffer.size());
    auto delayed = buffer[static_cast<size_t> (readIndex)];
    delayed = toneFilter.process (delayed, clamp (0.04f + tone * 0.24f, 0.04f, 0.28f));
    buffer[static_cast<size_t> (writeIndex)] = softClip (input + delayed * clamp (feedback, 0.0f, 0.86f));
    writeIndex = (writeIndex + 1) % static_cast<int> (buffer.size());
    return delayed;
}

void AcidEngine::TptLowpass::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
}

void AcidEngine::TptLowpass::reset()
{
    ic1eq = 0.0f;
    ic2eq = 0.0f;
}

float AcidEngine::TptLowpass::process (float input, float cutoffHz, float resonance, float nonlinearDrive)
{
    const auto cutoff = clamp (cutoffHz, 20.0f, static_cast<float> (sampleRate * 0.45));
    const auto g = std::tan (pi * cutoff / static_cast<float> (sampleRate));
    const auto r = 1.05f - clamp (resonance, 0.0f, 0.98f) * 0.98f;
    const auto h = 1.0f / (1.0f + 2.0f * r * g + g * g);

    const auto driven = std::tanh (input * nonlinearDrive);
    const auto v3 = driven - ic2eq;
    const auto v1 = h * (g * v3 + ic1eq);
    const auto v2 = ic2eq + g * v1;
    ic1eq = 2.0f * v1 - ic1eq;
    ic2eq = 2.0f * v2 - ic2eq;

    return std::tanh (v2 * 1.15f);
}
} // namespace acidlab
