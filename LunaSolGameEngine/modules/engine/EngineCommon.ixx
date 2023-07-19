module;
#include <optional>
#include <string_view>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>

export module Engine.Common;

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

    export
    {
        using AppInitFunc = std::function<LS::System::ErrorCode()>;
        using AppRunFunc = std::function<void()>;
    }

    struct LSApp
    {
        LSApp(uint32_t width, uint32_t height,
            std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc);
        ~LSApp() = default;

        LSApp(const LSApp&) = delete;
        LSApp& operator=(const LSApp&) = delete;

        LSApp(LSApp&&) = default;
        LSApp& operator=(LSApp&&) = default;

        System::ErrorCode Initialize(int argCount, char* argsV[]);
        System::ErrorCode Initialize();
        void Run();

        Ref<LSWindowBase> Window;
        std::vector<std::string_view> CommandArgs;
        bool IsRunning = false;
        bool IsPaused = false;
    private:
        AppInitFunc InitFunc;
        AppRunFunc RunFunc;
    };

    export auto CreateAppRef(uint32_t width, uint32_t height,
        std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc) -> Ref<LSApp>;

    export void RegisterKeyboardInput(Ref<LSApp>& app, LSOnKeyboardDown onKeyDown, LSOnKeyboardUp onKeyUp);
    export void RegisterMouseInput(Ref<LSApp>& app, LSOnMouseDown onMouseDown, LSOnMouseUp onMouseUp, LSOnMouseWheelScroll mouseWheel, LSOnMouseMove cursorMove);
}

module : private;

namespace LS
{
    LSApp::LSApp(uint32_t width, uint32_t height, std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc) : InitFunc(std::move(initFunc)),
        RunFunc(runFunc)
    {
        Window = BuildWindow(width, height, title);
    }

    System::ErrorCode LSApp::Initialize(int argCount, char* argsV[])
    {
        CommandArgs = std::vector<std::string_view>(argsV + 1, argsV + argCount);

        if (InitFunc)
        {
            return InitFunc();
        }
        return System::ErrorCode(System::ErrorStatus::SUCCESS, System::ErrorCategory::GENERAL, "App initialized");
    }

    System::ErrorCode LSApp::Initialize()
    {
        if (InitFunc)
        {
            return InitFunc();
        }
        return System::ErrorCode(System::ErrorStatus::SUCCESS, System::ErrorCategory::GENERAL, "App initialized");
    }

    void LSApp::Run()
    {
        IsRunning = true;
        if (RunFunc)
        {
            RunFunc();
        }
    }

    auto CreateAppRef(uint32_t width, uint32_t height, std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc) -> Ref<LSApp>
    {
        auto out = std::make_unique<LSApp>(width, height, title, std::move(initFunc), std::move(runFunc));
        return out;
    }

    void RegisterKeyboardInput(Ref<LSApp>& app, LSOnKeyboardDown onKeyDown, LSOnKeyboardUp onKeyUp)
    {
        if (!app)
            return;
        app->Window->RegisterKeyboardDown(onKeyDown);
        app->Window->RegisterKeyboardUp(onKeyUp);
    }
    void RegisterMouseInput(Ref<LSApp>& app, LSOnMouseDown onMouseDown, LSOnMouseUp onMouseUp, LSOnMouseWheelScroll mouseWheel, LSOnMouseMove cursorMove)
    {
        if (!app)
            return;
        app->Window->RegisterMouseDown(onMouseDown);
        app->Window->RegisterMouseUp(onMouseUp);
        app->Window->RegisterMouseWheel(mouseWheel);
        app->Window->RegisterMouseMoveCallback(cursorMove);
    }
}