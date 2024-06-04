module;
#include <dxgi1_6.h>
#include <wrl/client.h>

export module DXGISwapChain;
import <cstdint>;

export namespace LS::Win32
{
    class DXGISwapChain
    {
    public:

        DXGISwapChain() = default;
        ~DXGISwapChain() = default;

        DXGISwapChain& operator=(DXGISwapChain&&) = default;
        DXGISwapChain& operator=(const DXGISwapChain&) = default;
        DXGISwapChain(const DXGISwapChain&) = default;
        DXGISwapChain(DXGISwapChain&&) = default;

        explicit operator bool() const
        {
            return m_pSwapChain;
        }

        void Resize(uint32_t width = 0, uint32_t height = 0, uint32_t flags = 0) const noexcept;
        void UpdateBackbufferCount(uint32_t count) noexcept;

        auto GetSwapChain() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain>;
        auto GetFrontBuffer() const noexcept -> Microsoft::WRL::ComPtr<IDXGISurface>;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
    };
}

module : private;

using namespace LS::Win32;

void LS::Win32::DXGISwapChain::Resize(uint32_t width, uint32_t height, uint32_t flags) const noexcept
{
    m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, flags);
}

void LS::Win32::DXGISwapChain::UpdateBackbufferCount(uint32_t count) noexcept
{
    m_pSwapChain->ResizeBuffers(count, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
}

auto DXGISwapChain::GetSwapChain() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain>
{
    return m_pSwapChain;
}

auto LS::Win32::DXGISwapChain::GetFrontBuffer() const noexcept -> Microsoft::WRL::ComPtr<IDXGISurface>
{
    Microsoft::WRL::ComPtr<IDXGISurface> surface;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&surface));
    return surface.Get();
}