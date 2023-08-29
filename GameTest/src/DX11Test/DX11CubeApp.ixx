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
#include <iterator>
#include <string>
#include <ranges>
#include <functional>

#include <d3d11_4.h>
#include "LSTimer.h"

export module DX11CubeApp;

import Engine.App;
import D3D11Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Engine.Logger;
import Helper.IO;
import GeometryGenerator;
import MathLib;
import DirectXCommon;
import LSData;
import LSE.Serialize.WavefrontObj;
import DX11Systems;


using namespace Microsoft::WRL;
using namespace LS;
using namespace LS::System;
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

namespace gt::dx11
{
    class DX11CubeApp : LSApp
    {
    public:
        DX11CubeApp() = default;
        ~DX11CubeApp() = default;

        LS::System::ErrorCode Init();
        void Run();

    private:
        LS::Platform::Dx11::BufferCache m_bufferCache;

    public:
        void PreDraw(ComPtr<ID3D11DeviceContext4>& context);
        void DrawScene(ComPtr<ID3D11DeviceContext4>& context);
        void HandleResize(uint32_t width, uint32_t height);

        void OnKeyboardDown(const LS::InputKeyDown& input);
        void OnKeyboardUp(const LS::InputKeyUp& input);
        void OnMouseDown(const LS::InputMouseDown& input);
        void OnMouseUp(const LS::InputMouseUp& input);
        void OnMouseMove(uint32_t x, uint32_t y);
        void OnMouseWheel(const LS::InputMouseWheelScroll& input);
        void OnWindowEvent(LS::LS_WINDOW_EVENT ev);
        void ReadOBJFile(std::filesystem::path path);
    };


    using namespace std::placeholders;

    DX11CubeApp g_cubeApp;
    constexpr auto bindInit = std::bind(&gt::dx11::DX11CubeApp::Init, &g_cubeApp);
    constexpr auto bindRun = std::bind(&gt::dx11::DX11CubeApp::Run, &g_cubeApp);
    constexpr auto bindResize = std::bind(&gt::dx11::DX11CubeApp::HandleResize, &g_cubeApp, _1, _2);
    constexpr auto bindOnWindowEvent = std::bind(&gt::dx11::DX11CubeApp::OnWindowEvent, &g_cubeApp, _1);
    constexpr auto bindOnKeyboardUp = std::bind(&gt::dx11::DX11CubeApp::OnKeyboardUp, &g_cubeApp, _1);
    constexpr auto bindOnKeyboardDown = std::bind(&gt::dx11::DX11CubeApp::OnKeyboardDown, &g_cubeApp, _1);
    constexpr auto bindOnMouseDown = std::bind(&gt::dx11::DX11CubeApp::OnMouseDown, &g_cubeApp, _1);
    constexpr auto bindOnMouseUp = std::bind(&gt::dx11::DX11CubeApp::OnMouseUp, &g_cubeApp, _1);
    constexpr auto bindOnMouseMove = std::bind(&gt::dx11::DX11CubeApp::OnMouseMove, &g_cubeApp, _1, _2);
    constexpr auto bindOnMouseWheel = std::bind(&gt::dx11::DX11CubeApp::OnMouseWheel, &g_cubeApp, _1);

    constexpr auto SCREEN_WIDTH = 1920u;
    constexpr auto SCREEN_HEIGHT = 1080u;

    export auto App = CreateAppRef(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App", std::move(bindInit), std::move(bindRun));

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
    ComPtr<ID3D11VertexShader> vertShader;
    ComPtr<ID3D11PixelShader> pixShader;
    ComPtr<ID3D11RasterizerState2> rsSolid;
    ComPtr<ID3D11RenderTargetView1> rtv;
    ComPtr<ID3D11DepthStencilView> dsView;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11InputLayout> objIL;
    ComPtr<ID3D11DeviceContext4> g_immContext;

    std::unordered_map<LS::LS_INPUT_KEY, bool> g_keysPressedMap;
    std::mutex g_pauseMutex;
    std::condition_variable g_pauseCondition;
    std::barrier g_pauseSync(1, []() noexcept { std::cout << "Pause sync complete.\n"; });
    LS::Serialize::WavefrontObj m_objFile;
    
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
        auto radians = LS::Math::ToRadians(10 / static_cast<float>(elapsed));
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

        g_cameraController.Walk(movement.z);
        g_cameraController.Strafe(movement.x);
    }

