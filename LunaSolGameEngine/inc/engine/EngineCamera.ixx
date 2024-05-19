module;
#include <cstdint>
#include <compare>
export module Engine.LSCamera;
import LSDataLib;

export namespace LS
{
    struct LSCamera
    {
        uint32_t Width;
        uint32_t Height;
        float FovVertical;//Vertical FOV in radians
        float FovHorizontal;//Horizontal FOV in radians
        float NearZ = 0.1f;
        float FarZ = 100.0f;
        LS::Mat4F Projection;
        LS::Mat4F View;
        LS::Mat4F Mvp;
        LS::Mat4F InvProj;
        LS::Mat4F InvView;
        LS::Mat4F InvMvp;
        LS::Vec4F Position;
        LS::Vec4F LookAt;
        LS::Vec4F Up;

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
            Up(up),
            NearZ(nearZ),
            FarZ(farZ)
        {
        }
    };
}