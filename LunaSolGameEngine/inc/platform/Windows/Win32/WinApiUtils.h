#pragma once
#include <string>
#include <format>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h> 
namespace LS::Win32
{
	[[nodiscard]] inline std::string HrToString(HRESULT hr)
	{
		return std::format("HRESULT of {:#x}", static_cast<UINT>(hr));
	}

	[[nodiscard]] inline std::wstring HrToWString(HRESULT hr)
	{
		return std::format(L"HRESULT of {:#x}", static_cast<UINT>(hr));
	}

	[[nodiscard]] inline HANDLE CreateEventHandler(LPCWSTR name = nullptr, BOOL isManualReset = FALSE, BOOL isSignaled = FALSE) noexcept
	{
		HANDLE eventHandle;
		eventHandle = ::CreateEventW(NULL, isManualReset, isSignaled, name);
		assert(eventHandle && "Failed to create fence event.");

		return eventHandle;
	}
}