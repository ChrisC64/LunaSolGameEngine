module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <wrl/client.h>
#include <array>

export module D3D12Lib:D3D12Common;

import Engine.App;

namespace WRL = Microsoft::WRL;
export namespace LS::Platform::Dx12
{
    struct D3D12Settings
    {
        D3D_FEATURE_LEVEL   MaxFeatureLevel = D3D_FEATURE_LEVEL_12_2;
        D3D_FEATURE_LEVEL   MinFeatureLevel = D3D_FEATURE_LEVEL_11_1;
        D3D_FEATURE_LEVEL   CurrFeatureLevel = MinFeatureLevel;
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

    //template<class T, size_t size>
    //using ComPtrArray = std::array<WRL::ComPtr<T>, size>;

    //using FrameCommandList = ComPtrArray<ID3D12GraphicsCommandList6, Global::NUM_CONTEXT>;
    //using CommandAllocator = ComPtrArray<ID3D12GraphicsCommandList6, Global::NUM_CONTEXT>;

    //struct FrameContext
    //{
    //    // Manages a heap for the command lists. 
    //    // This cannot be reset while the CommandList is still in flight on the GPU0
    //    CommandAllocator CommandAllocator;
    //    // Use with the bundle list, this allocator performs the 
    //    // same operations as a command list, but is associated with the bundle
    //    WRL::ComPtr<ID3D12CommandAllocator> BundleAllocator;
    //    // Sends commands to the GPU - represents this frames commands
    //    FrameCommandList CommandList;
    //    // Bundle up calls you would want repeated constantly, 
    //    // like setting up a draw for a vertex buffer. 
    //    WRL::ComPtr<ID3D12GraphicsCommandList> BundleList;
    //    // Singal value between the GPU and CPU to perform synchronization. 
    //    uint64_t                               FenceValue;
    //};
}