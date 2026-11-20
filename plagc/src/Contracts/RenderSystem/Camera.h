#pragma once

namespace plagc
{
    struct CameraVector3
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    class Camera
    {
    public:
        virtual ~Camera() = default;

        virtual void SetPosition(float x, float y, float z) = 0;
        virtual CameraVector3 GetPosition() const = 0;
        virtual void Move(float x, float y, float z) = 0;
        virtual void MoveRelative(float x, float y, float z) = 0;
        virtual void LookAt(float x, float y, float z) = 0;
        virtual void SetDirection(float x, float y, float z) = 0;
        virtual void Yaw(float degrees) = 0;
        virtual void Pitch(float degrees) = 0;

        virtual void SetPerspective(float fovYDegrees, float aspectRatio, float nearClip, float farClip) = 0;
        virtual void SetOrthographic(float width, float height, float nearClip, float farClip) = 0;
        virtual void SetAspectRatio(float aspectRatio) = 0;
        virtual void SetAutoAspectRatio(bool enabled) = 0;

    protected:
        Camera() = default;
    };
}
