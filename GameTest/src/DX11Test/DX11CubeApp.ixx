module;
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <directxmath/DirectXMath.h>
#include <directxmath/DirectXColors.h>
#include <dxgi1_6.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>
#include <fstream>
#include <iostream>
#include <format>
#include <compare>
#include <cmath>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <barrier>
#include <thread>
#include <d3d11_4.h>

#include "LSTimer.h"

export module DX11CubeApp;

import Engine.Common;
import D3D11Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Engine.Logger;
import Helper.IO;
import GeometryGenerator;
import MathLib;
import DirectXCommon;
import LSData;

using namespace Microsoft::WRL;
using namespace LS;
using namespace LS::Win32;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace DirectX;

struct Vertex
{
    LS::Vec4<float> Position;
    LS::Vec4<float> Color;
};

struct Cube
{
    std::array<Vertex, 8> Verts;
    std::array<uint32_t, 36> Indices;
    XMVECTOR Position;
    XMVECTOR Scale;
    XMVECTOR Rotation;
    XMMATRIX Transform;
};

namespace gt
{
    export LS::ENGINE_CODE Init();
    export void Run();

    constexpr auto SCREEN_WIDTH = 1920u;
    constexpr auto SCREEN_HEIGHT = 1080u;
    export auto App = CreateAppRef(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App", std::move(Init), std::move(Run));

#ifdef DEBUG
    const std::string shader_path = R"(build\x64\Debug\)";
#else
    const std::string shader_path = R"(build\x64\Release\)";
#endif

    LS::Win32::DeviceD3D11 g_device;
    Cube g_Cube;
    XMVECTOR g_UpVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR g_LookAtDefault = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    LS::DX::DXCamera g_camera(SCREEN_WIDTH, SCREEN_HEIGHT, XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f), g_LookAtDefault, g_UpVec, 90.0f);
    LS::DX::FreeFlyCameraControllerDX g_cameraController(g_camera);
    constexpr auto g_indexData = Geo::Generator::CreateCubeIndexArray();
    LS::LSTimer<std::uint64_t, 1ul, 1000ul> g_timer;
    ComPtr<ID3D11Buffer> g_viewBuffer, g_projBuffer, g_modelBuffer;
    ComPtr<ID3D11VertexShader> vertShader;
    ComPtr<ID3D11PixelShader> pixShader;
    ComPtr<ID3D11RasterizerState2> rsSolid;
    ComPtr<ID3D11RenderTargetView1> rtv;
    ComPtr<ID3D11DepthStencilView> dsView;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    ComPtr<ID3D11DeviceContext4> g_immContext;
    std::unordered_map<LS::LS_INPUT_KEY, bool> g_keysPressedMap;
    std::mutex g_pauseMutex;
    std::condition_variable g_pauseCondition;
    std::barrier g_pauseSync(1, []() noexcept { std::cout << "Pause sync complete.\n"; });

    auto CreateVertexShader(const LS::Win32::DeviceD3D11& device, ComPtr<ID3D11VertexShader>& shader, std::vector<std::byte>& byteCode) -> bool
    {
        auto vsResult = CreateVertexShaderFromByteCode(device.GetDevice().Get(), byteCode, &shader);
        if (FAILED(vsResult))
        {
            return false;
        }
        return true;
    }

    auto CreatePixelShader(const LS::Win32::DeviceD3D11& device, ComPtr<ID3D11PixelShader>& shader, std::vector<std::byte>& byteCode) -> bool
    {
        auto psResult = CreatePixelShaderFromByteCode(device.GetDevice().Get(), byteCode, &shader);
        if (FAILED(psResult))
        {
            return false;
        }
        return true;
    }

    void SetCameraPosition(float px, float py, float pz)
    {
        g_camera.Position = XMVectorSet(px, py, pz, 1.0f);
    }

    void UpdateCubeTransform()
    {
        XMMATRIX scale = XMMatrixScalingFromVector(g_Cube.Scale);
        //XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(g_Cube.Rotation);
        XMMATRIX rot = XMMatrixRotationQuaternion(g_Cube.Rotation);
        XMMATRIX translation = XMMatrixTranslationFromVector(g_Cube.Position);

        g_Cube.Transform = XMMatrixMultiply(translation, XMMatrixMultiply(scale, rot));
    }

