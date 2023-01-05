module;
#include "LSEFramework.h"

export module Engine.LSCamera;

using namespace DirectX;
export namespace LS
{
    using mat4 = XMFLOAT4X4;
    using xmmat = XMMATRIX;

    using vec3 = XMFLOAT3;
    using xmvec = XMVECTOR;
    //TODO: Structs don't require m_ prefix, I'll edit to make them like the other structs.
    struct LSCamera
    {
        xmmat Projection;
        xmmat View;
        xmmat Mvp;
        xmmat InvProj;
        xmmat InvView;
        xmmat InvMvp;
        xmvec Position;
        xmvec LookAt;
        xmvec Up;
        uint32_t Width;
        uint32_t Height;
        float FovAngleV;//Vertical FOV in radians
        float FovAngleH;//Horizontal FOV in radians

        LSCamera() = default;
        ~LSCamera() = default;

        LSCamera(uint32_t width, uint32_t height, xmvec position, xmvec lookAt, xmvec up, float farZ = 100.0f, float nearZ = 0.1f)
            : Projection(XMMatrixIdentity()),
            View(XMMatrixIdentity()),
            Mvp(XMMatrixIdentity()),
            InvProj(XMMatrixIdentity()),
            InvView(XMMatrixIdentity()),
            InvMvp(XMMatrixIdentity()),
            Width(width),
            Height(height),
            Position(position),
            LookAt(lookAt),
            Up(up)
        {
            Projection = XMMatrixPerspectiveFovLH( XMConvertToRadians(45.0f), static_cast<float>(width / height), nearZ, farZ);
            View = XMMatrixLookAtLH(position, lookAt, up);
            InvProj = XMMatrixInverse(nullptr, Projection);
            InvView = XMMatrixInverse(nullptr, View);
        }
    };
}