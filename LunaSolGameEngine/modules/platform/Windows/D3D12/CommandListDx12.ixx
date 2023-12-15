module;
#include <cstdint>
#include <array>
#include <d3d12.h>
#include <wrl/client.h>
#include <string_view>
#include <string>

export module D3D12Lib:CommandListDx12;
import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandListDx12
    {
    public:
        CommandListDx12(D3D12_COMMAND_LIST_TYPE type, std::string_view name);
        CommandListDx12(ID3D12Device4* pDevice, D3D12_COMMAND_LIST_TYPE type, std::string_view name);
        ~CommandListDx12() = default;

        auto Initialize(ID3D12Device4* pDevice) noexcept -> LS::System::ErrorCode;

        auto GetFenceValue() const noexcept -> uint64_t
        {
            return m_fenceValue;
        }

        auto GetCommandList() const noexcept -> WRL::ComPtr<ID3D12GraphicsCommandList>
        {
            return m_pCommandList;
        }

        auto GetCommandAllocator() const noexcept -> WRL::ComPtr<ID3D12CommandAllocator>
        {
            return m_pAllocator;
        }

        auto GetName() const noexcept -> std::string_view
        {
            return m_name;
        }

        void IncrementFenceValue() noexcept
        {
            m_fenceValue++;
        }

        void ResetCommandList() noexcept;
        void Close() noexcept;
        void Clear(const std::array<float, 4>& clearColor, const D3D12_CPU_DESCRIPTOR_HANDLE rtv) noexcept;
        void ClearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE dsv, float clearValue = 1.0f, uint8_t stencilValue = 255);
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
        void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle = nullptr) noexcept;
        void SetPipelineState(ID3D12PipelineState* state) noexcept;
        void SetGraphicsRootSignature(ID3D12RootSignature* rootSignature) noexcept;
        void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept;
        void SetVertexBuffers(uint32_t startSlot, uint32_t numViews, const D3D12_VERTEX_BUFFER_VIEW* views) noexcept;
        void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* indexBuffer) noexcept;
        void SetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports) noexcept;
        void SetScissorRects(uint32_t numRects, const D3D12_RECT* rects) noexcept;
        void SetGraphicsRoot32BitConstant(uint32_t rootParamIndx, uint32_t srcData, uint32_t destOffsetIn32BitValues) noexcept;
        void SetGraphicsRoot32BitConstants(uint32_t rootParamIndx, uint32_t num32BitValueToSet, const void* pData, uint32_t destOffsetIn32BitValues) noexcept;
        void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation = 0u, int32_t  baseVertexLocation = 0u, uint32_t startInstanceLocation = 0u) noexcept;
    private:

        D3D12_COMMAND_LIST_TYPE m_type;
        uint64_t m_fenceValue;
        WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        WRL::ComPtr<ID3D12CommandAllocator> m_pAllocator;
        std::string m_name;
    };
    
}