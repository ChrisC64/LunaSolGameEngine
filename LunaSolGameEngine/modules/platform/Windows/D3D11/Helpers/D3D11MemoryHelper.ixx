module;
#include "LSEFramework.h"

export module D3D11.MemoryHelper;

export namespace LS::Win32
{
    // GPU MEMORY ACCESS CALLS //

    template<typename T>
    [[nodiscard]] constexpr LSOptional<T*> Lock(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, uint32_t numSubResource,
        D3D11_MAP mapType, uint32_t mapFlags) noexcept
    {
        assert(pContext);
        assert(pResource);
        if (!pContext || !pResource)
            return std::nullopt;

        D3D11_MAPPED_SUBRESOURCE mappedData;
        HRESULT hr = pContext->Map(pResource, numSubResource, mapType, mapFlags, mappedData);

        if (FAILED(hr) || mappedData.pData == nullptr)
            return std::nullopt;

        return static_cast<T*>(mappedData.pData);
    }

    template<typename T>
    constexpr void Unlock(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, uint32_t numSubResource,
        D3D11_MAP mapType, uint32_t mapFlags) noexcept
    {
        assert(pContext);
        assert(pResource);
        if (!pContext || !pResource)
            return std::nullopt;

        pContext->Unmap(pResource, numSubResource);
    }

    constexpr void UpdateSubresource(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, uint32_t dstSubresource,
        const void* ptrData, uint32_t sourceRow = 0, uint32_t sourceDepth = 0, D3D11_BOX* dstBox = nullptr) noexcept
    {
        assert(pContext);
        assert(pResource);
        if (!pContext || !pResource)
            return;

        pContext->UpdateSubresource(pResource, dstSubresource, dstBox, ptrData, sourceRow, sourceDepth);
    }
    
    constexpr void UpdateSubresource1(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, uint32_t dstSubresource,
        const void* ptrData, uint32_t sourceRow = 0, uint32_t sourceDepth = 0, D3D11_BOX* dstBox = nullptr,
        std::span<D3D11_COPY_FLAGS> copyFlags = {}) noexcept
    {
        assert(pContext);
        assert(pResource);
        if (!pContext || !pResource)
            return;
        uint32_t flags = 0u;
        if (!copyFlags.empty())
        {
            for (auto flag : copyFlags)
            {
                flags |= flag;
            }
        }

        pContext->UpdateSubresource1(pResource, dstSubresource, dstBox, ptrData, sourceRow, sourceDepth, flags);
    }

    [[nodiscard]]
    constexpr HRESULT CreateBuffer(ID3D11Device* pDevice, const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept
    {
        assert(pDevice);

        return pDevice->CreateBuffer(pDesc, pInitialData, ppBuffer);
    }

}