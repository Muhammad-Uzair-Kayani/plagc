#pragma once
#include "Contracts/RenderSystem/Camera.h"

namespace Ogre { class Camera; }

namespace plagc
{
    class OgreCamera final : public Camera
    {
    public:
        explicit OgreCamera(Ogre::Camera* camera);
        ~OgreCamera() override = default;

        void SetPosition(float x, float y, float z) override;
        CameraVector3 GetPosition() const override;
        void Move(float x, float y, float z) override;
        void MoveRelative(float x, float y, float z) override;
        void LookAt(float x, float y, float z) override;
        void SetDirection(float x, float y, float z) override;
        void Yaw(float degrees) override;
        void Pitch(float degrees) override;

        void SetPerspective(float fovYDegrees, float aspectRatio, float nearClip, float farClip) override;
        void SetOrthographic(float width, float height, float nearClip, float farClip) override;
        void SetAspectRatio(float aspectRatio) override;
        void SetAutoAspectRatio(bool enabled) override;

        Ogre::Camera* GetOgreCamera() const { return m_Camera; }

    private:
        Ogre::Camera* m_Camera = nullptr; // Owned by Ogre::SceneManager.
    };
}
