module;
#include <compare>
#include <directxmath/DirectXMath.h>
export module DirectXCommon:DXCamera;

import Data.LSMathTypes;
import MathLib;

namespace LS::DX
{
    using namespace DirectX;
    using xmvec = XMVECTOR;
    using xmmat = XMMATRIX;
    using vec3 = XMFLOAT3;
    using vec4 = XMFLOAT4;
    using mat3 = XMFLOAT3X3;
    using mat4 = XMFLOAT4X4;

    export struct DXCamera
    {
        uint32_t Width;
        uint32_t Height;
        float NearZ = 0.1f;
        float FarZ = 1000.0f;
        //@brief the vertical angle in degrees
        float FovVertical = 0.0f;
        //@brief the horizontal angle in degrees
        float FovHorizontal = 0.0f;
        float AspectRatio = 0.0f;
        // Explicit padding, because we need to be 16 Byte aligned with 
        // the use of our SIMD objects below. I maybe can find something
        // to use this for, but cannot think of one right now.
        float pad = 0.0f; 
        xmvec Position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        xmvec Target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        xmvec Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        xmmat Projection = XMMatrixIdentity();
        xmmat View = XMMatrixIdentity();
        xmmat Mvp = XMMatrixIdentity();
        xmvec Forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        xmvec Right;

        DXCamera() = default;
        ~DXCamera() = default;

        DXCamera(uint32_t width, uint32_t height, xmvec position, xmvec target, xmvec up, float fovY = 45.0f, float farZ = 1'000.0f, float nearZ = 0.1f)
        {
            FovVertical = fovY;
            AspectRatio = static_cast<float>(width / height);
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FovVertical), AspectRatio, farZ, nearZ);
            View = XMMatrixLookAtLH(position, target, up);
            Width = width;
            Height = height;
            Position = position;
            Up = up;
            Target = target;
            FarZ = farZ;
            NearZ = nearZ;
            Right = XMVector4Dot(Up, Forward);
        }
        
        DXCamera(uint32_t width, uint32_t height, LS::Vec3F position, LS::Vec3F target, LS::Vec3F up, float fovY = 45.0f, float farZ = 1'000.0f, float nearZ = 0.1f)
        {
            FovVertical = fovY;
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FovVertical), static_cast<float>(width / height), farZ, nearZ);
            auto vecPos = XMVectorSet(position.x, position.y, position.z, 0.0f);
            auto vecUp = XMVectorSet(up.x, up.y, up.z, 0.0f);
            auto vecTarg = XMVectorSet(target.x, target.y, target.z, 0.0f);
            View = XMMatrixLookAtLH(vecPos, vecTarg, vecUp);
            Width = width;
            Height = height;
            Position = vecPos;
            Up = vecUp;
            Target = vecTarg;
            FarZ = farZ;
            NearZ = nearZ;
            Right = XMVector4Dot(Up, Forward);
        }

        xmmat InvProj()
        {
            return XMMatrixInverse(nullptr, Projection);
        }

        xmmat InvView()
        {
            return XMMatrixInverse(nullptr, View);
        }

        void UpdateProjection()
        {
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FovVertical), AspectRatio, FarZ, NearZ);
        }

        void UpdateView()
        {
            auto forwardNorm = XMVector3Normalize(Forward);
            auto upNorm = XMVector3Normalize(Up);
            auto rightNorm = XMVector3Normalize(Right);

            View = XMMatrixLookAtLH(Position, XMVectorAdd(Position, forwardNorm), upNorm);
            //View = XMMatrixLookAtLH(Position, Target, Up);
        }

        void TranslatePosition(LS::Vec3F pos)
        {
            Position = XMVectorAdd(Position, XMVectorSet(pos.x, pos.y, pos.z, 1.0f));
        }

        void Walk(float units)
        {
            Position = XMVectorMultiplyAdd(XMVectorSet(units, units, units, 0.0f), Forward, Position);
        }
        
        void Strafe(float units)
        {
            Position = XMVectorMultiplyAdd(XMVectorSet(units, units, units, 0.0f), Right, Position);
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

            Forward = XMQuaternionMultiply(Forward, rotation);
            Up = XMQuaternionMultiply(Up, rotation);
            Right = XMQuaternionMultiply(Right, rotation);
        }
        
        /**
         * @brief Rotates the camera around the Yaw axis (Y)
         * @param anglesDeg Angles of rotation
        */
        void RotateYaw(float anglesDeg)
        {
            auto rads = LS::Math::ToRadians(anglesDeg);
            auto rotMat = XMMatrixRotationY(rads);
            Right = XMVector3TransformNormal(Right, rotMat);
            Up = XMVector3TransformNormal(Up, rotMat);
            Forward = XMVector3TransformNormal(Forward, rotMat);
        }
        
        /**
         * @brief Rotates the camera around the Roll (Z)
         * @param anglesDeg Angles of Rotation
        */
        void RotateRoll(float anglesDeg)
        {
            auto rads = LS::Math::ToRadians(anglesDeg);
            auto rotMat = XMMatrixRotationZ(rads);
            Right = XMVector3TransformNormal(Right, rotMat);
            Up = XMVector3TransformNormal(Up, rotMat);
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
            Up = XMVector3TransformNormal(Up, rotMat);
            Forward = XMVector3TransformNormal(Forward, rotMat);
        }
    };
}