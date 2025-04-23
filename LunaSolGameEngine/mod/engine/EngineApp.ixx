module;

export module Engine.App;
import <optional>;
import <string_view>;
import <cstdint>;
import <functional>;
import <vector>;
import <memory>;
import <ranges>;
import <filesystem>;

import Engine.LSDevice;
import Engine.LSWindow;
import Engine.LSCamera;
import Engine.EngineCodes;
import Engine.Defines;
import Engine.Input;

import LSDataLib;

#ifdef LS_WIN32_BUILD
import Platform.Win32App;
#endif//LS_WIN32_BUILD

//TODO: I'll be removing this 
export namespace LS::Global
{
    //TODO: Not sure I like this, let's consider altering later
    const constinit auto FRAME_COUNT = 3u;
    const constinit auto NUM_CONTEXT = 3u;
    const constinit auto THREAD_COUNT = 4u;
    uint32_t FrameIndex = 0u;
}

namespace LS
{
    export using LSCommandArgs = std::vector<std::string>;
    //export auto ParseCommands(int argc, char* argv[]) noexcept -> SharedRef<LSCommandArgs>;
    //export auto ParseCommands(std::string_view args) noexcept -> SharedRef<LSCommandArgs>;

    /**
     * @brief Creates the device with the supported rendering type
     * @param api @link LS::DEVICE_API type to use
     * @return A device pointer or std::nullopt if not supported.
    */
    //export auto BuildDevice(DEVICE_API api) noexcept -> Nullable<Ref<ILSDevice>>;

    export class LSApp
    {
    private:
        LSApp(uint32_t width, uint32_t height, std::wstring_view title)
        {
            BaseInit();
#ifdef LS_WIN32_BUILD
            Win32::InitApp(width, height, title.data());
#endif//LS_WIN32_BUILD
        }

    public:
        static LSApp CreateApp(uint32_t width, uint32_t height, std::wstring_view title);

        ~LSApp();

        LSApp(const LSApp&) = delete;
        LSApp& operator=(const LSApp&) = delete;

        LSApp(LSApp&&) = default;
        LSApp& operator=(LSApp&&) = default;

        void RegisterMouseMove(LS::Input::LSOnMouseMove callback);
        void RegisterKeybaordInput(LS::Input::LSOnKeyboardInput callback);
        //[[nodiscard]] auto Initialize([[maybe_unused]] SharedRef<LSCommandArgs> args = nullptr) -> System::ErrorCode;
        bool IsRunning();
        auto PollEvent() -> LS::APP_STATE;
        [[nodiscard]] void* GetWindow();
        Vec2U GetWindowSize();
#ifdef LS_WIN32_BUILD
        void SetCustomWndProc(Win32::WndProcHandler wndProcCallback);
#endif//LS_WIN32_BUILD

    protected:
        APP_STATE m_state;
        std::filesystem::path m_appDir;

    private:
        void BaseInit();
        void FindAppDir();
    };
}

module : private;

#ifdef LS_WIN32_BUILD
import Win32.Utils;
import D3D11Lib;
#endif//LS_WIN32_BUILD

namespace LS
{
    LSApp::~LSApp()
    {
#ifdef LS_WIN32_BUILD
        Win32::Shutdown();
#endif
    }

    LSApp LSApp::CreateApp(uint32_t width, uint32_t height, std::wstring_view title)
    {
        LSApp app(width, height, title);
        return app;
    }

    void LSApp::RegisterMouseMove(Input::LSOnMouseMove callback)
    {
        Win32::g_AppMouseMove = callback;
    }

    void LSApp::RegisterKeybaordInput(Input::LSOnKeyboardInput callback)
    {
        Win32::g_AppKeyboardInput = callback;
    }

    void LSApp::BaseInit()
    {
        FindAppDir();
        m_state = LS::APP_STATE::INITIALIZED;
    }

    void LSApp::FindAppDir()
    {
#ifdef LS_WIN32_BUILD
        m_appDir = LS::Win32::FindModuleDir();
#endif
    }

    bool LSApp::IsRunning()
    {
#ifdef LS_WIN32_BUILD
        return Win32::g_AppInstance.IsClosing != 1;
#endif
    }

    auto LSApp::PollEvent() -> LS::APP_STATE
    {
#ifdef LS_WIN32_BUILD
        return Win32::PollApp();
#endif//LS_WIN32_BUILD
    }

    void* LSApp::GetWindow()
    {
#ifdef LS_WIN32_BUILD
        return (void*)Win32::g_AppInstance.Hwnd;
#endif//LS_WIN32_BUILD
    }

    Vec2U LSApp::GetWindowSize()
    {
#ifdef LS_WIN32_BUILD
        uint32_t width, height;
        Win32::GetWindowSize(width, height);
        return Vec2U{ .x = width, .y = height };
#endif//LS_WIN32_BUILD
    }

#ifdef LS_WIN32_BUILD
    void LSApp::SetCustomWndProc(Win32::WndProcHandler wndProcCallback)
    {
        Win32::g_AppInstance.WndProcHandler = wndProcCallback;
    }
#endif//LS_WIN32_BUILD

    /*auto ParseCommands(int argc, char* argv[]) noexcept -> SharedRef<LSCommandArgs>
    {
        SharedRef<LSCommandArgs> commandArgs = std::make_shared<LSCommandArgs>();
        for (int i = 0; i < argc; ++i)
        {
            commandArgs->emplace_back(argv[i]);
        }

        return commandArgs;
    }

    auto ParseCommands([[maybe_unused]] std::string_view args) noexcept -> SharedRef<LSCommandArgs>
    {
        SharedRef<LSCommandArgs> commandArgs = std::make_shared<LSCommandArgs>();
        const auto delim = ' ';
        std::string arg;
        for (auto i = 0; i < args.size(); ++i)
        {
            if (args[i] == delim)
            {
                commandArgs->push_back(arg);
            }
            arg += args[i];
        }

        return commandArgs;
    }*/

    /*auto BuildDevice(DEVICE_API api) noexcept -> Nullable<Ref<ILSDevice>>
    {
        using enum DEVICE_API;
        switch (api)
        {
        case NONE:
            return std::nullopt;
        case DIRECTX_11:
            return std::make_unique<LS::Win32::DeviceD3D11>();
        case DIRECTX_12:
            return std::nullopt;
        default:
            return std::nullopt;
        }
    }*/

}