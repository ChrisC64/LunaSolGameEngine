module;
#include <optional>
#include <cstdint>
#include <cassert>
#include <d3d11_4.h>
#include <span>
#include <stdexcept>

export module D3D11.MemoryHelper;
import LSData;

export namespace LS::Win32
{
    // GPU MEMORY ACCESS CALLS //

    template<typename T>
    [[nodiscard]] constexpr Nullable<T*> Lock(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, uint32_t numSubResource,
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

    /**
    * @brief Finds the D3D11_USAGE type for the given LS::BUFFER_USAGE enum
    * @param bufferUsage
    * @return the value that corresponds to the given D3D11_USAGE enum from the given LS::BUFFER_USAGE enum
   */
    [[nodiscard]]
    constexpr D3D11_USAGE FindUsageD3D11(LS::BUFFER_USAGE bufferUsage)
    {
        using enum LS::BUFFER_USAGE;

        switch (bufferUsage)
        {
        case DEFAULT_RW: return D3D11_USAGE::D3D11_USAGE_DEFAULT;
        case CONSTANT: return D3D11_USAGE::D3D11_USAGE_DEFAULT;
        case IMMUTABLE: return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
        case DYNAMIC: return D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        case COPY_ONLY: return D3D11_USAGE::D3D11_USAGE_STAGING;
        default:
            throw std::runtime_error("Failed to find a suitable buffer usage type");
        }
    }

    /**
     * @brief Convert the @link LS::BUFFER_BIND_TYPE to a @link D3D11_BIND_FLAG object
     * @param bindType
     * @return throws on unknown enums, otherwise returns enums that are suported
    */
    [[nodiscard]]
    constexpr D3D11_BIND_FLAG FindBufferBindTypeD3D11(LS::BUFFER_BIND_TYPE bindType)
    {
        using enum LS::BUFFER_BIND_TYPE;
        switch (bindType)
        {
        case UNKNOWN:
            throw std::runtime_error("Failed to find a corresponding enum to the given UNKNOWN bindType");
        case VERTEX: return D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
        case INDEX: return D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
        case CONSTANT_BUFFER: return D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        case SHADER_RESOURCE: return D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
        default:
            throw std::runtime_error("Unsupported or unknown enum passed for LS::BUFFER_BIND_TYPE to D3D11_BIND_FLAG conversion");
        }
    }

    /**
     * @brief Creates a buffer desc based on the given LS::LSBuffer template class
     * @tparam TObject
     * @param buffer
     * @return
    */
    template<class TObject>
    constexpr D3D11_BUFFER_DESC CreateBufferDescD3D11(const LS::LSBuffer<TObject>& buffer)
    {
        D3D11_BUFFER_DESC out{};
        out.ByteWidth = buffer.GetSizeInBytes();
        out.Usage = FindUsageD3D11(buffer.m_usage);
        out.BindFlags = FindBufferBindTypeD3D11(buffer.m_bindType);
        out.CPUAccessFlags = 0;
        return out;
    }

}