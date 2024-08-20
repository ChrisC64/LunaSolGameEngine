module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <span>
#include <cassert>
#include <directx/d3dx12.h>
#include <vector>
#include "engine/EngineLogDefines.h"
export module D3D12Lib.D3D12Utils.Descriptors;
import Win32.Utils;

export namespace LS::Platform::Dx12
{
    constexpr auto CreateDescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& descriptor, ID3D12DescriptorHeap* heap) noexcept -> HRESULT
    {
        assert(device);
        assert(heap);
        return device->CreateDescriptorHeap(&descriptor, IID_PPV_ARGS(&heap));
    }

    // CREATE FUNCTIONS //
    constexpr auto CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC desc, ID3D12Device* device, ID3D12DescriptorHeap* pHeap) -> HRESULT
    {
        assert(device);
        return device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap));
    };

    constexpr auto CreateCbvSrvUavDescriptor(uint32_t numDescriptors, bool isShaderVisible = false, uint32_t nodes = 0) -> D3D12_DESCRIPTOR_HEAP_DESC
    {
        return D3D12_DESCRIPTOR_HEAP_DESC{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptors,
            isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE, nodes };
    }

    constexpr auto CreateSamplerDescriptor(uint32_t numDescriptors, bool isShaderVisible = false, uint32_t nodes = 0) -> D3D12_DESCRIPTOR_HEAP_DESC
    {
        return D3D12_DESCRIPTOR_HEAP_DESC{ .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, .NumDescriptors = numDescriptors, 
            .Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = nodes };
    }

    constexpr auto CreateDepthStencilViewDescriptor(uint32_t numDescriptors, bool isShaderVisible = false, uint32_t nodes = 0) -> D3D12_DESCRIPTOR_HEAP_DESC
    {
        return D3D12_DESCRIPTOR_HEAP_DESC{ .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = numDescriptors,
            .Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE, .NodeMask = nodes };
    }

    constexpr auto CreateViewport(float tlX, float tlY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) -> D3D12_VIEWPORT
    {
        return D3D12_VIEWPORT{ .TopLeftX = tlX, .TopLeftY = tlY, .Width = width, .Height = height, .MinDepth = minDepth, .MaxDepth = maxDepth };
    }

    constexpr auto CreateScissorRect(LONG tlX, LONG tlY, LONG brX, LONG brY) -> D3D12_RECT
    {
        return D3D12_RECT{.left = tlX, .top = tlY, .right = brX, .bottom = brY };
    }

    constexpr auto GetDescriptorHandleIncrementSize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type) -> uint32_t
    {
        assert(device);
        return device->GetDescriptorHandleIncrementSize(type);
    }
    
    //TODO: Create Helper Functions for creating the following:
    // 2. Create Root Signatures
    // 3. Create and configure root signatures
    // 4. Descriptor Ranges

    /**
     * @brief Creates a render target view 
     * @param device Device object that will be used to create the RTVs
     * @param handle Handle to the CPU Descriptor 
     * @param rtResources The render target views to create
     * @param swapChain the render target 
     * @return HRESULT with error information
    */
    auto CreateRenderTarget(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE handle, std::span<ID3D12Resource*> rtResources, IDXGISwapChain* swapChain) -> HRESULT
    {
        assert(device);
        assert(rtResources.size() > 0);
        assert(swapChain);
        auto descIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE h(handle);
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(rtResources.size());
        for (auto i = 0u; i < rtResources.size(); i++)
        {
            auto result = swapChain->GetBuffer(i, IID_PPV_ARGS(&rtResources[i]));
            if (FAILED(result))
            {
                LS_LOG_ERROR(std::format(L"Failed to get buffer at pos: {} when creating render target view. {}", 
                    i, HrToWString(result)));
                return result;
            }
            device->CreateRenderTargetView(rtResources[i], nullptr, h);
            h.Offset(1, descIncSize);
            handles[i] = h;
        }
        return S_OK;
    }
}