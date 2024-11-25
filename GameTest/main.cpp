#include <streambuf>
#include <bitset>
#include <utility>
#include "engine/EngineLogDefines.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <windowsx.h>
#include <processenv.h>
#include <shellapi.h>
#include <d3dcompiler.h>

import LSEngine;

import <iostream>;
import <fstream>;
import <format>;
import <cstdint>;
import <string>;
import <filesystem>;
import <string_view>;

constexpr uint32_t SCREEN_WIDTH = 1920;
constexpr uint32_t SCREEN_HEIGHT = 1080;

#ifdef _DEBUG
int main(int argc, char* argv[])
{
    /*std::filesystem::path file = std::filesystem::current_path().string() + "log.txt";
    LS::Log::TraceError(L"Hello logger test!");
    LS::Log::TraceDebug(L"My second test!!");
    LS::Log::TraceWarn(L"WARNING!! Boss approaching!");
    LS::Log::Flush();*/
    using namespace LS::Win32;
    InitApp(SCREEN_WIDTH, SCREEN_HEIGHT, L"My new app!");
    LS::Platform::Dx12::RendererDX12 renderer(SCREEN_WIDTH, SCREEN_HEIGHT, 2, LS::Win32::GetHwnd());

    SetCustomWndProc([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            switch (msg)
            {
            case WM_LBUTTONUP:
                std::cout << "LMB Released!\n";
                //LS::Win32::ShowMessageBox(L"Hello MB", L"Message Box Approved");
                return (LRESULT)0;
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    );

    RegisterMouseMove([](double x, double y)
        {
            std::cout << std::format("X: {}, Y: {}\n", x, y);
        });

    //TODO: I want to do this following concept as:
    // 1. Create a builder (mayber as unique pointer even)
    // 2. Use builder to "build" root parameters
    // 3. Supply builder to the Renderer for it to then obtain the root parameters built
    // and have the Renderer create the root signature it needs based off the information. 
    LS::Platform::Dx12::Dx12PsoBuilder builder(0, 1);
    LS::DX::InitCompilerTools();

    const auto parentPath = LS::IO::GetParentPath();
    const auto vsPath = parentPath.string() + "\\VertexPassthrough.cso";
    const auto psPath = parentPath.string() + "\\PixelShaderPassthrough.cso";

    const auto vsData = LS::DX::DxcLoadFile(vsPath).value();
    const auto psData = LS::DX::DxcLoadFile(psPath).value();

    builder.LoadShader(vsData, LS::SHADER_TYPE::VERTEX);
    builder.LoadShader(psData, LS::SHADER_TYPE::PIXEL);
    auto& ilBuilder = builder.GetInputLayoutBuilder();
    ilBuilder.AddElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);

    const auto state1 = (uint32_t)renderer.BuildPipelineState(builder).value();
    struct Vertex
    {
        LS::Vec4F Position;
    };

    const auto aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    Vertex triangleVertices[] =
    {
        { 0.0f, 0.25f * aspectRatio, 0.0f },
        { 0.25f, -0.25f * aspectRatio, 0.0f },
        { -0.25f, -0.25f * aspectRatio, 0.0f }
    };

    // Upload to GPU // 
    //TODO: Make sure this has its own thread/queue to run off to and parallelize this for later
    const auto vbId = renderer.CreateVertexBuffer(triangleVertices, sizeof(triangleVertices), sizeof(Vertex)).value();

    auto commandList = renderer.CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, "main_cl").value();

    auto renderFrame = [&]()
        {
            renderer.BeginCommandList(commandList, state1);
            renderer.SetVertexBuffer(vbId, commandList);
            commandList.Clear({ 0.0f, 0.12f, 0.34f, 1.0f });
            commandList.DrawInstances(3, 1);
            renderer.EndCommandList(commandList);
            renderer.QueueCommand(&commandList);
        };

    while (IsAppRunning())
    {
        PollApp();
        renderer.BeginFrame();
        renderFrame();
        renderer.PresentFrame();
    }
    Shutdown();
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    using namespace LS::Win32;
    InitApp(SCREEN_WIDTH, SCREEN_HEIGHT, L"My new app!");
    LS::Platform::Dx12::RendererDX12 renderer(SCREEN_WIDTH, SCREEN_HEIGHT, 2, LS::Win32::GetHwnd());

    SetCustomWndProc([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            switch (msg)
            {
            case WM_LBUTTONUP:
                std::cout << "LMB Released!\n";
                //LS::Win32::ShowMessageBox(L"Hello MB", L"Message Box Approved");
                return (LRESULT)0;
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    );

    RegisterMouseMove([](double x, double y)
        {
            std::cout << std::format("X: {}, Y: {}\n", x, y);
        });

    LS::Platform::Dx12::Dx12PsoBuilder builder(0, 1);
    LS::DX::InitCompilerTools();

    const auto parentPath = LS::IO::GetParentPath();
    const auto vsPath = parentPath.string() + "\\VertexPassthrough.cso";
    const auto psPath = parentPath.string() + "\\PixelShaderPassthrough.cso";

    const auto vsData = LS::DX::DxcLoadFile(vsPath).value();
    const auto psData = LS::DX::DxcLoadFile(psPath).value();

    builder.LoadShader(vsData, LS::SHADER_TYPE::VERTEX);
    builder.LoadShader(psData, LS::SHADER_TYPE::PIXEL);
    auto& ilBuilder = builder.GetInputLayoutBuilder();
    ilBuilder.AddElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);

    const auto state1 = (uint32_t)renderer.BuildPipelineState(builder).value();
    struct Vertex
    {
        LS::Vec4F Position;
    };

    const auto aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    Vertex triangleVertices[] =
    {
        { 0.0f, 0.25f * aspectRatio, 0.0f },
        { 0.25f, -0.25f * aspectRatio, 0.0f },
        { -0.25f, -0.25f * aspectRatio, 0.0f }
    };

    // Upload to GPU // 
    const auto vbId = renderer.CreateVertexBuffer(triangleVertices, sizeof(triangleVertices), sizeof(Vertex)).value();

    auto commandList = renderer.CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, "main_cl").value();

    auto renderFrame = [&]()
        {
            renderer.BeginCommandList(commandList, state1);
            renderer.SetVertexBuffer(vbId, commandList);
            commandList.Clear({ 0.0f, 0.12f, 0.34f, 1.0f });
            commandList.DrawInstances(3, 1);
            renderer.EndCommandList(commandList);
            renderer.QueueCommand(&commandList);
        };

    while (IsAppRunning())
    {
        PollApp();
        renderer.BeginFrame();
        renderFrame();
        renderer.PresentFrame();
    }
    Shutdown();
}
#endif