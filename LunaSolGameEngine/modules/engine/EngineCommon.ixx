module;
#include "LSEFramework.h"
export module Engine.Common;

export import LSData;

export import Engine.LSDevice;
export import Engine.LSWindow;
export import Engine.LSCamera;
export import Engine.EngineCodes;
#ifdef LS_WINDOWS_BUILD
export import Engine.DXCamera;
#endif

export namespace LS::Global
{
    const constinit auto FRAME_COUNT = 3u;
    const constinit auto NUM_CONTEXT = 3u;
    const constinit auto THREAD_COUNT = 4u;
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
        using AppInitFunc = std::function<int()>;
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

        void Initialize(int argCount, char* argsV[]);
        void Initialize();
        void Run();

        Ref<LSWindowBase> Window;

    protected:
        std::vector<std::string_view> CommandArgs;

    private:
        AppInitFunc InitFunc;
        AppRunFunc RunFunc;
    };

    export auto InitializeApp(uint32_t width, uint32_t height, 
        std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc) -> Ref<LSApp>;
}

module : private;

namespace LS
{
    LSApp::LSApp(uint32_t width, uint32_t height, std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc) : InitFunc(std::move(initFunc)),
        RunFunc(runFunc)
    {
        Window = BuildWindow(width, height, title);
    }

    void LSApp::Initialize(int argCount, char* argsV[])
    {
        CommandArgs = std::vector<std::string_view>(argsV + 1, argsV + argCount);

        if (InitFunc)
        {
            InitFunc();
        }
    }
    
    void LSApp::Initialize()
    {
        if (InitFunc)
        {
            InitFunc();
        }
    }

    void LSApp::Run()
    {
        if (RunFunc)
        {
            RunFunc();
        }
    }

    auto InitializeApp(uint32_t width, uint32_t height, std::wstring_view title, AppInitFunc&& initFunc, AppRunFunc&& runFunc) -> Ref<LSApp>
    {
        //LSApp out(width, height, title, std::move(initFunc), std::move(runFunc));
        auto out = std::make_unique<LSApp>(width, height, title, std::move(initFunc), std::move(runFunc));
        return out;
    }
}