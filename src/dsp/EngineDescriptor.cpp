#include "EngineDescriptor.h"

#include <algorithm>
#include <cmath>

namespace acidlab
{
const std::array<EngineDescriptor, engineCount>& getEngineDescriptors() noexcept
{
    static const std::array<EngineDescriptor, engineCount> descriptors {{
        { "Warm",   "Warm",   "Bias",       "Sag",       "Softness",    { 0.50f, 0.42f, 0.58f }, 0.88f, 185, 255,  72 },
        { "Bite",   "Bite",   "Bias",       "Edge",      "Clamp",       { 0.52f, 0.62f, 0.55f }, 0.82f, 255, 124,  62 },
        { "Fold",   "Fold",   "Fold",       "Symmetry",  "Feedback",    { 0.55f, 0.50f, 0.28f }, 0.76f,  78, 222, 255 },
        { "Fuzz",   "Fuzz",   "Texture",    "Gate",      "Compression", { 0.64f, 0.18f, 0.58f }, 0.68f, 238,  82, 255 },
        { "Zap",    "Zap",    "Spark",      "Voltage",   "Air",         { 0.54f, 0.66f, 0.56f }, 0.62f, 255, 235,  76 },
        { "Tube",   "Tube",   "Bias",       "Sag",       "Heat",        { 0.54f, 0.48f, 0.60f }, 0.82f, 255, 148,  84 },
        { "Crush",  "Crush",  "Bits",       "Rate",      "Jitter",      { 0.64f, 0.42f, 0.12f }, 0.70f, 106, 255, 193 },
        { "Melt",   "Melt",   "Drift",      "Smear",     "Heat",        { 0.34f, 0.52f, 0.48f }, 0.84f, 255, 111, 173 },
        { "Tape",   "Tape",   "Saturation", "Wow",       "Hiss",        { 0.48f, 0.32f, 0.08f }, 0.86f, 224, 181, 104 },
        { "Ring",   "Ring",   "Frequency",  "Sidebands", "Phase",       { 0.34f, 0.48f, 0.50f }, 0.72f, 145, 151, 255 },
        { "Shred",  "Shred",  "Threshold",  "Edge",      "Gate",        { 0.48f, 0.72f, 0.16f }, 0.62f, 255,  72,  92 }
    }};
    return descriptors;
}

const EngineDescriptor& getEngineDescriptor (int mode) noexcept
{
    const auto safeMode = std::max (0, std::min (mode, engineCount - 1));
    return getEngineDescriptors()[static_cast<size_t> (safeMode)];
}

float normalisedEngineValue (int mode) noexcept
{
    return static_cast<float> (std::max (0, std::min (mode, engineCount - 1))) / static_cast<float> (engineCount - 1);
}

float migrateLegacyEngineValue (float legacyNormalised) noexcept
{
    const auto legacyMode = std::max (0, std::min (4, static_cast<int> (std::round (legacyNormalised * 4.0f))));
    return normalisedEngineValue (legacyMode);
}
}
