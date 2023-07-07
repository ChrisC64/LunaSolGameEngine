module;
#include <compare>
#include <directxmath/DirectXMath.h>
export module DirectXCommon:DXCameraController;

import :DXCamera;

export namespace LS::DX
{
    /**
     * @brief A free fly camera controller that allows the camera to move in any direction
     * in 3D space.
    */
    class FreeFlyCameraControllerDX
    {
    public:
        FreeFlyCameraControllerDX(DXCamera& camera) : m_camera(camera)
        {
        }

        /**
         * @brief Walks the camera forward based on the Forward vector of the camera
         * @param units movement in the forward vector direction
        */
        void Walk(float units)
        {
            m_camera.Position = XMVectorMultiplyAdd(XMVectorSet(units, units, units, 0.0f), m_camera.Forward, m_camera.Position);
        }

        /**
         * @brief Moves the camera based on the Camera's right vector. 
         * @param units movement in the right vector direction
        */
        void Strafe(float units)
        {
            m_camera.Position = XMVectorMultiplyAdd(XMVectorSet(units, units, units, 0.0f), m_camera.Forward, m_camera.Position);
        }

        /**
         * @brief Moves the camera in the given vector of directions.
         * @param movement A 3D component vector with units of movement per axis (x, y, z)
        */
        void Move(LS::Vec3F movement)
        {
            m_camera.Position = XMVectorAdd(XMVectorSet(movement.x, movement.y, movement.z, 0.0f), m_camera.Position);
        }

        void SetPosition(LS::Vec3F position)
        {
            m_camera.Position = XMVectorSet(position.x, position.y, position.z, 0.0f);
        }

        void SetAspectRatio(float aspectRatio)
        {
            m_camera.AspectRatio = aspectRatio;
        }

        void SetNearDistance(float nearDist)
        {
            m_camera.NearZ = nearDist;
        }
        
        void SetFarDistance(float farDist)
        {
            m_camera.FarZ = farDist;
        }

        void SetFov(float fov)
        {
            m_camera.FovVertical = fov;
        }

        xmmat GetOrthoProjection(float viewWidth, float viewHeight)
        {
            return XMMatrixOrthographicLH(viewWidth, viewHeight, m_camera.NearZ, m_camera.FarZ);
        }
        
        xmmat GetPerspectiveProjection()
        {
            return XMMatrixPerspectiveFovLH(XMConvertToRadians(m_camera.FovVertical), m_camera.AspectRatio, m_camera.FarZ, m_camera.NearZ);
        }

        void UpdateProjection()
        {
            m_camera.Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_camera.FovVertical), m_camera.AspectRatio, m_camera.FarZ, m_camera.NearZ);
        }

        void UpdateOrtho()
        {
            m_camera.Projection = XMMatrixOrthographicLH(m_camera.Width / 2.0f, m_camera.Height / 2.0f, m_camera.NearZ, m_camera.FarZ);
        }

        /**
         * @brief Rotates the camera around the Yaw axis (Y)
         * @param anglesDeg Angles of rotation
        */
        void RotateYaw(float anglesDeg)
        {
            auto rads = LS::Math::ToRadians(anglesDeg);
            auto rotMat = XMMatrixRotationY(rads);
            m_camera.Up = XMVector3TransformNormal(m_camera.Up, rotMat);
            m_camera.Forward = XMVector3TransformNormal(m_camera.Forward, rotMat);
            m_camera.Right = XMVector3Cross(m_camera.Up, m_camera.Forward);
            //Right = XMVector3TransformNormal(Right, rotMat);
        }

        /**
         * @brief Rotates the camera around the Roll (Z)
         * @param anglesDeg Angles of Rotation
        */
        void RotateRoll(float anglesDeg)
        {
            auto rads = LS::Math::ToRadians(anglesDeg);
            auto rotMat = XMMatrixRotationZ(rads);
            m_camera.Right = XMVector3TransformNormal(m_camera.Right, rotMat);
            m_camera.Up = XMVector3TransformNormal(m_camera.Up, rotMat);
            //Forward = XMVector3TransformNormal(Forward, rotMat);
        }

        /**
         * @brief Rotates around the Pitch (X)
         * @param anglesDeg Angles of rotation
        */
        void RotatePitch(float anglesDeg)
        {
            auto rads = LS::Math::ToRadians(anglesDeg);
            auto rotMat = XMMatrixRotationX(rads);
            //Right = XMVector3TransformNormal(Right, rotMat);
            m_camera.Up = XMVector3TransformNormal(m_camera.Up, rotMat);
            m_camera.Forward = XMVector3TransformNormal(m_camera.Forward, rotMat);
        }

        /**
         * @brief Rotates around a normalized axis
         * @param axis A vector containing x, y, and z axis rotation between 0 and 1
         * @param anglesDeg The degrees or rotation to apply to the normalized vector
        */
        void RotateAxis(LS::Vec3F axis, float anglesDeg)
        {
            auto rads = LS::Math::ToRadians(anglesDeg);
            auto rotation = XMQuaternionRotationNormal(XMVectorSet(axis.x, axis.y, axis.z, 1.0f), rads);

            m_camera.Forward = XMQuaternionMultiply(m_camera.Forward, rotation);
            m_camera.Up = XMQuaternionMultiply(m_camera.Up, rotation);
            m_camera.Right = XMQuaternionMultiply(m_camera.Right, rotation);
        }

    private:
        DXCamera& m_camera;
    };
}