    void InitCube()
    {
        g_Cube.Position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        g_Cube.Scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
        g_Cube.Rotation = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        UpdateCubeTransform();

        auto [verts, indices] = LS::Geo::Generator::CreateCubeVertsAndIndices(1.0f);

        for (auto i = 0u; i < verts.size(); ++i)
        {
            LS::Vec3<float> v = verts[i];
            g_Cube.Verts[i].Position = LS::Vec4<float>{ v.x, v.y, v.z, 1.0f };
            g_Cube.Verts[i].Color = LS::Vec4<float>{ 1.0f, 0.33f, 0.234f, 1.0f };
        }

        g_Cube.Indices = std::move(indices);
    }

    void RotateCube(uint64_t elapsed)
    {
        if (elapsed == 0)
            return;
        auto radians = LS::Math::ToRadians(10 / elapsed);
        XMVECTOR rotationAxis = XMVectorSet(0.450f, 1.0f, 0.250f, 1.0f);
        auto rotQuat = XMQuaternionRotationNormal(rotationAxis, radians);
        g_Cube.Rotation = XMQuaternionMultiply(g_Cube.Rotation, rotQuat);
    }

    void UpdateCamera()
    {
        g_camera.Mvp = g_Cube.Transform;
        g_cameraController.UpdateProjection();
        g_camera.UpdateView();
        XMFLOAT3 rightVec;
        XMFLOAT3 upVec;
        XMFLOAT3 forwardVec;
        XMStoreFloat3(&rightVec, g_camera.Right);
        XMStoreFloat3(&upVec, g_camera.Up);
        XMStoreFloat3(&forwardVec, g_camera.Forward);
    }

    void UpdateMovement()
    {
        using enum LS::LS_INPUT_KEY;
        LS::Vec3F movement;
        auto dt = g_timer.GetDeltaTime().count() * 1'000.0f;
        float movespeed = 300.0f / dt;
        for (auto [k, p] : g_keysPressedMap)
        {
            if (k == W && p)
            {
                movement.z += movespeed;
            }
            if (k == S && p)
            {
                movement.z += -movespeed;
            }
            if (k == A && p)
            {
                movement.x += -movespeed;
            }
            if (k == D && p)
            {
                movement.x += movespeed;
            }
        }

        //g_camera.TranslatePosition(movement);
        g_cameraController.Walk(movement.z);
        g_cameraController.Strafe(movement.x);
        //g_camera.Move(LS::Vec3F{.x = movement.x, .y = 0.0f, .z = movement.z});
    }

    void OnKeyboardDown(const LS::InputKeyDown& input);
    void OnKeyboardUp(const LS::InputKeyUp& input);
    void OnMouseDown(const LS::InputMouseDown& input);
    void OnMouseUp(const LS::InputMouseUp& input);
    void OnMouseMove(uint32_t x, uint32_t y);
    void OnMouseWheel(const LS::InputMouseWheelScroll& input);
    void OnWindowEvent(LS::LS_WINDOW_EVENT ev);


