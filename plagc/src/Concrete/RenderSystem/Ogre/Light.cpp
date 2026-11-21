#include "pch.h"
#include "Light.h"

#include <OgreLight.h>
#include <OgreSceneNode.h>
#include <OgreMath.h>

#include <stdexcept>

namespace plagc
{
    OgreLightCommon::OgreLightCommon(Ogre::Light* light, Ogre::SceneNode* node) : m_Light(light), m_Node(node)
    {
        if (!m_Light || !m_Node) throw std::invalid_argument("Ogre light and transform node are required");
    }

    void OgreLightCommon::SetDiffuseColor(const LightColor& color) { m_Light->setDiffuseColour(color.R, color.G, color.B); }
    LightColor OgreLightCommon::GetDiffuseColor() const
    {
        const auto& color = m_Light->getDiffuseColour();
        return { color.r, color.g, color.b };
    }

    void OgreLightCommon::SetSpecularColor(const LightColor& color) { m_Light->setSpecularColour(color.R, color.G, color.B); }
    LightColor OgreLightCommon::GetSpecularColor() const
    {
        const auto& color = m_Light->getSpecularColour();
        return { color.r, color.g, color.b };
    }

    void OgreLightCommon::SetPowerScale(float scale)
    {
        if (!(scale >= 0.0f)) throw std::invalid_argument("Light power must be non-negative");
        m_Light->setPowerScale(scale);
    }
    float OgreLightCommon::GetPowerScale() const { return m_Light->getPowerScale(); }
    void OgreLightCommon::SetCastShadows(bool enabled) { m_Light->setCastShadows(enabled); }
    bool OgreLightCommon::GetCastShadows() const { return m_Light->getCastShadows(); }
    void OgreLightCommon::SetVisible(bool visible) { m_Light->setVisible(visible); }
    bool OgreLightCommon::IsVisible() const { return m_Light->getVisible(); }

    void OgreLightCommon::SetPosition(float x, float y, float z) { m_Node->setPosition(x, y, z); }
    LightVector3 OgreLightCommon::GetPosition() const
    {
        const auto& position = m_Node->getPosition();
        return { position.x, position.y, position.z };
    }

    void OgreLightCommon::SetDirection(float x, float y, float z)
    {
        const Ogre::Vector3 direction(x, y, z);
        if (!(direction.squaredLength() > 0.0f)) throw std::invalid_argument("Light direction cannot be zero");
        m_Light->setDirection(direction.normalisedCopy());
    }
    LightVector3 OgreLightCommon::GetDirection() const
    {
        const Ogre::Vector3 direction = m_Light->getDirection();
        return { direction.x, direction.y, direction.z };
    }

    void OgreLightCommon::SetAttenuation(float range, float constant, float linear, float quadratic)
    {
        if (!(range > 0.0f && constant >= 0.0f && linear >= 0.0f && quadratic >= 0.0f) ||
            !(constant > 0.0f || linear > 0.0f || quadratic > 0.0f))
            throw std::invalid_argument("Invalid light attenuation");
        m_Light->setAttenuation(range, constant, linear, quadratic);
    }

    void OgreLightCommon::SetAttenuationByRadius(float radius, float threshold)
    {
        if (!(radius > 0.0f && threshold > 0.0f && threshold < 1.0f))
            throw std::invalid_argument("Invalid light radius or luminosity threshold");
        m_Light->setAttenuationBasedOnRadius(radius, threshold);
    }
    AttenuationProperties OgreLightCommon::GetAttenuation() const
    {
        return { m_Light->getAttenuationRange(), m_Light->getAttenuationConstant(),
            m_Light->getAttenuationLinear(), m_Light->getAttenuationQuadric() };
    }

    void OgreLightCommon::SetSpotlightRange(float innerAngleDegrees, float outerAngleDegrees, float falloff)
    {
        if (!(innerAngleDegrees >= 0.0f && outerAngleDegrees >= innerAngleDegrees &&
            outerAngleDegrees < 180.0f && falloff > 0.0f))
            throw std::invalid_argument("Invalid spotlight angles or falloff");
        m_Light->setSpotlightRange(Ogre::Radian(Ogre::Degree(innerAngleDegrees)),
            Ogre::Radian(Ogre::Degree(outerAngleDegrees)), falloff);
    }
    float OgreLightCommon::GetSpotlightInnerAngle() const { return m_Light->getSpotlightInnerAngle().valueDegrees(); }
    float OgreLightCommon::GetSpotlightOuterAngle() const { return m_Light->getSpotlightOuterAngle().valueDegrees(); }
    float OgreLightCommon::GetSpotlightFalloff() const { return m_Light->getSpotlightFalloff(); }

