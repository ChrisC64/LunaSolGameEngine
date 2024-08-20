module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <wrl/client.h>
#include <array>

export module D3D12Lib.D3D12Common;

export namespace LS::Platform::Dx12
{
    struct D3D12Settings
    {
        D3D_FEATURE_LEVEL   FeatureLevel = D3D_FEATURE_LEVEL_12_0;
        DXGI_FORMAT         PixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_SWAP_EFFECT    SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        DXGI_ALPHA_MODE     AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        DXGI_SCALING        Scaling = DXGI_SCALING_STRETCH;
        uint32_t            FrameBufferCount = 2;
        uint32_t            Width;
        uint32_t            Height;
        HWND                Hwnd;
        bool                EnableVSync = true;
        bool                IsStereoScopic = false;
    };
}