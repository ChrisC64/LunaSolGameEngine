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

    struct LSCamera
    {
    protected:
        xmmat m_projection;
        xmmat m_view;
        xmmat m_mvp;
        xmmat m_invProj;
        xmmat m_invView;
        xmmat m_invMvp;

        uint32_t m_width;
        uint32_t m_height;

        xmvec m_position;
        xmvec m_lookAt;
        xmvec m_up;
        float m_fovAngleY;
        float m_fovAngleX;

    public:
        LSCamera() = default;
        ~LSCamera() = default;

        LSCamera(uint32_t width, uint32_t height, xmvec position, xmvec lookAt, xmvec up, float farZ = 1.0f, float nearZ = 0.0f)
            : m_projection(XMMatrixIdentity()),
            m_view(XMMatrixIdentity()),
            m_mvp(XMMatrixIdentity()),
            m_invProj(XMMatrixIdentity()),
            m_invView(XMMatrixIdentity()),
            m_invMvp(XMMatrixIdentity()),
            m_width(width),
            m_height(height),
            m_position(position),
            m_lookAt(lookAt),
            m_up(up),
        {
            m_projection = XMMatrixPerspectiveLH(width, height, nearZ, farZ);
            m_view = XMMatrixLookAtLH(position, lookAt, up);
            m_invProj = XMMatrixInverse(nullptr, m_projection);
            m_invView = XMMatrixInverse(nullptr, m_view);

        }
    };
}