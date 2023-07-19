module;
#include <optional>
#include <cstdint>
#include <cassert>
#include <d3d11_4.h>
#include <span>
#include <stdexcept>
#include "platform\Windows\Win32\WinApiUtils.h"
export module D3D11.MemoryHelper;
import LSData;
import Engine.Logger;

export namespace LS::Win32
{
    /**
    * @brief Finds the D3D11_USAGE type for the given LS::BUFFER_USAGE enum
    * @param bufferUsage
    * @return the value that corresponds to the given D3D11_USAGE enum from the given LS::BUFFER_USAGE enum
   */
    [[nodiscard]]
    constexpr auto FindUsageD3D11(LS::BUFFER_USAGE bufferUsage) -> D3D11_USAGE
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
    constexpr auto FindBufferBindTypeD3D11(LS::BUFFER_BIND_TYPE bindType) -> D3D11_BIND_FLAG
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

    [[nodiscard]]
    constexpr auto FindCpuAccessD3D11(LS::CPU_RESOURCE_ACCESS cpuAccess) -> D3D11_CPU_ACCESS_FLAG
    {
        using enum LS::CPU_RESOURCE_ACCESS;
        switch (cpuAccess)
        {
        case UNDEFINED:
            return (D3D11_CPU_ACCESS_FLAG)0;
        case WRITE_ONLY:
            return D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
        case READ_ONLY:
            return D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
        case WRITE_AND_READ:
            return (D3D11_CPU_ACCESS_FLAG)(D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ);
        default:
            return (D3D11_CPU_ACCESS_FLAG)0;
        }
    }

    /**
     * @brief Creates a buffer desc based on the given LS::LSBuffer template class
     * @tparam TObject
     * @param buffer
     * @return
    */
    template<class TObject>
    [[nodiscard]] constexpr auto CreateBufferDescD3D11(const LS::LSBuffer<TObject>& buffer) noexcept -> D3D11_BUFFER_DESC
    {
        D3D11_BUFFER_DESC out{};
        out.ByteWidth = buffer.GetSizeInBytes();
        out.Usage = FindUsageD3D11(buffer.m_usage);
        out.BindFlags = FindBufferBindTypeD3D11(buffer.m_bindType);
        out.CPUAccessFlags = 0;
        return out;
    }

    [[nodiscard]] constexpr auto CreateBufferDescD3D11(uint32_t byteWidth, LS::BUFFER_USAGE bufferUsage, LS::BUFFER_BIND_TYPE bindType, LS::CPU_RESOURCE_ACCESS cpuAccess = LS::CPU_RESOURCE_ACCESS::UNDEFINED)
    {
        D3D11_BUFFER_DESC out{};
        out.ByteWidth = byteWidth;
        out.Usage = FindUsageD3D11(bufferUsage);
        out.BindFlags = FindBufferBindTypeD3D11(bindType);
        out.CPUAccessFlags = FindCpuAccessD3D11(cpuAccess);
        return out;
    }

