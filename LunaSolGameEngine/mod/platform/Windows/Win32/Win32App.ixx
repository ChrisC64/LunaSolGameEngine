module;
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <windowsx.h>
export module Platform.Win32App;

import Engine.Input;
import Engine.LSWindow;
import Engine.Logger;
import Engine.Defines;

import <functional>;

export namespace LS::Win32
{
    using WndProcHandler = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

    // Application has the following responsibilities
    // Managing windows created
    // Providing translation layer between Win32 Messages and Application Messages
    void InitApp(u32 width, u32 height, const wchar_t* title);
    void Shutdown();
    auto PollApp() -> LS::APP_STATE;
    void CloseWindow();
    void SetCustomWndProc(WndProcHandler handler);
    void ShowMessageBox(const wchar_t* msg, const wchar_t* title);
    void RegisterMouseMove(LS::Input::LSOnMouseMove2 callback);
    void RegisterMouseInput(LS::Input::LSOnMouseInput callback);
    void RegisterKeyboardInput(LS::Input::LSOnKeyboardInput callback);
    void GetWindowSize(uint32_t& width, uint32_t& height);

    [[nodiscard]] auto TranslateAppState(MSG msg) -> LS::APP_STATE;
    [[nodiscard]] auto GetHwnd() -> HWND;
    [[nodiscard]] bool IsKeyDown(LS::Input::KEYBOARD key);
    [[nodiscard]] bool IsKeyDownAsync(LS::Input::KEYBOARD key);
    [[nodiscard]] bool IsKeyUp(LS::Input::KEYBOARD key);
    [[nodiscard]] bool IsKeyUpAsync(LS::Input::KEYBOARD key);

    struct AppWin32
    {
        HINSTANCE Instance{};
        WNDCLASSEX WndClass{};
        HWND Hwnd{};
        WndProcHandler WndProcHandler{};
        bool IsClosing = false;
    };

    AppWin32 g_AppInstance{};
    LS::Input::LSOnMouseMove g_AppMouseMove;
    LS::Input::LSOnKeyboardInput g_AppKeyboardInput;
    LS::Input::LSOnMouseInput g_AppMouseInput;
    LS::Input::LSOnMouseWheel g_AppMouseWheel;
}

module : private;
import <stdexcept>;
import <format>;

import Win32.Utils;

using namespace LS::Win32;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (g_AppInstance.WndProcHandler)
        return g_AppInstance.WndProcHandler(hwnd, msg, wparam, lparam);

    switch (msg)
    {
    case WM_CLOSE:
    {
        g_AppInstance.IsClosing = false;
        CloseWindow();
        return 0;
    }
    case WM_QUIT:
        return 0;
    case WM_DESTROY:
        return 0;
    case WM_KEYDOWN:
        if (g_AppKeyboardInput)
        {
            g_AppKeyboardInput(ToLSKey(wparam), GetModKeys(), LS::Input::STATE::PRESS);
        }
        return 0;
    case WM_KEYUP:
        if (g_AppKeyboardInput)
        {
            g_AppKeyboardInput(ToLSKey(wparam), GetModKeys(), LS::Input::STATE::RELEASE);
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (g_AppMouseInput)
        {
            g_AppMouseInput(ToMouseButton(msg), GetModKeys(), LS::Input::STATE::PRESS);
        }
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        if (g_AppMouseInput)
        {
            g_AppMouseInput(ToMouseButton(msg), GetModKeys(), LS::Input::STATE::RELEASE);
        }
        return 0;
    case WM_MOUSEMOVE:
        if (g_AppMouseMove)
        {
            uint32_t x, y;
            x = (uint32_t)GET_X_LPARAM(lparam);
            y = (uint32_t)GET_Y_LPARAM(lparam);
            g_AppMouseMove(x, y);
        }
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

void LS::Win32::InitApp(u32 width, u32 height, const wchar_t* title)
{
    WNDCLASSEX wc{};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = title;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc))
    {
        // Error Handle 
        throw std::runtime_error("Failed to create instance");
    }

    auto style = WS_OVERLAPPEDWINDOW;

    RECT rt;
    rt.left = 0;
    rt.top = 0;
    rt.right = width;
    rt.bottom = height;
    // AdjustWindowRectExForDPI may also be worth considering //
    auto hresult = AdjustWindowRectEx(&rt, style, false, 0);

    if (!hresult)
    {
        auto error = GetLastError();
        auto eResult = HRESULT_FROM_WIN32(error);
        auto wformat = std::format(L"Failed to find window rect size. Error: {}\n", eResult);

        MessageBox(nullptr, wformat.c_str(), L"An Error Ocurred", MB_OK | MB_ICONERROR);
        throw std::runtime_error("An error ocurred");
    }

    auto newWidth = rt.right - rt.left;
    auto newHeight = rt.bottom - rt.top;
    auto hwnd = CreateWindowEx(0,
        title,
        title,
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        newWidth, newHeight,
        NULL,
        NULL,
        reinterpret_cast<HINSTANCE>(g_AppInstance.Instance),
        NULL);

    if (!hwnd)
    {
        auto result = HRESULT_FROM_WIN32(GetLastError());
        auto format = std::format(L"Failed to create Window: {}\n", result);
        MessageBox(nullptr, format.c_str(), L"An Error Ocurred", MB_OK | MB_ICONERROR);
        throw std::runtime_error("Failed to create the window");
    }

    g_AppInstance.Hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    g_AppInstance.IsClosing = false;
}

