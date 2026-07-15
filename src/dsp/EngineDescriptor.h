#pragma once

#include <array>
#include <cstddef>

namespace acidlab
{
constexpr int engineCount = 11;
constexpr int engineControlCount = 3;

struct EngineControls
{
    float a = 0.5f;
    float b = 0.5f;
    float c = 0.5f;
};

struct EngineDescriptor
{
    const char* id;
    const char* name;
    const char* controlALabel;
    const char* controlBLabel;
    const char* controlCLabel;
    EngineControls defaults;
    float outputCompensation;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

const std::array<EngineDescriptor, engineCount>& getEngineDescriptors() noexcept;
const EngineDescriptor& getEngineDescriptor (int mode) noexcept;
float normalisedEngineValue (int mode) noexcept;
float migrateLegacyEngineValue (float legacyNormalised) noexcept;
}