    //TODO: Create common colors in Engine Common or make color conceptl
    std::array<float, 4> g_red = { 1.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 4> g_green = { 0.0f, 1.0f, 0.0f, 1.0f };
    std::array<float, 4> g_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    std::array<float, 4> g_black = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 4> g_white = { 1.0f, 1.0f, 1.0f, 1.0f };

    std::vector<float> g_objVert;
    std::vector<float> g_objNormals;
    std::vector<float> g_objUvCoords;
    std::vector<int> g_objIndices;
}

module : private;
using namespace gt;
using namespace LS;

LS::System::ErrorCode gt::dx11::DX11CubeApp::Init()
{
    using enum LS::System::ErrorStatus;
    auto& window = App->Window;
    LS::ColorRGB bgColor(1.0f, 0.0f, 0.0f);
    window->SetBackgroundColor(bgColor);

    RegisterKeyboardInput(App, bindOnKeyboardDown, bindOnKeyboardUp);
    RegisterMouseInput(App, bindOnMouseDown, bindOnMouseUp, bindOnMouseWheel, bindOnMouseMove);
    window->RegisterWindowEventCallback(bindOnWindowEvent);
    g_device.CreateDevice();

    g_immContext = g_device.GetImmediateContext();
    g_device.CreateSwapchain(window.get());

    LS::Log::TraceDebug(L"Compiling shaders....");
    // Shader compilation //
    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size())))
    {
        return CreateFailCode("Failed to find module path.");
    }

    std::wstring path = std::wstring(modulePath.data());
    auto lastOf = path.find_last_of(L"\\");
    path.erase(lastOf);
    std::wstring vsPath = std::format(L"{}\\VertexShader.cso", path);
    std::wstring psPath = std::format(L"{}\\PixelShader.cso", path);

    Nullable<std::vector<std::byte>> vsFile = LS::IO::ReadFile(vsPath);
    if (!vsFile)
    {
        return CreateFailCode("Failed to read VertexShader.cso file");;
    }
    std::vector<std::byte> vsData = vsFile.value();

    Nullable<std::vector<std::byte>> psFile = LS::IO::ReadFile(psPath);
    if (!psFile)
    {
        return CreateFailCode("Failed to read PixelShader.cso");
    }
    std::vector<std::byte> psData = psFile.value();

    bool shaderResult = CreateVertexShader(g_device, vertShader, vsData);
    if (!shaderResult)
    {
        LS::Log::TraceError(L"Failed to create vertex shader");
        return CreateFailCode("Failed to create vertex shader");
    }

    shaderResult = CreatePixelShader(g_device, pixShader, psData);
    if (!shaderResult)
    {
        LS::Log::TraceError(L"Failed to create pixel shader");
        return CreateFailCode("Failed to create pixel shader");
    }
    LS::Log::TraceDebug(L"Shader Compilation Complete!!");

    // Input Layout // 
    Nullable<std::vector<D3D11_INPUT_ELEMENT_DESC>> reflectResult = BuildFromReflection(vsData);
    if (!reflectResult)
    {
        LS::Log::TraceError(L"Failed to create input layout for Vertex shader");
        return CreateFailCode("Failed to create input layout from reflection.");
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescs = reflectResult.value();
    auto ilResult = g_device.CreateInputLayout(inputDescs.data(), (uint32_t)inputDescs.size(), vsData, &inputLayout);
    if (FAILED(ilResult))
    {
        LS::Log::TraceError(L"Failed to create input layout");
        return CreateFailCode("Failed to create input layout");
    }
    // Init Cube and Camera //
    InitCube();
    UpdateCamera();

    // Buffer Creation //
    // Vertex Buffer //
    LS::Log::TraceDebug(L"Creating buffers...");

    const auto vbOpt = LS::Platform::Dx11::CreateVertexBuffer(g_device.GetDevice().Get(), g_Cube.Verts);
    if (!vbOpt)
    {
        LS::Log::TraceError(L"Failed to create vertex buffer");
        return CreateFailCode("Failed to create vertex buffer");
    }

    if (const auto result = m_bufferCache.Insert("vertex_buffer", vbOpt.value()); !result)
    {
        return result;
    }
    
    // Index Buffer //
    const auto ibOpt = LS::Platform::Dx11::CreateIndexBuffer(g_device.GetDevice().Get(), g_indexData);
    if (!ibOpt)
    {
        LS::Log::TraceError(L"Failed to create index buffer.");
        return CreateFailCode("Failed to create index buffer");
    }

    if (const auto result = m_bufferCache.Insert("index_buffer", ibOpt.value()); !result)
    {
        return result;
    }

    // Camera Buffers //
    auto bufferOptional = LS::Platform::Dx11::CreateConstantBuffer(g_device.GetDevice().Get(), g_camera.View);
    if (!bufferOptional)
        return CreateFailCode("Failed to create camera view matrix buffer");

    if (const auto result = m_bufferCache.Insert("cam_view", bufferOptional.value()); !result)
    {
        return result;
    }

    bufferOptional = LS::Platform::Dx11::CreateConstantBuffer(g_device.GetDevice().Get(), g_camera.Projection);
    if (!bufferOptional)
        return CreateFailCode("Failed to create camera projection matrix buffer");
    
    if (const auto result = m_bufferCache.Insert("cam_proj", bufferOptional.value()); !result)
    {
        return result;
    }

    bufferOptional = LS::Platform::Dx11::CreateConstantBuffer(g_device.GetDevice().Get(), g_camera.Mvp);
    if (!bufferOptional)
        return CreateFailCode("Failed to create camera MVP matrix buffer");
    
    if (const auto result = m_bufferCache.Insert("cam_mvp", bufferOptional.value()); !result)
    {
        return result;
    }

    LS::Log::TraceDebug(L"Buffers created!!");
    // Rasterizer Creation // 
    LS::Log::TraceDebug(L"Building rasterizer state...");
    Nullable<ID3D11RasterizerState2*> rsSolidOpt = CreateRasterizerState2(g_device.GetDevice().Get(), SolidFill_BackCull_FCW_DCE);
    if (!rsSolidOpt)
    {
        LS::Log::TraceError(L"Failed to create rasterizer state");
        return CreateFailCode("Failed to create rasterizer state");
    }

    rsSolid.Attach(rsSolidOpt.value());
    LS::Log::TraceDebug(L"Rasterizer state completed!!");
    // Render Target Creation //
    LS::Log::TraceDebug(L"Building render target view....");

    HRESULT hr = g_device.CreateRTVFromBackBuffer(rtv.GetAddressOf());
    if (FAILED(hr))
    {
        LS::Log::TraceError(L"Failed to create render target view from backbuffer");
        return CreateFailCode("Failed to create render target from back buffer");
    }
    LS::Log::TraceDebug(L"Render target view created!!");
    // Depth Stencil //
    LS::Log::TraceDebug(L"Building depth stencil....");

    auto dsResult = g_device.CreateDepthStencilViewForSwapchain(rtv.Get(), &dsView);
    if (FAILED(dsResult))
    {
        return CreateFailCode("Failed to create depth stencil");
    }
    /*CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    ComPtr<ID3D11DepthStencilState> defaultState;
    auto dss = CreateDepthStencilState(g_device.GetDevice().Get(), defaultDepthDesc).value();
    defaultState.Attach(dss);
    LS::Log::TraceDebug(L"Depth stencil created!!");
    SetDepthStencilState(g_immContext.Get(), defaultState.Get(), 1);*/

    ReadOBJFile("res/cube_face1.obj");

    Vertex vd;
    std::vector<Vertex> tvd;
    LS::Vec4<float> color = { .x = 1.0f, .y = 0.4f, .z = 0.23f, .w = 1.0f };
    const auto& verts = m_objFile.GetVertices();

    for (auto i = 0u; i < verts.size(); ++i)
    {
        vd.Position.x = verts[i].x;
        vd.Position.y = verts[i].y;
        vd.Position.z = verts[i].z;
        vd.Position.w = 1.0f;

        vd.Color = color;
        tvd.push_back(vd);
    }

    const auto& faces = m_objFile.GetFaces();
    for (auto i = 0; i < faces.size(); ++i)
    {
        const auto& indices = faces[i].Indices;
        g_objIndices.insert(g_objIndices.end(), indices.begin(), indices.end());
    }

    auto objVbOpt = LS::Platform::Dx11::CreateVertexBuffer(g_device.GetDevice().Get(), tvd);
    if (!objVbOpt)
    {
        LS::Log::TraceError(L"Failed to create vertex buffer");
        return CreateFailCode("Failed to create vertex buffer");
    }

    if (const auto result = m_bufferCache.Insert("obj_vb", objVbOpt.value()); !result)
    {
        return result;
    }

    // Index Buffer //
    auto objIbOpt = LS::Platform::Dx11::CreateIndexBuffer(g_device.GetDevice().Get(), g_objIndices);
    if (!objIbOpt)
    {
        LS::Log::TraceError(L"Failed to create index buffer.");
        return CreateFailCode("Failed to create index buffer");
    }

    if (const auto result = m_bufferCache.Insert("obj_ib", objIbOpt.value()); !result)
    {
        return result;
    }

    return System::CreateSuccessCode();
}

