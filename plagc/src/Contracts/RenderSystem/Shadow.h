#pragma once
#include <cstdint>

namespace plagc
{
    enum class ShadowFilter
    {
        PCF_2x2,
        PCF_3x3,
        PCF_4x4,
        PCF_5x5,
        PCF_6x6,
        ESM
    };

    struct ShadowSettings
    {
        std::uint32_t DirectionalResolution = 2048;
        std::uint32_t SpotResolution = 1024;
        std::uint32_t PointResolution = 512;
        std::uint8_t PssmSplits = 3;
        std::uint8_t MaxSpotLights = 2;
        std::uint8_t MaxPointLights = 1;
        float PssmLambda = 0.95f;
        float SplitPadding = 1.0f;
        float SplitBlend = 0.125f;
        float SplitFade = 0.313f;
        ShadowFilter Filter = ShadowFilter::PCF_3x3;
        std::uint16_t EsmK = 600;
    };
}
