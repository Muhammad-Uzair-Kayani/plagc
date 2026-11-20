#include "pch.h"
#include "Camera.h"

#include <Ogre.h>

#include <stdexcept>

namespace plagc
{
    OgreCamera::OgreCamera(Ogre::Camera* camera) : m_Camera(camera)
    {
        if (!m_Camera) throw std::invalid_argument("OgreCamera requires a valid Ogre::Camera");
    }

    void OgreCamera::SetPosition(float x, float y, float z) { m_Camera->setPosition(x, y, z); }

    CameraVector3 OgreCamera::GetPosition() const
    {
        const Ogre::Vector3& position = m_Camera->getPosition();
        return { position.x, position.y, position.z };
    }

    void OgreCamera::Move(float x, float y, float z) { m_Camera->move(Ogre::Vector3(x, y, z)); }

    void OgreCamera::MoveRelative(float x, float y, float z) { m_Camera->moveRelative(Ogre::Vector3(x, y, z)); }

    void OgreCamera::LookAt(float x, float y, float z) { m_Camera->lookAt(x, y, z); }

    void OgreCamera::SetDirection(float x, float y, float z) { m_Camera->setDirection(x, y, z); }

    void OgreCamera::Yaw(float degrees) { m_Camera->yaw(Ogre::Radian(Ogre::Degree(degrees))); }

    void OgreCamera::Pitch(float degrees) { m_Camera->pitch(Ogre::Radian(Ogre::Degree(degrees))); }


    void OgreCamera::SetPerspective(float fovYDegrees, float aspectRatio, float nearClip, float farClip)
    {
        if (!(fovYDegrees > 0.0f && fovYDegrees < 179.0f)) throw std::invalid_argument("Invalid vertical field of view");
        if (!(aspectRatio > 0.0f)) throw std::invalid_argument("Aspect ratio must be positive");
        if (!(nearClip > 0.0f && (farClip == 0.0f || farClip > nearClip))) throw std::invalid_argument("Invalid clipping distances");

        m_Camera->setProjectionType(Ogre::PT_PERSPECTIVE);
        m_Camera->setAutoAspectRatio(false);
        m_Camera->setFOVy(Ogre::Radian(Ogre::Degree(fovYDegrees)));
        m_Camera->setAspectRatio(aspectRatio);
        m_Camera->setNearClipDistance(nearClip);
        m_Camera->setFarClipDistance(farClip);
    }

    void OgreCamera::SetOrthographic(float width, float height, float nearClip, float farClip)
    {
        if (!(width > 0.0f && height > 0.0f)) throw std::invalid_argument("Orthographic dimensions must be positive");
        if (!(nearClip > 0.0f && farClip > nearClip)) throw std::invalid_argument("Invalid clipping distances");

        m_Camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
        m_Camera->setAutoAspectRatio(false);
        m_Camera->setOrthoWindow(width, height);
        m_Camera->setNearClipDistance(nearClip);
        m_Camera->setFarClipDistance(farClip);
    }

    void OgreCamera::SetAspectRatio(float aspectRatio)
    {
        if (!(aspectRatio > 0.0f)) throw std::invalid_argument("Aspect ratio must be positive");
        m_Camera->setAutoAspectRatio(false);
        m_Camera->setAspectRatio(aspectRatio);
    }

    void OgreCamera::SetAutoAspectRatio(bool enabled) { m_Camera->setAutoAspectRatio(enabled); }
}
