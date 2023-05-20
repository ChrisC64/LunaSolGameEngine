module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>
#include <span>
#include <cassert>
#include <vector>
export module D3D12Lib:D3D12Helper;

export namespace LS::Win32
{
    constexpr void SetViewPorts(ID3D12GraphicsCommandList* command, std::span<D3D12_VIEWPORT> viewports) noexcept
    {
        assert(command);
        assert(viewports.size() <= UINT32_MAX);
        command->RSSetViewports((UINT)viewports.size(), &viewports.front());
    }

    constexpr void SetScissorRects(ID3D12GraphicsCommandList* command, std::span<D3D12_RECT> rects) noexcept
    {
        assert(command);
        assert(rects.size() <= UINT32_MAX);
        command->RSSetScissorRects((UINT)rects.size(), &rects.front());
    }

    constexpr auto CreateDescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC* descriptor, ID3D12DescriptorHeap* heap) noexcept -> HRESULT
    {
        assert(device);
        assert(descriptor);
        assert(heap);
        return device->CreateDescriptorHeap(descriptor, IID_PPV_ARGS(&heap));
    }

    constexpr auto GetDescriptorHandleIncrementSize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type) -> uint32_t
    {
        assert(device);
        return device->GetDescriptorHandleIncrementSize(type);
    }

    constexpr auto CreateDescriptorHeapDesc(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT nodeMask) -> D3D12_DESCRIPTOR_HEAP_DESC
    {
        return D3D12_DESCRIPTOR_HEAP_DESC{ .Type = type, .NumDescriptors = numDescriptors, .Flags = flags, .NodeMask = nodeMask };
    };

    auto CreateRenderTarget(ID3D12Device* device, CD3DX12_CPU_DESCRIPTOR_HANDLE handle, std::span<ID3D12Resource*> rtResources, IDXGISwapChain* swapChain,
        uint32_t descriptorIncrementSize)
    {
        assert(device);
        assert(rtResources.size() > 0);
        assert(swapChain);

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(rtResources.size());
        for (auto i = 0u; i < rtResources.size(); i++)
        {
            auto result = swapChain->GetBuffer(i, IID_PPV_ARGS(&rtResources[i])));
            device->CreateRenderTargetView(rtResources[i], nullptr, handle);
            handle.Offset(1, descriptorIncrementSize);
            handles[i] = handle;
        }
    }
}