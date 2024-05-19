module;
#include <optional>
#include <string_view>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <ranges>
export module Engine.App;

import LSEDataLib;

import Engine.LSDevice;
import Engine.LSWindow;
import Engine.LSCamera;
import Engine.EngineCodes;
import Engine.Defines;

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

    class LSApp
    {
    public:
        LSApp()
        {
            Window = BuildWindow(600, 600, L"LS Application");
        }

        LSApp(uint32_t width, uint32_t height, std::wstring_view title);
        virtual ~LSApp() = default;

        LSApp(const LSApp&) = delete;
        LSApp& operator=(const LSApp&) = delete;

        LSApp(LSApp&&) = default;
        LSApp& operator=(LSApp&&) = default;

        [[nodiscard]] virtual auto Initialize([[maybe_unused]] SharedRef<LSCommandArgs> args) -> System::ErrorCode = 0;
        virtual void Run() = 0;

    protected:
        Ref<LSWindowBase> Window;
        //TODO: Get rid of this ugly mess.
        bool IsRunning = false;
        bool IsPaused = false;

        void RegisterKeyboardInput(LSOnKeyboardDown onKeyDown, LSOnKeyboardUp onKeyUp);
        void RegisterMouseInput(LSOnMouseDown onMouseDown, LSOnMouseUp onMouseUp, LSOnMouseWheelScroll mouseWheel, LSOnMouseMove cursorMove);
    };
}

module : private;

namespace LS
{
    LSApp::LSApp(uint32_t width, uint32_t height, std::wstring_view title) 
    {
        Window = BuildWindow(width, height, title);
    }

    void LS::LSApp::RegisterKeyboardInput(LSOnKeyboardDown onKeyDown, LSOnKeyboardUp onKeyUp)
    {
        Window->RegisterKeyboardDown(onKeyDown);
        Window->RegisterKeyboardUp(onKeyUp);
    }
    void LS::LSApp::RegisterMouseInput(LSOnMouseDown onMouseDown, LSOnMouseUp onMouseUp, LSOnMouseWheelScroll mouseWheel, LSOnMouseMove cursorMove)
    {
        Window->RegisterMouseDown(onMouseDown);
        Window->RegisterMouseUp(onMouseUp);
        Window->RegisterMouseWheel(mouseWheel);
        Window->RegisterMouseMoveCallback(cursorMove);
    }

    auto ParseCommands(int argc, char* argv[]) noexcept -> SharedRef<LSCommandArgs>
    {
        SharedRef<LSCommandArgs> commandArgs = std::make_shared<LSCommandArgs>();
        int i = 1;
        for (; i < argc; ++i)
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
}