    void PreDraw(ComPtr<ID3D11DeviceContext4>& context);
    void DrawScene(ComPtr<ID3D11DeviceContext4>& context);
    void HandleResize(uint32_t width, uint32_t height);
    //TODO: Create common colors in Engine Common or make color conceptl
    std::array<float, 4> g_red = { 1.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 4> g_green = { 0.0f, 1.0f, 0.0f, 1.0f };
    std::array<float, 4> g_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    std::array<float, 4> g_black = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 4> g_white = { 1.0f, 1.0f, 1.0f, 1.0f };
}

module : private;
using namespace gt;

LS::ENGINE_CODE gt::Init()
{
    using enum LS::ENGINE_CODE;

    auto& window = App->Window;
    LS::ColorRGB bgColor(1.0f, 0.0f, 0.0f);
    window->SetBackgroundColor(bgColor);

    RegisterKeyboardInput(App, OnKeyboardDown, OnKeyboardUp);
    RegisterMouseInput(App, OnMouseDown, OnMouseUp, OnMouseWheel, OnMouseMove);
    window->RegisterWindowEventCallback(OnWindowEvent);
    g_device.CreateDevice();

    g_immContext = g_device.GetImmediateContext();
    /*LS::LSSwapchainInfo swapchain;
    swapchain.Width = window->GetWidth();
    swapchain.Height = window->GetHeight();
    HWND hwnd = (HWND)window->GetHandleToWindow();
    g_device.CreateSwapchain(hwnd, swapchain);*/
    g_device.CreateSwapchain(window.get());

    LS::Log::TraceDebug(L"Compiling shaders....");
    // Shader compilation //
    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size())))
    {
        throw std::runtime_error("Failed to find module path\n");
    }

    std::wstring path = std::wstring(modulePath.data());
    auto lastOf = path.find_last_of(L"\\");
    path.erase(lastOf);
    std::wstring vsPath = std::format(L"{}\\VertexShader.cso", path);
    std::wstring psPath = std::format(L"{}\\PixelShader.cso", path);

    Nullable<std::vector<std::byte>> vsFile = LS::IO::ReadFile(vsPath);
    if (!vsFile)
    {
        return FILE_ERROR;
    }
    std::vector<std::byte> vsData = vsFile.value();

    Nullable<std::vector<std::byte>> psFile = LS::IO::ReadFile(psPath);
    if (!psFile)
    {
        return FILE_ERROR;
    }
    std::vector<std::byte> psData = psFile.value();

    bool shaderResult = CreateVertexShader(g_device, vertShader, vsData);
    if (!shaderResult)
    {
        LS::Log::TraceError(L"Failed to create vertex shader");
        return RESOURCE_CREATION_FAILED;
    }

    shaderResult = CreatePixelShader(g_device, pixShader, psData);
    if (!shaderResult)
    {
        LS::Log::TraceError(L"Failed to create pixel shader");
        return RESOURCE_CREATION_FAILED;
    }
    LS::Log::TraceDebug(L"Shader Compilation Complete!!");

    // Input Layout // 
    Nullable<std::vector<D3D11_INPUT_ELEMENT_DESC>> reflectResult = BuildFromReflection(vsData);
    if (!reflectResult)
    {
        LS::Log::TraceError(L"Failed to create input layout for Vertex shader");
        return RESOURCE_CREATION_FAILED;
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescs = reflectResult.value();
    auto ilResult = g_device.CreateInputLayout(inputDescs.data(), (uint32_t)inputDescs.size(), vsData, &inputLayout);
    if (FAILED(ilResult))
    {
        LS::Log::TraceError(L"Failed to create input layout");
        return RESOURCE_CREATION_FAILED;
    }
    // Init Cube and Camera //
    InitCube();
    UpdateCamera();
    // Buffer Creation //
    // Vertex Buffer //
    LS::Log::TraceDebug(L"Creating buffers...");

    auto bufferResult = g_device.CreateVertexBuffer(g_Cube.Verts.data(), sizeof(g_Cube.Verts), &vertexBuffer);
    if (FAILED(bufferResult))
    {
        LS::Log::TraceError(L"Failed to create vertex buffer");
        return RESOURCE_CREATION_FAILED;
    }

    // Index Buffer //
    bufferResult = g_device.CreateIndexBuffer(g_indexData.data(), sizeof(g_indexData), &indexBuffer);
    if (FAILED(bufferResult))
    {
        LS::Log::TraceError(L"Failed to create index buffer.");
        return RESOURCE_CREATION_FAILED;
    }

    // Camera Buffers //
    CD3D11_BUFFER_DESC matBd(sizeof(float) * 16, D3D11_BIND_CONSTANT_BUFFER);
    D3D11_SUBRESOURCE_DATA viewSd, projSd, modelSd;

    // Setup Camera Position // 
    viewSd.pSysMem = &g_camera.View;
    projSd.pSysMem = &g_camera.Projection;
    modelSd.pSysMem = &g_camera.Mvp;

    bufferResult = g_device.CreateBuffer(&matBd, &viewSd, &g_viewBuffer);
    if (FAILED(bufferResult))
    {
        LS::Log::TraceError(L"Failed to create View matrix buffer");
        return RESOURCE_CREATION_FAILED;
    }
    bufferResult = g_device.CreateBuffer(&matBd, &projSd, &g_projBuffer);
    if (FAILED(bufferResult))
    {
        LS::Log::TraceError(L"Failed to create Projection matrix buffer");
        return RESOURCE_CREATION_FAILED;
    }
    bufferResult = g_device.CreateBuffer(&matBd, &modelSd, &g_modelBuffer);
    if (FAILED(bufferResult))
    {
        LS::Log::TraceError(L"Failed to create Model matrix buffer");
        return RESOURCE_CREATION_FAILED;
    }
    LS::Log::TraceDebug(L"Buffers created!!");
    // Rasterizer Creation // 
    LS::Log::TraceDebug(L"Building rasterizer state...");
    Nullable<ID3D11RasterizerState2*> rsSolidOpt = CreateRasterizerState2(g_device.GetDevice().Get(), SolidFill_BackCull_CCFront_DCE);
    if (!rsSolidOpt)
    {
        LS::Log::TraceError(L"Failed to create rasterizer state");
        return RESOURCE_CREATION_FAILED;
    }

    rsSolid.Attach(rsSolidOpt.value());
    LS::Log::TraceDebug(L"Rasterizer state completed!!");
    // Render Target Creation //
    LS::Log::TraceDebug(L"Building render target view....");

    HRESULT hr = g_device.CreateRTVFromBackBuffer(rtv.GetAddressOf());
    if (FAILED(hr))
    {
        LS::Log::TraceError(L"Failed to create render target view from backbuffer");
        return RESOURCE_CREATION_FAILED;
    }
    LS::Log::TraceDebug(L"Render target view created!!");
    // Depth Stencil //
    LS::Log::TraceDebug(L"Building depth stencil....");

    auto dsResult = g_device.CreateDepthStencilViewForSwapchain(rtv.Get(), &dsView);
    if (FAILED(dsResult))
        return RESOURCE_CREATION_FAILED;

    //CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    //ComPtr<ID3D11DepthStencilState> defaultState;
    //auto dss = CreateDepthStencilState(g_device.GetDevice().Get(), defaultDepthDesc).value();
    //defaultState.Attach(dss);
    //LS::Log::TraceDebug(L"Depth stencil created!!");

    return LS_SUCCESS;
}

