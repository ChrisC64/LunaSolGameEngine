module;
#include <optional>
#include <string_view>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

export module Engine.App;

export import LSData;

export import Engine.LSDevice;
export import Engine.LSWindow;
export import Engine.LSCamera;
export import Engine.EngineCodes;

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

        [[nodiscard]] virtual auto Initialize([[maybe_unused]] int argCount = 0, [[maybe_unused]] char* argsV[] = nullptr) -> System::ErrorCode = 0;
        virtual void Run() = 0;

    protected:
        Ref<LSWindowBase> Window;
        std::vector<std::string> CommandArgs;
        //TODO: Get rid of this ugly mess.
        bool IsRunning = false;
        bool IsPaused = false;

        void InitCommandArgs(int argCount, char* argsV[], bool skipFirst = true)
        {
            int i = skipFirst ? 1 : 0;
            for (; i < argCount; ++i)
            {
                CommandArgs.emplace_back(argsV[i]);
            }
        }

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
}