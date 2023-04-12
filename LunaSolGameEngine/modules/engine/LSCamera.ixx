module;
#include "LSEFramework.h"

export module Engine.LSCamera;
import Data.LSMath.Types;
using namespace DirectX;
export namespace LS
{
    struct LSCamera
    {
        LS::Mat4F Projection;
        LS::Mat4F View;
        LS::Mat4F Mvp;
        LS::Mat4F InvProj;
        LS::Mat4F InvView;
        LS::Mat4F InvMvp;
        LS::Vec4F Position;
        LS::Vec4F LookAt;
        LS::Vec4F Up;
        uint32_t Width;
        uint32_t Height;
        float FovAngleV;//Vertical FOV in radians
        float FovAngleH;//Horizontal FOV in radians

        LSCamera() = default;
        ~LSCamera() = default;

        LSCamera(uint32_t width, uint32_t height, LS::Vec4F position, LS::Vec4F lookAt, LS::Vec4F up, float farZ = 100.0f, float nearZ = 0.1f)
            : Projection(Mat4F::Identity()),
            View(Mat4F::Identity()),
            Mvp(Mat4F::Identity()),
            InvProj(Mat4F::Identity()),
            InvView(Mat4F::Identity()),
            InvMvp(Mat4F::Identity()),
            Width(width),
            Height(height),
            Position(position),
            LookAt(lookAt),
            Up(up)
        {
            /*Projection = XMMatrixPerspectiveFovLH( XMConvertToRadians(45.0f), static_cast<float>(width / height), nearZ, farZ);
            View = XMMatrixLookAtLH(position, lookAt, up);
            InvProj = XMMatrixInverse(nullptr, Projection);
            InvView = XMMatrixInverse(nullptr, View);*/
        }
    };
}