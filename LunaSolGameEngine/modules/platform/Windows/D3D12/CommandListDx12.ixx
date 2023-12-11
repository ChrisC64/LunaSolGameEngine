module;
#include <cstdint>
#include <array>
#include <d3d12.h>
#include <wrl/client.h>
#include <string_view>
#include <string>

export module D3D12Lib:CommandListDx12;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandListDx12
    {
    public:
        CommandListDx12(ID3D12Device4* pDevice, D3D12_COMMAND_LIST_TYPE type, std::string_view name);
        ~CommandListDx12() = default;

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
        void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) noexcept;

    private:

        D3D12_COMMAND_LIST_TYPE m_type;
        uint64_t m_fenceValue;
        WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        WRL::ComPtr<ID3D12CommandAllocator> m_pAllocator;
        std::string m_name;
    };
    
}