    [[nodiscard]] constexpr auto CreateBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, D3D11_BIND_FLAG bindFlags, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0) -> D3D11_BUFFER_DESC
    {
        D3D11_BUFFER_DESC out{
            .ByteWidth = byteWidth, .Usage = usage, .BindFlags = (UINT)bindFlags,
            .CPUAccessFlags = cpuAccess, .MiscFlags = miscFlags, .StructureByteStride = structuredByteStride };
        return out;
    }

    [[nodiscard]] constexpr auto CreateVertexBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0)
    {
        return CreateBufferDescD3D11(byteWidth, usage, D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER, cpuAccess, miscFlags, structuredByteStride);
    }

    [[nodiscard]] constexpr auto CreateIndexBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0)
    {
        return CreateBufferDescD3D11(byteWidth, usage, D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER, cpuAccess, miscFlags, structuredByteStride);
    }

    [[nodiscard]] constexpr auto CreateConstantBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0)
    {
        return CreateBufferDescD3D11(byteWidth, usage, D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER, cpuAccess, miscFlags, structuredByteStride);
    }

    [[nodiscard]] constexpr auto CreateShaderResourceBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0)
    {
        return CreateBufferDescD3D11(byteWidth, usage, D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE, cpuAccess, miscFlags, structuredByteStride);
    }

    [[nodiscard]] constexpr auto CreateRenderTargetBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0)
    {
        return CreateBufferDescD3D11(byteWidth, usage, D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET, cpuAccess, miscFlags, structuredByteStride);
    }

    [[nodiscard]] constexpr auto CreateDepthStencilBufferDescD3D11(uint32_t byteWidth, D3D11_USAGE usage, UINT cpuAccess = 0, UINT miscFlags = 0, UINT structuredByteStride = 0)
    {
        return CreateBufferDescD3D11(byteWidth, usage, D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL, cpuAccess, miscFlags, structuredByteStride);
    }

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

    constexpr void UpdateSubresource(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, const void* ptrData, 
        uint32_t dstSubresource = 0, uint32_t sourceRow = 0, uint32_t sourceDepth = 0, D3D11_BOX* dstBox = nullptr) noexcept
    {
        assert(pContext);
        assert(pResource);
        if (!pContext || !pResource)
            return;

        pContext->UpdateSubresource(pResource, dstSubresource, dstBox, ptrData, sourceRow, sourceDepth);
    }
    
    constexpr void UpdateSubresource1(ID3D11DeviceContext4* pContext, ID3D11Resource* pResource, const void* ptrData, 
        uint32_t dstSubresource = 0, uint32_t sourceRow = 0, uint32_t sourceDepth = 0, D3D11_BOX* dstBox = nullptr,
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

    // Resource Creation Methods //
    [[nodiscard]] constexpr auto CreateBuffer(ID3D11Device* pDevice, const D3D11_BUFFER_DESC* pDesc, 
        const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept -> HRESULT
    {
        assert(pDevice);

        return pDevice->CreateBuffer(pDesc, pInitialData, ppBuffer);
    }

    [[nodiscard]] constexpr auto CreateVertexBufferD3D11(ID3D11Device* pDevice, ID3D11Buffer** buffer, const void* data, uint32_t byteWidth,
        D3D11_USAGE bufferUsage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, 
        uint32_t structureByteStride = 0) noexcept -> HRESULT
    {
        assert(pDevice);
        D3D11_BUFFER_DESC bd = CreateVertexBufferDescD3D11(byteWidth, bufferUsage, cpuAccess, miscFlags, structureByteStride);
        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = data;
        srd.SysMemPitch = 0;
        srd.SysMemSlicePitch = 0;

        return CreateBuffer(pDevice, &bd, &srd, buffer);
    }

    [[nodiscard]] constexpr auto CreateIndexBufferD3D11(ID3D11Device* pDevice, ID3D11Buffer** buffer, const void* data, uint32_t bytes,
        D3D11_USAGE bufferUsage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, 
        uint32_t structureByteStride = 0) noexcept -> HRESULT
    {
        assert(pDevice);
        D3D11_BUFFER_DESC bd = CreateIndexBufferDescD3D11(bytes, bufferUsage, cpuAccess, miscFlags, structureByteStride);
        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = data;
        srd.SysMemPitch = 0;
        srd.SysMemSlicePitch = 0;

        return CreateBuffer(pDevice, &bd, &srd, buffer);
    }

    [[nodiscard]] constexpr auto CreateConstantBufferD3D11(ID3D11Device* pDevice, ID3D11Buffer** buffer, const void* data, uint32_t byteWidth,
        D3D11_USAGE bufferUsage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, 
        uint32_t structureByteStride = 0) noexcept -> HRESULT
    {
        assert(pDevice);
        assert(byteWidth % 16 == 0);
        D3D11_BUFFER_DESC bd = CreateConstantBufferDescD3D11(byteWidth, bufferUsage, cpuAccess, miscFlags, structureByteStride);
        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = data;
        srd.SysMemPitch = 0;
        srd.SysMemSlicePitch = 0;

        return CreateBuffer(pDevice, &bd, &srd, buffer);
    }
}