module;
#include "vendor\imgui\imgui.h"
#include "vendor\imgui\backends\imgui_impl_win32.h"
#include "vendor\imgui\backends\imgui_impl_dx11.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

export module DearDx11;

import LSEngine;

import <cstdint>;
import <string_view>;

namespace gt::dx11
{
    LRESULT WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

export namespace gt::dx11
{
    class ImGuiDx11 : public LS::LSApp
    {
    public:
        ImGuiDx11(uint32_t width, uint32_t height, std::wstring_view title);
        ~ImGuiDx11() = default;

        auto Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        LS::LSDeviceSettings m_settings;
        LS::Win32::RenderD3D11 m_renderer;
    };
}

module : private;

using namespace gt::dx11;

ImGuiDx11::ImGuiDx11(uint32_t width, uint32_t height, std::wstring_view title) : LSApp(width, height, title),
    m_settings(LS::CreateDeviceSettings(m_Window->GetWidth(), m_Window->GetHeight(), LS::DEVICE_API::DIRECTX_11)),
    m_renderer(m_settings, m_Window.get())
{
}

auto ImGuiDx11::Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init((HWND)m_Window->GetHandleToWindow());

    auto win32Window = static_cast<LS::Win32::Win32Window*>(m_Window.get());
    using namespace std::placeholders;
    auto bind = std::bind(gt::dx11::WndProcImpl, _1, _2, _3, _4);

    win32Window->SetWndProcHandler(bind);

    if (auto deviceResult = m_renderer.Initialize(); !deviceResult)
    {
        return deviceResult;
    }

    ID3D11DeviceContext* context;
    m_renderer.GetDevice()->GetImmediateContext(&context);
    ImGui_ImplDX11_Init(m_renderer.GetDevice(), context);

    return LS::System::CreateSuccessCode();
}

void gt::dx11::ImGuiDx11::Run()
{
    m_Window->Show();
    ImGuiIO& io = ImGui::GetIO();

    std::array<float, 4> color{ 1.0f, 0.43f, 0.02f, 1.0f };
    auto currWidth = m_Window->GetWidth();
    auto currHeight = m_Window->GetHeight();
    while (m_Window->IsOpen())
    {
        m_Window->PollEvent();
        auto mousePos = m_Window->GetMousePos();
        if (currWidth != m_Window->GetWidth() && currHeight != m_Window->GetHeight())
        {
            m_renderer.Resize(m_Window->GetWidth(), m_Window->GetHeight());
            currWidth = m_Window->GetWidth();
            currHeight = m_Window->GetHeight();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImGui::Text("Width: %d", currWidth);
        ImGui::Text("Height: %d", currHeight);
        ImGui::Text("Mouse Pos: %d x %d", mousePos.x, mousePos.y);
        ImGui::SliderFloat4("Color", color.data(), 0.0f, 1.0f);

        m_renderer.Clear(color);

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            // Update and Render additional Platform Windows
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        m_renderer.Draw();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

LRESULT gt::dx11::WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Let ImGui handle itself, but we want to still allow rest of default window behavior to go to our current
    // window handler for the rest of the messages we do handle. 
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;
    return 0;
}