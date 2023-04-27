module;
#include <d3d12.h>
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <dxgicommon.h>
#include <span>
#include <memory>
#include <string>
#include <string_view>

#pragma comment(lib, "dxguid.lib")

export module D3D12Lib:Device;

import Data.LSDataTypes;
import Engine.LSWindow;
import Platform.Win32Window;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    /**
     * @brief Represents minimum basic settings that should be set
    */
    struct MinSettings
    {
        D3D_FEATURE_LEVEL MaxFeatureLevel = D3D_FEATURE_LEVEL_12_2;
        D3D_FEATURE_LEVEL MinFeatureLevel = D3D_FEATURE_LEVEL_11_1;
        D3D_FEATURE_LEVEL CurrFeatureLevel = MinFeatureLevel;
        uint32_t FrameBufferCount = 2;
        DXGI_FORMAT PixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    };

    /**
     * @brief Additional extra settings that would be more useful for advance users for control
    */
    struct ExSettings
    {
        DXGI_SWAP_EFFECT SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        DXGI_ALPHA_MODE AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        DXGI_SCALING Scaling = DXGI_SCALING_STRETCH;
        bool IsStereoScopic = false;
    };

    struct D3D12Settings
    {
        MinSettings MinSettings;
        ExSettings ExSettings;
    };

    class DeviceD3D12
    {
    public:
        DeviceD3D12() = default;
        ~DeviceD3D12() = default;

        bool CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr) noexcept;
        void InitWindow(uint32_t width, uint32_t height, std::wstring_view title) noexcept;

        D3D12Settings Settings()
        {
            return m_Settings;
        }

        D3D12Settings Settings() const
        {
            return m_Settings;
        }

        auto Window() noexcept -> LSWindowBase*
        {
            return m_pWindow.get();
        }
        auto Window() const noexcept -> const LSWindowBase*
        {
            return m_pWindow.get();
        }

    private:
        D3D12Settings       m_Settings{};
        Ref<Win32Window>    m_pWindow = nullptr;

        // ComPtr Objects // 
        WRL::ComPtr<ID3D12Device9>      m_pDevice = nullptr;
        WRL::ComPtr<ID3D12Debug5>       m_pDebug = nullptr;
        WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue = nullptr;
        WRL::ComPtr<IDXGISwapChain4>    m_pSwapChain = nullptr;
        WRL::ComPtr<IDXGIFactory7>      m_pFactoryDxgi = nullptr;
        //TODO: Build a resource manager for the objects we'll need to create for the Direct3D 12 API

        auto FindCompatDisplay(std::span<WRL::ComPtr<IDXGIAdapter4>> adapters) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter4>>;
        bool CreateCommandQueue() noexcept;
        void CreateSwapchain();
    };
}