void gt::Run()
{
    App->Window->Show();
    g_timer.Start();
    while (App->Window->IsOpen())
    {
        if (App->IsPaused)
        {
            std::cout << "Paused app!\n";
            using std::chrono::operator""ms;
            std::this_thread::sleep_for(100ms);
            continue;
        }
        auto elapsed = g_timer.GetTotalTimeTicked();
        auto deltaTime = g_timer.GetDeltaTime();
        App->Window->PollEvent();
        g_timer.Tick();

        UpdateMovement();
        RotateCube(deltaTime.count());
        UpdateCubeTransform();
        UpdateCamera();

        PreDraw(g_immContext);
        DrawScene(g_immContext);
        Present1(g_device.GetSwapChain().Get(), 1);

        // Update Buffers //
        LS::Win32::UpdateSubresource(g_immContext.Get(), g_modelBuffer.Get(), &g_camera.Mvp);
        LS::Win32::UpdateSubresource(g_immContext.Get(), g_viewBuffer.Get(), &g_camera.View);
        LS::Win32::UpdateSubresource(g_immContext.Get(), g_projBuffer.Get(), &g_camera.Projection);
    }
}

void gt::OnKeyboardDown(const LS::InputKeyDown& input)
{
    using enum LS::LS_INPUT_KEY;
    if (input.Key == ESCAPE)
    {
        App->Window->Close();
    }

    g_keysPressedMap.insert_or_assign(input.Key, true);
}

void gt::OnKeyboardUp(const LS::InputKeyUp& input)
{
    g_keysPressedMap.insert_or_assign(input.Key, false);
}
static LS::Vec2<uint32_t> g_lastPoint;
static bool IsLMBDown = false;

void gt::OnMouseDown(const LS::InputMouseDown& input)
{
    //std::cout << std::format("Mouse Down at: {}, {}\n", input.X, input.Y);
    if (input.Button == LS::LS_INPUT_MOUSE::LMB)
    {
        std::cout << "LMB Down!\n";
        g_lastPoint.x = input.X;
        g_lastPoint.y = input.Y;
        IsLMBDown = true;
    }
}

void gt::OnMouseUp(const LS::InputMouseUp& input)
{
    //std::cout << std::format("Mouse Up at: {}, {}\n", input.X, input.Y);
    if (input.Button == LS::LS_INPUT_MOUSE::LMB)
    {
        std::cout << "LMB Up!\n";
        g_lastPoint.x = input.X;
        g_lastPoint.y = input.Y;
        IsLMBDown = false;
    }
}

void gt::OnMouseMove(uint32_t x, uint32_t y)
{
    //std::cout << std::format("Mouse Move: ({}, {})\n", x, y);

    if (IsLMBDown)
    {
        int lx = x - g_lastPoint.x;
        int ly = y - g_lastPoint.y;
        auto dt = g_timer.GetDeltaTime().count() / 1'000.0f;
        float mx = lx * 10.5f * dt;
        float my = ly * 10.75f * dt;
        // Normalize between screen size //
        /*auto nx = px / (float)SCREEN_WIDTH;
        auto ny = py / (float)SCREEN_HEIGHT;*/
        //float nx = x / (float)SCREEN_WIDTH;
        //float ny = y / (float)SCREEN_HEIGHT;
        LS::Vec3F rotation = { .x = 0.0f, .y = 1.0f, .z = 0.0f };

        //auto value = nx / 360.0;
        //auto value = std::lerp(0.0f, 360.0f, nx);
        std::cout << "Value: " << mx << "\n";
        g_cameraController.RotateYaw(mx);
        g_cameraController.RotatePitch(my);
        g_lastPoint.x = x;
        g_lastPoint.y = y;
    }
}