void gt::dx11::DX11CubeApp::Run()
{
    App->Window->Show();
    g_timer.Start();
    auto viewOpt = m_bufferCache.Get("cam_view");
    auto projOpt = m_bufferCache.Get("cam_proj");
    auto mvpOpt = m_bufferCache.Get("cam_mvp");

    if (!viewOpt || !projOpt || !mvpOpt)
    {
        LS::Log::TraceError(L"Failed to obtain buffers for this operation");
        return;
    }

    const auto view = viewOpt.value();
    const auto proj = projOpt.value();
    const auto mvp = mvpOpt.value();

    while (App->Window->IsOpen())
    {
        if (App->IsPaused)
        {
            std::cout << "Paused app!\n";
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
        LS::Platform::Dx11::UpdateSubresource(g_immContext.Get(), mvp.Get(), &g_camera.Mvp);
        LS::Platform::Dx11::UpdateSubresource(g_immContext.Get(), view.Get(), &g_camera.View);
        LS::Platform::Dx11::UpdateSubresource(g_immContext.Get(), proj.Get(), &g_camera.Projection);
    }
}

void gt::dx11::DX11CubeApp::OnKeyboardDown(const LS::InputKeyDown& input)
{
    using enum LS::LS_INPUT_KEY;
    if (input.Key == ESCAPE)
    {
        App->Window->Close();
    }

    g_keysPressedMap.insert_or_assign(input.Key, true);
}

void gt::dx11::DX11CubeApp::OnKeyboardUp(const LS::InputKeyUp& input)
{
    g_keysPressedMap.insert_or_assign(input.Key, false);
}
static LS::Vec2<uint32_t> g_lastPoint;
static bool IsLMBDown = false;

void gt::dx11::DX11CubeApp::OnMouseDown(const LS::InputMouseDown& input)
{
    if (input.Button == LS::LS_INPUT_MOUSE::LMB)
    {
        std::cout << "LMB Down!\n";
        g_lastPoint.x = input.X;
        g_lastPoint.y = input.Y;
        IsLMBDown = true;
    }
}

void gt::dx11::DX11CubeApp::OnMouseUp(const LS::InputMouseUp& input)
{
    if (input.Button == LS::LS_INPUT_MOUSE::LMB)
    {
        std::cout << "LMB Up!\n";
        g_lastPoint.x = input.X;
        g_lastPoint.y = input.Y;
        IsLMBDown = false;
    }
}

void gt::dx11::DX11CubeApp::OnMouseMove(uint32_t x, uint32_t y)
{
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
        g_cameraController.RotatePitch(-my);
        g_lastPoint.x = x;
        g_lastPoint.y = y;
    }
}

