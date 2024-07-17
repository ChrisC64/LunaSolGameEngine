module;
#include <compare>
#include <directxmath/DirectXMath.h>
export module DirectXCommon.DXCamera;

import LSDataLib;
import MathLib;

using namespace DirectX;
using xmvec = XMVECTOR;
using xmmat = XMMATRIX;
using vec3 = XMFLOAT3;
using vec4 = XMFLOAT4;
using mat3 = XMFLOAT3X3;
using mat4 = XMFLOAT4X4;

namespace LS::DX
{
    export class DXCamera
    {
    public:
        DXCamera() = default;
        ~DXCamera() = default;

        DXCamera(uint32_t width, uint32_t height, xmvec position, xmvec forward, xmvec up, float fovY = 45.0f, float farZ = 1'000.0f, float nearZ = 0.1f)
        {
            FovVertical = fovY;
            AspectRatio = static_cast<float>(width / (float)height);
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FovVertical), AspectRatio, farZ, nearZ);
            View = XMMatrixLookAtLH(position, forward, up);
            Width = width;
            Height = height;
            Position = position;
            Up = up;
            Forward = forward;
            FarZ = farZ;
            NearZ = nearZ;
            Right = XMVector3Cross(Up, Forward);
        }

        DXCamera(uint32_t width, uint32_t height, LS::Vec3F position, LS::Vec3F forward, LS::Vec3F up, float fovY = 45.0f, float farZ = 1'000.0f, float nearZ = 0.1f)
        {
            FovVertical = fovY;
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FovVertical), static_cast<float>(width / (float)height), farZ, nearZ);
            auto vecPos = XMVectorSet(position.x, position.y, position.z, 0.0f);
            auto vecUp = XMVectorSet(up.x, up.y, up.z, 0.0f);
            auto vecForward = XMVectorSet(forward.x, forward.y, forward.z, 0.0f);
            View = XMMatrixLookAtLH(vecPos, vecForward, vecUp);
            Width = width;
            Height = height;
            Position = vecPos;
            Up = vecUp;
            Forward = vecForward;
            FarZ = farZ;
            NearZ = nearZ;
            Right = XMVector3Cross(Up, Forward);
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
        }

        void Initialize(uint32_t width, uint32_t height, xmvec pos, xmvec target, xmvec up, float fovY = 45.0f, float farZ = 1'000.0f, float nearZ = 0.1f)
        {
            FovVertical = fovY;
            AspectRatio = static_cast<float>(width / (float)height);
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FovVertical), AspectRatio, farZ, nearZ);
            View = XMMatrixLookAtLH(pos, target, up);
            Width = width;
            Height = height;
            Position = pos;
            Up = up;
            Forward = target;
            FarZ = farZ;
            NearZ = nearZ;
            Right = XMVector3Cross(Up, Forward);
        }

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
        //xmvec Target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        xmvec Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        xmmat Projection = XMMatrixIdentity();
        xmmat View = XMMatrixIdentity();
        xmmat Mvp = XMMatrixIdentity();
        xmvec Forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        xmvec Right;

        XMFLOAT3 PositionF3()
        {
            XMFLOAT3 out;
            XMStoreFloat3(&out, Position);
            return out;
        }
    };
}