void LS::Win32::Shutdown()
{
    UnregisterClass(g_AppInstance.WndClass.lpszClassName, g_AppInstance.WndClass.hInstance);
}

auto LS::Win32::PollApp() -> LS::APP_STATE
{
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return TranslateAppState(msg);
}

void LS::Win32::CloseWindow()
{
    PostQuitMessage(0);
    //DestroyWindow(g_AppInstance.Hwnd);
}

void LS::Win32::SetCustomWndProc(WndProcHandler handler)
{
    g_AppInstance.WndProcHandler = handler;
}

void LS::Win32::ShowMessageBox(const wchar_t* msg, const wchar_t* title)
{
    MessageBox(g_AppInstance.Hwnd, msg, title, 0);
}

void LS::Win32::RegisterMouseMove(LS::Input::LSOnMouseMove2 callback)
{
    g_AppMouseMove = callback;
}

void LS::Win32::RegisterMouseInput(LS::Input::LSOnMouseInput callback)
{
    g_AppMouseInput = callback;
}

void LS::Win32::RegisterKeyboardInput(LS::Input::LSOnKeyboardInput callback)
{
    g_AppKeyboardInput = callback;
}

void LS::Win32::GetWindowSize(uint32_t& width, uint32_t& height)
{
    RECT rect;
    GetClientRect(g_AppInstance.Hwnd, &rect);
    /*GetWindowRect(g_AppInstance.Hwnd, &rect);
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, 0);*/
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}

auto LS::Win32::TranslateAppState(MSG msg) -> LS::APP_STATE
{
    using enum LS::APP_STATE;
    switch (msg.message)
    {
    case WM_DESTROY:
    case WM_QUIT:
    case WM_CLOSE:
        return QUIT;
    case WM_CREATE:
        return INITIALIZED;
    case WM_MOVE:
        return SUSPEND;
    case WM_SIZE:
        return SUSPEND;
    default:
        return RUNNING;
    }
}

[[nodiscard]]
auto LS::Win32::GetHwnd() -> HWND
{
    return g_AppInstance.Hwnd;
}

[[nodiscard]]
bool LS::Win32::IsKeyDown(LS::Input::KEYBOARD key)
{
    return GetKeyState(ToWindowsKey(key)) & 0x8000;
}

[[nodiscard]]
bool LS::Win32::IsKeyDownAsync(LS::Input::KEYBOARD key)
{
    return GetAsyncKeyState(ToWindowsKey(key)) & 0x8000;
}

[[nodiscard]]
bool LS::Win32::IsKeyUp(LS::Input::KEYBOARD key)
{
    return !IsKeyDown(key);
}

[[nodiscard]]
bool LS::Win32::IsKeyUpAsync(LS::Input::KEYBOARD key)
{
    return !IsKeyDownAsync(key);
}