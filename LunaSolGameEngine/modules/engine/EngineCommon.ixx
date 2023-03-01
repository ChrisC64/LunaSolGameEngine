module;
#include "LSEFramework.h"
export module Engine.Common;

export import LSData;

export import Engine.LSApp;
export import Engine.LSDevice;
export import Engine.LSWindow;
export import Engine.LSCamera;

export namespace LS
{
    /**
     * @brief Creates the device with the supported rendering type
     * @param api @link LS::DEVICE_API type to use
     * @return A device pointer or std::nullopt if not supported.
    */
    auto BuildDevice(DEVICE_API api) noexcept -> Nullable<Ref<ILSDevice>>;

    /**
     * @brief Builds a window for display
     * @param width the width of the window
     * @param height the height of the window
     * @param title the title to give the window
    */
    auto BuildWindow(uint32_t width, uint32_t height, std::wstring_view title) noexcept -> Ref<LSWindowBase>;
}