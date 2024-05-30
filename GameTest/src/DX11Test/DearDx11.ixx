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

ImGuiDx11::ImGuiDx11(uint32_t width, uint32_t height, std::wstring_view title) :
    m_settings(LS::CreateDeviceSettings(m_Window->GetWidth(), m_Window->GetHeight(), LS::DEVICE_API::DIRECTX_11)),
    m_renderer(m_settings, m_Window.get())
{
    m_Window = LS::BuildWindow(width, height, L"ImGui DX11 Sample");
}

auto ImGuiDx11::Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init((HWND)m_Window->GetHandleToWindow());

    if(auto deviceResult = m_renderer.Initialize(); !deviceResult)
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
    while (m_Window->IsOpen())
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
