module;
#include <dxgi1_6.h>
#include <wrl/client.h>

export module DXGISwapChain;
import <cstdint>;
import Engine.Defines;

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

        [[nodiscard]]
        bool IsInit() const noexcept
        {
            return m_pSwapChain != nullptr;
        }

        [[nodiscard]]
        bool InitializeForHwnd(IDXGIFactory2* pFactory, HWND hwnd, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* desc,
            DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc = nullptr, IDXGIOutput* restrictOutput = nullptr) noexcept;

        void Resize(uint32_t width = 0, uint32_t height = 0, uint32_t flags = 0) const noexcept;
        void UpdateBackbufferCount(uint32_t count) noexcept;

        [[nodiscard]]
        auto GetSwapChain() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain>;

        [[nodiscard]]
        auto GetSwapChain() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain>;

        [[nodiscard]]
        auto GetSwapChain1() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain1>;

        [[nodiscard]]
        auto GetSwapChain1() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain1>;
        
        [[nodiscard]]
        auto GetSwapChain2() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain2>;

        [[nodiscard]]
        auto GetSwapChain2() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain2>;
        
        [[nodiscard]]
        auto GetSwapChain3() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain3>;

        [[nodiscard]]
        auto GetSwapChain3() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain3>;
        
        [[nodiscard]]
        auto GetSwapChain4() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain4>;

        [[nodiscard]]
        auto GetSwapChain4() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain4>;

        [[nodiscard]]
        auto GetFrontBuffer() const noexcept -> Microsoft::WRL::ComPtr<IDXGISurface>;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
    };

    /*[[nodiscard]]
    bool InitializeForHwnd(IDXGIFactory2* pFactory, HWND hwnd, IUnknown* pDevice, uint32_t width, uint32_t height,
        uint32_t bufferCount = 2, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ALPHA_MODE = DXGI_ALPHA_MODE_UNSPECIFIED,
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc = nullptr, IDXGIOutput* restrictOutput = nullptr) noexcept;*/
}

module : private;

import Engine.Logger;
import Engine.Defines;
import <optional>;
using namespace LS::Win32;

bool DXGISwapChain::InitializeForHwnd(IDXGIFactory2* pFactory, HWND hwnd, IUnknown* pDevice, const DXGI_SWAP_CHAIN_DESC1* desc,
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc, IDXGIOutput* restrictOutput) noexcept
{
    if (!pFactory || !pDevice)
    {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain1> temp;
    HRESULT hr = pFactory->CreateSwapChainForHwnd(
        pDevice,
        hwnd,
        desc,
        fullscreenDesc ? fullscreenDesc : nullptr,
        restrictOutput ? restrictOutput : nullptr,
        &temp
    );

    if (FAILED(hr))
    {
        return false;
    }

    hr = temp.As(&m_pSwapChain);
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

void DXGISwapChain::Resize(uint32_t width, uint32_t height, uint32_t flags) const noexcept
{
    m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, flags);
}

void DXGISwapChain::UpdateBackbufferCount(uint32_t count) noexcept
{
    m_pSwapChain->ResizeBuffers(count, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
}

auto DXGISwapChain::GetSwapChain() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain1() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain1>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain1() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain1>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain2() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain2>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain2() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain2>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain3() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain3>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain3() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain3>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain4() const noexcept -> const Microsoft::WRL::ComPtr<IDXGISwapChain4>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetSwapChain4() noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain4>
{
    return m_pSwapChain;
}

auto DXGISwapChain::GetFrontBuffer() const noexcept -> Microsoft::WRL::ComPtr<IDXGISurface>
{
    Microsoft::WRL::ComPtr<IDXGISurface> surface;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&surface));
    return surface.Get();
}