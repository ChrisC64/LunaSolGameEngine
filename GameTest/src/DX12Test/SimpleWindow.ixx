module;
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <directx/d3dx12.h>
#include <DirectXMath.h>

export module DX12.SimpleWindow;

import <filesystem>;
import <array>;
import <vector>;
import <span>;
import <algorithm>;
import <thread>;
import <optional>;
import <stdexcept>;
import <mutex>;
import <ranges>;
import <semaphore>;
import <functional>;
import <format>;
import <memory>;
import <array>;
import <cstdint>;
import <iostream>;
import LSEngine;
import D3D12Lib;

namespace gt::dx12
{
    namespace WRL = Microsoft::WRL;
    using namespace DirectX;

    export class SimpleWindow : public LS::LSApp
    {
    public:
        SimpleWindow(uint32_t width, uint32_t height) : LSApp(width, height, L"Simple Window"),
            m_renderer(width, height, NUM_OF_FRAMES, m_Window.get())
        {
        }

        ~SimpleWindow() = default;

        auto Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        const uint32_t NUM_OF_FRAMES = 3;

        uint64_t m_fenceValue;

        LS::Platform::Dx12::RendererDX12 m_renderer;
        LS::Platform::Dx12::CommandListDx12 m_commandList;
        LS::Platform::Dx12::CommandListDx12 m_cl2;

        [[nodiscard]] bool LoadPipeline();
        void LoadAssets();
        void PopulateCommandList();
        void OnRender();
        void OnDestroy();
        void OnUpdate();
        void WaitForPreviousFrame();
    };
}

module : private;

auto gt::dx12::SimpleWindow::Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode
{
    auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_commandList = m_renderer.CreateCommandList(type, "main_cl").value();
    m_cl2 = m_renderer.CreateCommandList(type, "cl2").value();

    LoadAssets();
    return LS::System::CreateSuccessCode();
}

void gt::dx12::SimpleWindow::Run()
{
    m_Window->Show();

    m_State = LS::APP_STATE::RUNNING;
    auto currWidth = m_Window->GetWidth();
    auto currHeight = m_Window->GetHeight();
    while (m_Window->IsOpen())
    {
        m_Window->PollEvent();
        m_renderer.WaitOnNextFrame();
        if (currWidth != m_Window->GetWidth() || currHeight != m_Window->GetHeight())
        {
            if (auto result = m_renderer.Resize(m_Window->GetWidth(), m_Window->GetHeight()); 
                !result)
            {
                throw std::runtime_error(std::format("Failed to resize frame buffer. Error: {}", result.Message()));
            }
            currWidth = m_Window->GetWidth();
            currHeight = m_Window->GetHeight();
        }

        OnRender();
        OnUpdate();
    }

    OnDestroy();
}

bool gt::dx12::SimpleWindow::LoadPipeline()
{
    // Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    //m_heapSrv.Initialize(device.Get());

    return true;
}

void gt::dx12::SimpleWindow::LoadAssets()
{
}

void gt::dx12::SimpleWindow::PopulateCommandList()
{
    const LS::Platform::Dx12::FrameBufferDxgi& framebuffer = m_renderer.GetFrameBuffer();
    //const LS::Platform::Dx12::FrameDx12& frame = m_renderer.GetFrameBuffer().GetCurrentFrame();
    //TODO: Memory leak happens when using this command (better investigate why)
    m_renderer.BeginCommandList(m_commandList);
    //m_commandList.BeginFrame(frame);
    m_commandList.Clear({ 0.0f, 0.12f, 0.34f, 1.0f });
    m_renderer.EndCommandList(m_commandList);
    m_renderer.QueueCommand(&m_commandList);

    //m_cl2.BeginFrame(framebuffer);
    //m_cl2.BeginFrame(frame);
    //m_cl2.Clear({ 0.0, 1.f, 0.0f, 1.0f });
    //m_cl2.EndFrame();
    //m_renderer.QueueCommand(&m_cl2);
}

void gt::dx12::SimpleWindow::OnRender()
{
    PopulateCommandList();
    const auto fence = m_renderer.ExecuteCommands();
    m_renderer.WaitForCommands(fence);
}

void gt::dx12::SimpleWindow::OnDestroy()
{
}

void gt::dx12::SimpleWindow::OnUpdate()
{

}

void gt::dx12::SimpleWindow::WaitForPreviousFrame()
{
}