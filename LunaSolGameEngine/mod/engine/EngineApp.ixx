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
    export auto ParseCommands(int argc, char* argv[]) noexcept -> SharedRef<LSCommandArgs>;
    export auto ParseCommands(std::string_view args) noexcept -> SharedRef<LSCommandArgs>;

    /**
     * @brief Creates the device with the supported rendering type
     * @param api @link LS::DEVICE_API type to use
     * @return A device pointer or std::nullopt if not supported.
    */
    export auto BuildDevice(DEVICE_API api) noexcept -> Nullable<Ref<ILSDevice>>;

    /**
     * @brief Builds a window for display
     * @param width the width of the window
     * @param height the height of the window
     * @param title the title to give the window
    */
    export auto BuildWindow(uint32_t width, uint32_t height, std::wstring_view title) noexcept -> Ref<LSWindowBase>;

    export class LSApp;

    export enum class APP_STATE
    {
        UNINITIALIZED,
        INITIALIZED,
        START,
        RUNNING,
        PAUSED,
        CLOSED
    };

    class LSApp
    {
    public:
        LSApp()
        {
            m_Window = BuildWindow(600, 600, L"LS Application");
        }

        virtual ~LSApp() = default;

        LSApp(const LSApp&) = delete;
        LSApp& operator=(const LSApp&) = delete;

        LSApp(LSApp&&) = default;
        LSApp& operator=(LSApp&&) = default;

        [[nodiscard]] virtual auto Initialize([[maybe_unused]] SharedRef<LSCommandArgs> args = nullptr) -> System::ErrorCode = 0;
        virtual void Run() = 0;

    protected:
        LSApp(uint32_t width, uint32_t height, std::wstring_view title);

        Ref<LSWindowBase> m_Window;
        APP_STATE m_State;
        std::filesystem::path m_appDir;
        void RegisterKeyboardInput(Input::LSOnKeyboardDown onKeyDown, Input::LSOnKeyboardUp onKeyUp);
        void RegisterMouseInput(Input::LSOnMouseDown onMouseDown, Input::LSOnMouseUp onMouseUp, Input::LSOnMouseWheelScroll mouseWheel, Input::LSOnMouseMove cursorMove);
        void BaseInit();

    private:
        void FindAppDir();
    };

    export template<class T, class... Args>
        requires std::derived_from<T, LS::LSApp>
    auto CreateApp(Args&&... args) -> LS::Ref<LS::LSApp>
    {
        LS::Ref<LS::LSApp> out = std::make_unique<T>(args...);
        return out;
    }
}

module : private;

import D3D11Lib;
import Platform.Win32Window;
import Win32.Utils;

namespace LS
{
    LSApp::LSApp(uint32_t width, uint32_t height, std::wstring_view title)
    {
        BaseInit();
        m_Window = BuildWindow(width, height, title);
    }

    void LS::LSApp::RegisterKeyboardInput(Input::LSOnKeyboardDown onKeyDown, Input::LSOnKeyboardUp onKeyUp)
    {
        m_Window->RegisterKeyboardDown(onKeyDown);
        m_Window->RegisterKeyboardUp(onKeyUp);
    }

    void LS::LSApp::RegisterMouseInput(Input::LSOnMouseDown onMouseDown, Input::LSOnMouseUp onMouseUp, Input::LSOnMouseWheelScroll mouseWheel, Input::LSOnMouseMove cursorMove)
    {
        m_Window->RegisterMouseDown(onMouseDown);
        m_Window->RegisterMouseUp(onMouseUp);
        m_Window->RegisterMouseWheel(mouseWheel);
        m_Window->RegisterMouseMoveCallback(cursorMove);
    }

    void LSApp::BaseInit()
    {
        FindAppDir();
    }

    void LSApp::FindAppDir()
    {
#ifdef LS_WIN32_BUILD
        m_appDir = LS::Win32::FindModuleDir();
#endif
    }

    auto ParseCommands(int argc, char* argv[]) noexcept -> SharedRef<LSCommandArgs>
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
    }

    auto BuildDevice(DEVICE_API api) noexcept -> Nullable<Ref<ILSDevice>>
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
    }

    auto BuildWindow(uint32_t width, uint32_t height, std::wstring_view title) noexcept -> Ref<LSWindowBase>
    {
        return std::make_unique<LS::Win32::Win32Window>(width, height, title);
    }
}