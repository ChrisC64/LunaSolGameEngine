module;
#include <directxmath/DirectXMath.h>
export module DirectXCommon:DXCamera;

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
        //@brief the vertical angle in radians
        float FovAngleVRads = 0.0f;
        //@brief the horizontal angle in radians
        float FovAngleHRads = 0.0f;
        float AspectRatio = 0.0f;
        // Explicit padding, because we need to be 16 Byte aligned with 
        // the use of our SIMD objects below. I maybe can find something
        // to use this for, but cannot think of one right now.
        float pad = 0.0f; 
        xmvec Position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        xmvec Target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        xmvec Up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
        xmmat Projection = XMMatrixIdentity();
        xmmat View = XMMatrixIdentity();
        xmmat Mvp = XMMatrixIdentity();
        xmvec Forward;
        xmvec Right;

        DXCamera() = default;
        ~DXCamera() = default;

        DXCamera(uint32_t width, uint32_t height, xmvec position, xmvec target, xmvec up, float farZ = 1'000.0f, float nearZ = 0.1f)
        {
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(width / height), farZ, nearZ);
            View = XMMatrixLookAtLH(position, target, up);
            Width = width;
            Height = height;
            Position = position;
            Up = up;
            Target = target;
            FarZ = farZ;
            NearZ = nearZ;
        }

        xmmat InvProj()
        {
            return XMMatrixInverse(nullptr, Projection);
        }

        xmmat InvView()
        {
            return XMMatrixInverse(nullptr, View);
        }

        void UpdateProjection(float fovY = 45.0f)
        {
            Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovY), static_cast<float>(Width / Height), FarZ, NearZ);
        }

        void UpdateView()
        {
            View = XMMatrixLookAtLH(Position, Target, Up);
        }

    };
}