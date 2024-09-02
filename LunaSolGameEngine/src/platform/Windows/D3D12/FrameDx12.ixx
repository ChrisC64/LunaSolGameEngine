module;
#include <cassert>
#include <dxgi.h>
#include <d3d12.h>
#include <wrl/client.h>
export module D3D12Lib.FrameDx12;

export namespace LS::Platform::Dx12
{
    class FrameDx12
    {
    public:
        FrameDx12() = default;
        ~FrameDx12() = default;

        FrameDx12(const FrameDx12&) = default;
        FrameDx12(FrameDx12&&) = default;

        FrameDx12& operator=(const FrameDx12&) = default;
        FrameDx12& operator=(FrameDx12&&) = default;

        [[nodiscard]] auto GetFormat() const noexcept -> DXGI_FORMAT
        {
            return m_format;
        }

        [[nodiscard]] auto GetFrame() const noexcept -> Microsoft::WRL::ComPtr<ID3D12Resource>
        {
            return m_frame;
        }

        [[nodiscard]] auto GetFrameConst() const noexcept -> const Microsoft::WRL::ComPtr<ID3D12Resource>&
        {
            return m_frame;
        }

        [[nodiscard]] auto GetDescriptorHandle() const noexcept -> const D3D12_CPU_DESCRIPTOR_HANDLE*
        {
            return &m_descHandle;
        }

        void InitFrame(IDXGISwapChain* pSwapChain, UINT pos) noexcept
        {
            assert(pSwapChain && "Swapchain cannot be null");
            DXGI_SWAP_CHAIN_DESC desc;
            pSwapChain->GetDesc(&desc);
            assert(pos < desc.BufferCount && "Position cannot be greater or equal to back buffer count");

            if (!pSwapChain || pos >= desc.BufferCount)
                return ;

            Microsoft::WRL::ComPtr<IUnknown> temp;
            pSwapChain->GetBuffer(pos, __uuidof(ID3D12Resource), reinterpret_cast<void**>(temp.ReleaseAndGetAddressOf()));
            temp.As(&m_frame);
            const wchar_t* name = L"Frame " + pos + L'\n';
            m_frame->SetName(name);
        }

        void SetFrame(Microsoft::WRL::ComPtr<ID3D12Resource> frame)
        {
            m_frame = frame;
        }

        void SetDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) noexcept
        {
            m_descHandle = handle;
        }

        void SetDxgiFormat(DXGI_FORMAT format)
        {
            m_format = format;
        }

    private:
        DXGI_FORMAT                             m_format;
        Microsoft::WRL::ComPtr<ID3D12Resource>  m_frame;
        D3D12_CPU_DESCRIPTOR_HANDLE             m_descHandle;
    };
}