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
#include <array>
#pragma comment(lib, "dxguid.lib")

export module D3D12Lib:Device;

import :D3D12Common;
import :ResourceManagerD3D12;

import Platform.Win32Window;
import Engine.Common;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    class DeviceD3D12
    {
    public:
        DeviceD3D12() = default;
        DeviceD3D12(const D3D12Settings settings);
        ~DeviceD3D12() = default;

        /**
         * @brief Perform basic rendering setup and display a window
         * @param width Pixel width of display area
         * @param height Pixel height of display area
         * @param title Title of window
         * @param displayAdapter (optional) the DXGI display adapter to use, if null will find first available display
         * @return True if operation was a success, false if it failed
        */
        bool Initialize(uint32_t width, uint32_t height, std::wstring_view title, WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr) noexcept;

    private:
        /**
         * @brief Initialize the D3D12 Device object
         * @param displayAdapter The display to use, if none if provided, it will find the first available display to use
         * @return true if operation was a success, false if an error occurred.
        */
        bool CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr) noexcept;
        /**
         * @brief Creates and displays the window for this renderer
         * @param width Pixel width
         * @param height Pixel Height
         * @param title Title for window
        */
        void InitWindow(uint32_t width, uint32_t height, std::wstring_view title) noexcept;

        /**
         * @brief Attaches and takes ownership of a Window for the renderer to use and manage
         * @param window The window to own and use for rendering
        */
        void TakeWindow(Ref<Win32Window>& window) noexcept;

        /**
         * @brief Find a compatible display from the objects provided that meets the minimum feature level in @link D3D12Settings
         * @param adapters list of display adapters to iterate through
         * @return optional value that may contain objects or none if there is no display that meets the requirement
        */
        auto FindCompatDisplay(std::span<WRL::ComPtr<IDXGIAdapter4>> adapters) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter4>>;
        bool CreateCommandQueue() noexcept;
        void CreateSwapchain();
        
        // Objects of Class // 
        D3D12Settings        m_Settings{};
        Ref<Win32Window>     m_pWindow = nullptr;
        uint64_t             m_frameIndex = 0u;
        ResourceManagerD3D12 m_resManager;
        //TODO: Build a resource manager for the objects we'll need to create for the Direct3D 12 API
        // ComPtr Objects // 
        WRL::ComPtr<ID3D12Device9>      m_pDevice = nullptr;
        WRL::ComPtr<ID3D12Debug5>       m_pDebug = nullptr;
        WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue = nullptr;
        WRL::ComPtr<IDXGISwapChain4>    m_pSwapChain = nullptr;
        WRL::ComPtr<IDXGIFactory7>      m_pFactoryDxgi = nullptr;


    public: // Public inline functions (mainly getters/setters)
        D3D12Settings Settings()
        {
            return m_Settings;
        }

        D3D12Settings Settings() const
        {
            return m_Settings;
        }

        auto Window() noexcept -> Win32Window*
        {
            return m_pWindow.get();
        }
        auto Window() const noexcept -> const Win32Window*
        {
            return m_pWindow.get();
        }

        auto WindowBase() const noexcept -> LSWindowBase*
        {
            return m_pWindow.get();
        }

        auto WindowBase() noexcept -> LSWindowBase*
        {
            return m_pWindow.get();
        }

    };
}