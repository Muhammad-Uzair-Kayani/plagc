#pragma once

namespace plagc
{
    enum class LightType
    {
        Directional,
        Point,
        Spot
    };

    struct LightColor
    {
        float R = 1.0f;
        float G = 1.0f;
        float B = 1.0f;
    };

    struct LightVector3
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    struct AttenuationProperties
    {
        float Range = 100.0f;
        float Constant = 1.0f;
        float Linear = 0.0f;
        float Quadratic = 0.0f;
    };

    class Light
    {
    public:
        virtual ~Light() = default;

        virtual LightType GetType() const = 0;
        virtual void SetDiffuseColor(const LightColor& color) = 0;
        virtual LightColor GetDiffuseColor() const = 0;
        virtual void SetSpecularColor(const LightColor& color) = 0;
        virtual LightColor GetSpecularColor() const = 0;
        virtual void SetPowerScale(float scale) = 0;
        virtual float GetPowerScale() const = 0;
        virtual void SetCastShadows(bool enabled) = 0;
        virtual bool GetCastShadows() const = 0;
        virtual void SetVisible(bool visible) = 0;
        virtual bool IsVisible() const = 0;

    protected:
        Light() = default;
    };

    class DirectionalLight : public Light
    {
    public:
        virtual void SetDirection(float x, float y, float z) = 0;
        virtual LightVector3 GetDirection() const = 0;
    };

    class PositionalLight : public Light
    {
    public:
        virtual void SetPosition(float x, float y, float z) = 0;
        virtual LightVector3 GetPosition() const = 0;
        virtual void SetAttenuation(float range, float constant, float linear, float quadratic) = 0;
        virtual void SetAttenuationByRadius(float radius, float threshold) = 0;
        virtual AttenuationProperties GetAttenuation() const = 0;
    };

    class PointLight : public PositionalLight {};

    class SpotLight : public PositionalLight
    {
    public:
        virtual void SetDirection(float x, float y, float z) = 0;
        virtual LightVector3 GetDirection() const = 0;
        virtual void SetSpotlightRange(float innerAngleDegrees, float outerAngleDegrees, float falloff = 1.0f) = 0;
        virtual float GetSpotlightInnerAngle() const = 0;
        virtual float GetSpotlightOuterAngle() const = 0;
        virtual float GetSpotlightFalloff() const = 0;
    };
}
