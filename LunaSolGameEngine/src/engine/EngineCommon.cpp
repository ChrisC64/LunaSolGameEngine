#include "LSEFramework.h"

import Engine.Common;
#ifdef LS_WINDOWS_BUILD
import D3D11Lib;
import Platform.Win32Window;
#endif

namespace LS
{
    auto BuildDevice(DEVICE_API api) noexcept -> Nullable<Ref<ILSDevice>>
    {
        using enum DEVICE_API;
        switch (api)
        {
        case NONE:
            return std::nullopt;
        case DIRECTX_11:
            return std::make_unique<Win32::DeviceD3D11>();
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