static double deltaCounter = 50.0;
void gt::OnMouseWheel(const LS::InputMouseWheelScroll& input)
{
    std::cout << std::format("Mouse Wheel Scroll: {}, Coords: {}, {}\n", input.Delta, input.X, input.Y);
    const auto upperBounds = 120.0f;
    const auto lowerBounds = 30.0f;
    const auto counterLimit = 100.0;

    deltaCounter += input.Delta * 2.0f;

    if (deltaCounter > counterLimit)
        deltaCounter = counterLimit;
    if (deltaCounter < 0.0)
        deltaCounter = 0.0;

    auto value = std::lerp(lowerBounds, upperBounds, deltaCounter / counterLimit);

    g_camera.FovVertical = value;
}

void gt::OnWindowEvent(LS::LS_WINDOW_EVENT ev)
{
    using enum LS::LS_WINDOW_EVENT;
    switch (ev)
    {
    case CLOSE_WINDOW:
    {
        App->IsRunning = false;
        App->IsPaused = true;
    }
    break;
    case WINDOW_RESIZE_START:
    {
        App->IsPaused = true;
        break;
    }
    case WINDOW_RESIZE_END:
    {
        const auto width = App->Window->GetWidth();
        const auto height = App->Window->GetHeight();
        HandleResize(width, height);
    }
    break;
    case MAXIMIZED_WINDOW:
    {
        const auto width = App->Window->GetWidth();
        const auto height = App->Window->GetHeight();
        HandleResize(width, height);
    }
    break;
    }

}

void gt::PreDraw(ComPtr<ID3D11DeviceContext4>& context)
{
    // Set States and Objects //
    SetRenderTarget(context.Get(), rtv.Get(), dsView.Get());
    SetRasterizerState(context.Get(), rsSolid.Get());
    SetTopology(context.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetInputlayout(context.Get(), inputLayout.Get());
    SetViewport(context.Get(), static_cast<float>(App->Window->GetWidth()), static_cast<float>(App->Window->GetHeight()));
    //SetViewport(context.Get(), static_cast<float>(App->Window->GetWidth()), static_cast<float>(App->Window->GetHeight() - 100.0f), 0.0f, 100.0f);
    // Bind to State //
    BindVS(context.Get(), vertShader.Get());
    BindPS(context.Get(), pixShader.Get());
    SetVertexBuffer(context.Get(), vertexBuffer.Get(), 0, sizeof(Vertex));
    SetIndexBuffer(context.Get(), indexBuffer.Get());
    BindVSConstantBuffer(context.Get(), 0, g_viewBuffer.Get());
    BindVSConstantBuffer(context.Get(), 1, g_projBuffer.Get());
    BindVSConstantBuffer(context.Get(), 2, g_modelBuffer.Get());
    // Draw Setup //
    ClearRT(context.Get(), rtv.Get(), g_blue);
    ClearDS(context.Get(), dsView.Get());
}

void gt::DrawScene(ComPtr<ID3D11DeviceContext4>& context)
{
    DrawIndexed(context.Get(), g_indexData.size());
}

void gt::HandleResize(uint32_t width, uint32_t height)
{
    App->IsPaused = true;
    ClearDeviceDependentResources(g_immContext.Get());
    if (rtv)
    {
        rtv = nullptr;
    }
    if (dsView)
    {
        dsView = nullptr;
    }
    using std::chrono::operator""ms;
    std::this_thread::sleep_for(50ms);

    HRESULT hr = g_device.ResizeSwapchain(width, height);
    if (FAILED(hr))
    {
        LS::Log::TraceError(L"Failed to resize the swapchain buffer");
        g_device.DebugPrintLiveObjects();
        goto exit_resize;
    }
    g_device.CreateSwapchain(App->Window.get());
    hr = g_device.CreateRTVFromBackBuffer(rtv.GetAddressOf());
    if (FAILED(hr))
    {
        LS::Log::TraceError(L"Failed to create render target view from backbuffer");
        g_device.DebugPrintLiveObjects();
        goto exit_resize;
    }
    hr = g_device.CreateDepthStencilViewForSwapchain(rtv.Get(), &dsView);
    if (FAILED(hr))
    {
        LS::Log::TraceError(L"Failed to create depth stencil view from backbuffer");
        g_device.DebugPrintLiveObjects();
        goto exit_resize;
    }
exit_resize:
    App->IsPaused = false;
}
