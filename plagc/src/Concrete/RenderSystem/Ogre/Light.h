#pragma once
#include "Contracts/RenderSystem/Light.h"

namespace Ogre { class Light; class SceneNode; }

namespace plagc
{
    // Shared Ogre operations; not another public Light base or owner.
    class OgreLightCommon
    {
    public:
        OgreLightCommon(Ogre::Light* light, Ogre::SceneNode* node);

        void SetDiffuseColor(const LightColor& color);
        LightColor GetDiffuseColor() const;
        void SetSpecularColor(const LightColor& color);
        LightColor GetSpecularColor() const;
        void SetPowerScale(float scale);
        float GetPowerScale() const;
        void SetCastShadows(bool enabled);
        bool GetCastShadows() const;
        void SetVisible(bool visible);
        bool IsVisible() const;

        void SetPosition(float x, float y, float z);
        LightVector3 GetPosition() const;
        void SetDirection(float x, float y, float z);
        LightVector3 GetDirection() const;
        void SetAttenuation(float range, float constant, float linear, float quadratic);
        void SetAttenuationByRadius(float radius, float threshold);
        AttenuationProperties GetAttenuation() const;
        void SetSpotlightRange(float innerAngleDegrees, float outerAngleDegrees, float falloff);
        float GetSpotlightInnerAngle() const;
        float GetSpotlightOuterAngle() const;
        float GetSpotlightFalloff() const;

    private:
        Ogre::Light* m_Light = nullptr;          // Owned by Ogre::SceneManager.
        Ogre::SceneNode* m_Node = nullptr;       // Owned by Ogre::SceneManager.
    };

    class OgreDirectionalLight final : public DirectionalLight
    {
    public:
        OgreDirectionalLight(Ogre::Light* light, Ogre::SceneNode* node);
        LightType GetType() const override { return LightType::Directional; }
        void SetDiffuseColor(const LightColor& color) override;
        LightColor GetDiffuseColor() const override;
        void SetSpecularColor(const LightColor& color) override;
        LightColor GetSpecularColor() const override;
        void SetPowerScale(float scale) override;
        float GetPowerScale() const override;
        void SetCastShadows(bool enabled) override;
        bool GetCastShadows() const override;
        void SetVisible(bool visible) override;
        bool IsVisible() const override;
        void SetDirection(float x, float y, float z) override;
        LightVector3 GetDirection() const override;

    private:
        OgreLightCommon m_Common;
    };

    class OgrePointLight final : public PointLight
    {
    public:
        OgrePointLight(Ogre::Light* light, Ogre::SceneNode* node);
        LightType GetType() const override { return LightType::Point; }
        void SetDiffuseColor(const LightColor& color) override;
        LightColor GetDiffuseColor() const override;
        void SetSpecularColor(const LightColor& color) override;
        LightColor GetSpecularColor() const override;
        void SetPowerScale(float scale) override;
        float GetPowerScale() const override;
        void SetCastShadows(bool enabled) override;
        bool GetCastShadows() const override;
        void SetVisible(bool visible) override;
        bool IsVisible() const override;
        void SetPosition(float x, float y, float z) override;
        LightVector3 GetPosition() const override;
        void SetAttenuation(float range, float constant, float linear, float quadratic) override;
        void SetAttenuationByRadius(float radius, float threshold) override;
        AttenuationProperties GetAttenuation() const override;

    private:
        OgreLightCommon m_Common;
    };

    class OgreSpotLight final : public SpotLight
    {
    public:
        OgreSpotLight(Ogre::Light* light, Ogre::SceneNode* node);
        LightType GetType() const override { return LightType::Spot; }
        void SetDiffuseColor(const LightColor& color) override;
        LightColor GetDiffuseColor() const override;
        void SetSpecularColor(const LightColor& color) override;
        LightColor GetSpecularColor() const override;
        void SetPowerScale(float scale) override;
        float GetPowerScale() const override;
        void SetCastShadows(bool enabled) override;
        bool GetCastShadows() const override;
        void SetVisible(bool visible) override;
        bool IsVisible() const override;
        void SetPosition(float x, float y, float z) override;
        LightVector3 GetPosition() const override;
        void SetAttenuation(float range, float constant, float linear, float quadratic) override;
        void SetAttenuationByRadius(float radius, float threshold) override;
        AttenuationProperties GetAttenuation() const override;
        void SetDirection(float x, float y, float z) override;
        LightVector3 GetDirection() const override;
        void SetSpotlightRange(float innerAngleDegrees, float outerAngleDegrees, float falloff = 1.0f) override;
        float GetSpotlightInnerAngle() const override;
        float GetSpotlightOuterAngle() const override;
        float GetSpotlightFalloff() const override;

    private:
        OgreLightCommon m_Common;
    };
}
