module;
#include <string>
//#include <format>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h> 
export module Win32.Utils;

export namespace LS::Win32
{
    [[nodiscard]] inline std::string HrToString(HRESULT hr)
    {
        return fmt::format("HRESULT of {:#x}", static_cast<UINT>(hr));
    }

    [[nodiscard]] inline std::wstring HrToWString(HRESULT hr)
    {
        return fmt::format(L"HRESULT of {:#x}", static_cast<UINT>(hr));
    }

    [[nodiscard]] inline HANDLE CreateEventHandler(LPCWSTR name = nullptr, BOOL isManualReset = FALSE, BOOL isSignaled = FALSE) noexcept
    {
        HANDLE eventHandle;
        eventHandle = ::CreateEventW(NULL, isManualReset, isSignaled, name);
        assert(eventHandle && "Failed to create fence event.");

        return eventHandle;
    }
}