static double deltaCounter = 50.0;
void gt::dx11::DX11CubeApp::OnMouseWheel(const LS::InputMouseWheelScroll& input)
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

    float value = (float)std::lerp(lowerBounds, upperBounds, deltaCounter / counterLimit);

    g_camera.FovVertical = value;
}

void gt::dx11::DX11CubeApp::OnWindowEvent(LS::LS_WINDOW_EVENT ev)
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

void gt::dx11::DX11CubeApp::PreDraw(ComPtr<ID3D11DeviceContext4>& context)
{
    const auto view = m_bufferCache.Get("cam_view").value();
    const auto proj = m_bufferCache.Get("cam_proj").value();
    const auto mvp = m_bufferCache.Get("cam_mvp").value();
    const auto vb = m_bufferCache.Get("vertex_buffer").value();
    const auto ib = m_bufferCache.Get("index_buffer").value();
    const auto obj_vb = m_bufferCache.Get("obj_vb").value();
    const auto obj_ib = m_bufferCache.Get("obj_ib").value();

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
    
    /*SetVertexBuffer(context.Get(), vb.Get(), 0, sizeof(Vertex));
    SetIndexBuffer(context.Get(), ib.Get());*/
    
    SetVertexBuffer(context.Get(), obj_vb.Get(), 0, sizeof(Vertex));
    SetIndexBuffer(context.Get(), obj_ib.Get());
    
    BindVSConstantBuffer(context.Get(), 0, view.Get());
    BindVSConstantBuffer(context.Get(), 1, proj.Get());
    BindVSConstantBuffer(context.Get(), 2, mvp.Get());
    // Draw Setup //
    ClearRT(context.Get(), rtv.Get(), g_blue);
    ClearDS(context.Get(), dsView.Get());
}

void gt::dx11::DX11CubeApp::DrawScene(ComPtr<ID3D11DeviceContext4>& context)
{
    DrawIndexed(context.Get(), (uint32_t)g_objIndices.size());
    //DrawIndexed(context.Get(), (uint32_t)g_indexData.size());
}

void gt::dx11::DX11CubeApp::HandleResize(uint32_t width, uint32_t height)
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


void gt::dx11::DX11CubeApp::ReadOBJFile(std::filesystem::path path)
{
    m_objFile.SetClockwise(true);
    auto result = m_objFile.LoadFile(path);
    if (!result)
    {
        throw std::runtime_error(result.Message().data());
    }

    auto loadResult = m_objFile.LoadObject();
    if (!loadResult)
    {
        throw std::runtime_error(loadResult.Message().data());
    }
}