    OgreDirectionalLight::OgreDirectionalLight(Ogre::Light* light, Ogre::SceneNode* node) : m_Common(light, node) {}
    void OgreDirectionalLight::SetDiffuseColor(const LightColor& color) { m_Common.SetDiffuseColor(color); }
    LightColor OgreDirectionalLight::GetDiffuseColor() const { return m_Common.GetDiffuseColor(); }
    void OgreDirectionalLight::SetSpecularColor(const LightColor& color) { m_Common.SetSpecularColor(color); }
    LightColor OgreDirectionalLight::GetSpecularColor() const { return m_Common.GetSpecularColor(); }
    void OgreDirectionalLight::SetPowerScale(float scale) { m_Common.SetPowerScale(scale); }
    float OgreDirectionalLight::GetPowerScale() const { return m_Common.GetPowerScale(); }
    void OgreDirectionalLight::SetCastShadows(bool enabled) { m_Common.SetCastShadows(enabled); }
    bool OgreDirectionalLight::GetCastShadows() const { return m_Common.GetCastShadows(); }
    void OgreDirectionalLight::SetVisible(bool visible) { m_Common.SetVisible(visible); }
    bool OgreDirectionalLight::IsVisible() const { return m_Common.IsVisible(); }
    void OgreDirectionalLight::SetDirection(float x, float y, float z) { m_Common.SetDirection(x, y, z); }
    LightVector3 OgreDirectionalLight::GetDirection() const { return m_Common.GetDirection(); }

    OgrePointLight::OgrePointLight(Ogre::Light* light, Ogre::SceneNode* node) : m_Common(light, node) {}
    void OgrePointLight::SetDiffuseColor(const LightColor& color) { m_Common.SetDiffuseColor(color); }
    LightColor OgrePointLight::GetDiffuseColor() const { return m_Common.GetDiffuseColor(); }
    void OgrePointLight::SetSpecularColor(const LightColor& color) { m_Common.SetSpecularColor(color); }
    LightColor OgrePointLight::GetSpecularColor() const { return m_Common.GetSpecularColor(); }
    void OgrePointLight::SetPowerScale(float scale) { m_Common.SetPowerScale(scale); }
    float OgrePointLight::GetPowerScale() const { return m_Common.GetPowerScale(); }
    void OgrePointLight::SetCastShadows(bool enabled) { m_Common.SetCastShadows(enabled); }
    bool OgrePointLight::GetCastShadows() const { return m_Common.GetCastShadows(); }
    void OgrePointLight::SetVisible(bool visible) { m_Common.SetVisible(visible); }
    bool OgrePointLight::IsVisible() const { return m_Common.IsVisible(); }
    void OgrePointLight::SetPosition(float x, float y, float z) { m_Common.SetPosition(x, y, z); }
    LightVector3 OgrePointLight::GetPosition() const { return m_Common.GetPosition(); }
    void OgrePointLight::SetAttenuation(float range, float constant, float linear, float quadratic) { m_Common.SetAttenuation(range, constant, linear, quadratic); }
    void OgrePointLight::SetAttenuationByRadius(float radius, float threshold) { m_Common.SetAttenuationByRadius(radius, threshold); }
    AttenuationProperties OgrePointLight::GetAttenuation() const { return m_Common.GetAttenuation(); }

    OgreSpotLight::OgreSpotLight(Ogre::Light* light, Ogre::SceneNode* node) : m_Common(light, node) {}
    void OgreSpotLight::SetDiffuseColor(const LightColor& color) { m_Common.SetDiffuseColor(color); }
    LightColor OgreSpotLight::GetDiffuseColor() const { return m_Common.GetDiffuseColor(); }
    void OgreSpotLight::SetSpecularColor(const LightColor& color) { m_Common.SetSpecularColor(color); }
    LightColor OgreSpotLight::GetSpecularColor() const { return m_Common.GetSpecularColor(); }
    void OgreSpotLight::SetPowerScale(float scale) { m_Common.SetPowerScale(scale); }
    float OgreSpotLight::GetPowerScale() const { return m_Common.GetPowerScale(); }
    void OgreSpotLight::SetCastShadows(bool enabled) { m_Common.SetCastShadows(enabled); }
    bool OgreSpotLight::GetCastShadows() const { return m_Common.GetCastShadows(); }
    void OgreSpotLight::SetVisible(bool visible) { m_Common.SetVisible(visible); }
    bool OgreSpotLight::IsVisible() const { return m_Common.IsVisible(); }
    void OgreSpotLight::SetPosition(float x, float y, float z) { m_Common.SetPosition(x, y, z); }
    LightVector3 OgreSpotLight::GetPosition() const { return m_Common.GetPosition(); }
    void OgreSpotLight::SetAttenuation(float range, float constant, float linear, float quadratic) { m_Common.SetAttenuation(range, constant, linear, quadratic); }
    void OgreSpotLight::SetAttenuationByRadius(float radius, float threshold) { m_Common.SetAttenuationByRadius(radius, threshold); }
    AttenuationProperties OgreSpotLight::GetAttenuation() const { return m_Common.GetAttenuation(); }
    void OgreSpotLight::SetDirection(float x, float y, float z) { m_Common.SetDirection(x, y, z); }
    LightVector3 OgreSpotLight::GetDirection() const { return m_Common.GetDirection(); }
    void OgreSpotLight::SetSpotlightRange(float innerAngleDegrees, float outerAngleDegrees, float falloff) { m_Common.SetSpotlightRange(innerAngleDegrees, outerAngleDegrees, falloff); }
    float OgreSpotLight::GetSpotlightInnerAngle() const { return m_Common.GetSpotlightInnerAngle(); }
    float OgreSpotLight::GetSpotlightOuterAngle() const { return m_Common.GetSpotlightOuterAngle(); }
    float OgreSpotLight::GetSpotlightFalloff() const { return m_Common.GetSpotlightFalloff(); }
}
