#pragma once
#include <string>
#include <format>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h> 

inline std::string HrToString(HRESULT hr)
{
    return std::format("HRESULT of {:#x}", static_cast<UINT>(hr));
}

inline std::wstring HrToWString(HRESULT hr)
{
    return std::format(L"HRESULT of {:#x}", static_cast<UINT>(hr));
}
