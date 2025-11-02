#include <streambuf>
#include <bitset>
#include <utility>
#include <chrono>
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

constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 600;

struct ConstantBuffer
{
    LS::Vec4F offset;
    float padding[60]; // Padding so the constant buffer is 256-byte aligned.
};

#ifdef _DEBUG
int main(int argc, char* argv[])
{
    /*std::filesystem::path file = std::filesystem::current_path().string() + "log.txt";
    LS::Log::TraceError(L"Hello logger test!");
    LS::Log::TraceDebug(L"My second test!!");
    LS::Log::TraceWarn(L"WARNING!! Boss approaching!");
    LS::Log::Flush();*/
    using namespace LS::Win32;
    auto app = LS::LSApp::CreateApp(SCREEN_WIDTH, SCREEN_HEIGHT, L"LS App");
    LS::Platform::Dx12::RendererDX12 renderer(SCREEN_WIDTH, SCREEN_HEIGHT, 2, (HWND)app.GetWindow());

    app.RegisterMouseMove([](uint32_t x, uint32_t y)
        {
            //std::cout << std::format("X: {}, Y: {}\n", x, y);
        });

    //TODO: Just load the shader passed in to LoadShader(std::filesystem::path) and any additional params required
    // Returns an optional object that is the ID of the shader that was compiled. 
    LS::Platform::Dx12::Dx12PsoBuilder builder(0, 2);
    LS::DX::InitCompilerTools();

    const auto parentPath = LS::IO::GetParentPath();
    //const auto vsPath = parentPath.string() + "\\VertexPassthrough.cso";
    const auto vsPath2 = parentPath.string() + "\\shader_cb.hlsl";
    const auto psPath = parentPath.string() + "\\PixelShaderPassthrough.cso";

    //const auto vsData = LS::DX::DxcLoadFile(vsPath).value();
    const auto vsCb = LS::DX::FxcCompileShader(vsPath2, "VSMain", "vs_5_0");
    //const auto psData = LS::DX::DxcLoadFile(psPath).value();
    const auto psCb = LS::DX::FxcCompileShader(vsPath2, "PSMain", "ps_5_0");

    /*auto rpBuilder = builder.GetRootParamBuilder();*/
    builder.GetRootParamBuilder()
        .BeginDescriptorRange(1)
        .CreateDescRange(LS::DESCRIPTOR_RANGE_TYPE::CBV, 1, 0)
        .EndDescriptorRange(0, LS::SHADER_VISIBILITY::VISIBILITY_VERTEX);
    /*rpBuilder.CreateCbvParam(0, 0, LS::DESCRIPTOR_RANGE_FLAGS::DATA_STATIC, LS::SHADER_VISIBILITY::VISIBILITY_VERTEX);*/
    //const auto rootParam1 = LS::Platform::Dx12::CreateRoot1ParamAsCBV(0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX);

    //builder.LoadShader(vsData, LS::SHADER_TYPE::VERTEX);
    //builder.LoadShader(psData, LS::SHADER_TYPE::PIXEL);
    builder.LoadShader(vsCb.ByteCode, LS::SHADER_TYPE::VERTEX);
    //builder.LoadShader(vsCb, LS::SHADER_TYPE::VERTEX);
    builder.LoadShader(psCb.ByteCode, LS::SHADER_TYPE::PIXEL);

    // Create input layout and supply the given ID to use its compiled data
    auto& ilBuilder = builder.GetInputLayoutBuilder();
    ilBuilder.AddElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);
    ilBuilder.AddElement("COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0);

    const auto state1 = (uint32_t)renderer.BuildPipelineState(builder).value();
    struct Vertex
    {
        LS::Vec3F Position;
        LS::Vec3F Color;
    };

    const auto aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    Vertex triangleVertices[3];
    triangleVertices[0] = Vertex{ .Position = {.x = 0.0f, .y = 0.25f * aspectRatio, .z = 0.0f }, .Color = {.x = 1.0f, .y = 1.0f, .z = 0.0f } };
    triangleVertices[1] = Vertex{ .Position = { 0.25f, -0.25f * aspectRatio, 0.0f }, .Color = {.x = 1.0f, .y = 0.0f, .z = 1.0f } };
    triangleVertices[2] = Vertex{ .Position = { -0.25f, -0.25f * aspectRatio, 0.0f }, .Color = {.x = 0.0f, .y = 1.0f, .z = 1.0f } };
    /*{
        {
            .Position = {.x = 0.0f, .y = 0.25f * aspectRatio, .z = 0.0f }, .Color = {.x = 1.0f, .y = 1.0f, .z = 0.0f }
        },
        {
            .Position = { 0.25f, -0.25f * aspectRatio, 0.0f }, .Color = {.x = 1.0f, .y = 0.0f, .z = 1.0f }
        },
        {
            .Position = { - 0.25f, -0.25f * aspectRatio, 0.0f }, .Color = {.x = 0.0f, .y = 1.0f, .z = 1.0f }
        }
    };*/

    ConstantBuffer cbData{};
    cbData.offset = LS::Vec4F{ 0.0f, 0.0f, 0.0f, 0.0f };

    // Upload to GPU // 
    //TODO: Make sure this has its own thread/queue to run off to and parallelize this for later
    const auto vbId = renderer.CreateVertexBuffer(triangleVertices, sizeof(triangleVertices), sizeof(Vertex)).value();
    const auto cbId = renderer.CreateConstantBuffer(&cbData, sizeof(ConstantBuffer)).value();

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
    LS::Vec2U currSize = app.GetWindowSize();
    //TODO: Implement a working resize event
    auto begin = std::chrono::steady_clock::now();
    std::chrono::seconds time;
    uint32_t counter = 1;
    while (app.IsRunning())
    {
        auto state = app.PollEvent();
        if (state == LS::APP_STATE::QUIT)
            break;

        LS::Vec2U newSize = app.GetWindowSize();
        if (currSize != newSize)
        {
            if (auto result = renderer.Resize(newSize.x, newSize.y); !result)
            {
                throw std::runtime_error("Failed to resize frame buffer.");
            }
            currSize = newSize;
        }
        // Update data //
        /*if (time.count() != 0 && std::chrono::seconds(counter).count() % time.count() == 0)
        {
            counter = time.count() + 1;
            cbData.offset.x += 0.05f;
            if (cbData.offset.x >= 1.25f)
            {
                cbData.offset.x = -1.00f;
            }
            std::cout << std::format("Offset: ({}, {}, {}, {})\n", cbData.offset.x, cbData.offset.y, cbData.offset.z, cbData.offset.w);
            renderer.UpdateCbData(cbId, &cbData, sizeof(ConstantBuffer));
        }*/

        cbData.offset.x += 0.0005f;
        if (cbData.offset.x > 1.25f)
        {
            cbData.offset.x = -1.25f;
        }
        renderer.UpdateCbData(cbId, &cbData, sizeof(cbData));

        auto end = std::chrono::steady_clock::now();
        time = std::chrono::duration_cast<std::chrono::seconds>(end - begin);
        renderer.BeginFrame();
        renderFrame();
        renderer.PresentFrame();
    }
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