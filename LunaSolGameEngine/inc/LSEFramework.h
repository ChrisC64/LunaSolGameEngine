#pragma once

// STD C++ //
#include <memory>
#include <cstdint>
#include <cstddef>
#include <thread>
#include <vector>
#include <array>
#include <concepts>
#include <compare>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>
#include <variant>

// Windows and DirectX API Includes //
#ifdef LS_WINDOWS_BUILD
// Windows Headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
// DirectX/Windows Headers //
#include <d2d1helper.h>
#include <d2d1.h>
#include <wrl/client.h>
#include <winrt/base.h>
#include <d2d1.h>
#include <dwrite_3.h>
// D3D11 Headers //
#include <d3d11_4.h>

// DXGI Interfaces
#include <dxgi1_6.h>

// DirectX Libraries //
#include <directxmath/DirectXMath.h>
#include <directxmath/DirectXColors.h>
#include <directxtk/CommonStates.h>
#include <d3dcompiler.h>

// libs to include //
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "Dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")
#endif

// DEFINES //
#ifdef _DEBUG
#define TRACE(x) {      \
std::stringstream ss;	\
ss << x;				\
OutputDebugStringA(ss.str().c_str());\
}
#define TRACE_W(x) {						\
std::wstringstream ws;					\
 ws << x;								\
 OutputDebugStringW(ws.str().c_str()); }

#define TRACE_ERR(x) std::cerr << "ERROR: [" << __FILE__<< "]" << " [" << __func__ << "] " << "[Line: " << __LINE__ << "]" << x;
#else
#define TRACE(x) // Does nothing (should output to a file maybe?)
#define TRACE_W(x)// Does nothing (maybe output to file?)
#define TRACE_ERR(x)//Should really output to file perhapse
#endif _DEBUG
