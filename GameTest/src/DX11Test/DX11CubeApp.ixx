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
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


export module DX11CubeApp;

import Engine.App;
import D3D11Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Engine.Logger;
import Engine.LSDevice;
import Helper.IO;
import GeometryGenerator;
import MathLib;
import DirectXCommon;
import LSEDataLib;
import LSE.Serialize.WavefrontObj;
import LSE.Serialize.AssimpLoader;
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
    export class DX11CubeApp : public LS::LSApp
    {
    public:
        DX11CubeApp() = default;
        DX11CubeApp(uint32_t width, uint32_t height, std::wstring_view title);
        ~DX11CubeApp() = default;

        auto Initialize([[maybe_unused]] SharedRef<LS::LSCommandArgs> args) -> System::ErrorCode override;
        void Run() override;

    private:
        LS::LSDeviceSettings m_settings;
        LS::Win32::RenderD3D11 m_renderer;
        LS::Platform::Dx11::BufferCache m_bufferCache;
        LS::PipelineDescriptor m_cubePipeline;
        
        void CompileShaders();
        auto GetBytecodes() -> std::array<std::vector<std::byte>, 2>;

    public:
        void PreDraw(ComPtr<ID3D11DeviceContext> context);
        void DrawScene(ComPtr<ID3D11DeviceContext> context);
        void HandleResize(uint32_t width, uint32_t height);

        void OnKeyboardDown(const LS::InputKeyDown& input);
        void OnKeyboardUp(const LS::InputKeyUp& input);
        void OnMouseDown(const LS::InputMouseDown& input);
        void OnMouseUp(const LS::InputMouseUp& input);
        void OnMouseMove(uint32_t x, uint32_t y);
        void OnMouseWheel(const LS::InputMouseWheelScroll& input);
        void OnWindowEvent(LS::LS_WINDOW_EVENT ev);
        void ReadOBJFile(std::filesystem::path path);
        void SetupPipeline();
    };

    using namespace std::placeholders;

    constexpr auto SCREEN_WIDTH = 1920u;
    constexpr auto SCREEN_HEIGHT = 1080u;

#ifdef DEBUG
    const std::string shader_path = R"(build\x64\Debug\)";
#else
    const std::string shader_path = R"(build\x64\Release\)";
#endif

    Cube g_Cube;
    XMVECTOR g_UpVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR g_LookAtDefault = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    LS::DX::DXCamera g_camera(SCREEN_WIDTH, SCREEN_HEIGHT, XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f), g_LookAtDefault, g_UpVec, 90.0f);
    LS::DX::FreeFlyCameraControllerDX g_cameraController(g_camera);
    constexpr auto g_indexData = Geo::Generator::CreateCubeIndexArray();
    LS::LSTimer<std::uint64_t, 1ul, 1000ul> g_timer;
    ComPtr<ID3D11VertexShader> vertShader;
    ComPtr<ID3D11PixelShader> pixShader;
    ComPtr<ID3D11RasterizerState> rsSolid;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11DepthStencilView> dsView;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11InputLayout> objIL;

    std::unordered_map<LS::LS_INPUT_KEY, bool> g_keysPressedMap;
    LS::Serialize::WavefrontObj m_objFile;
    LS::Serialize::AssimpLoader m_assLoader;
    
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

gt::dx11::DX11CubeApp::DX11CubeApp(uint32_t width, uint32_t height, std::wstring_view title) : LSApp(width, height, title),
    m_settings(LS::CreateDeviceSettings(Window->GetWidth(), Window->GetHeight(), LS::DEVICE_API::DIRECTX_11)),
    m_renderer(m_settings, Window.get())
{
}

auto gt::dx11::DX11CubeApp::Initialize(SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode
{
    using enum LS::System::ErrorStatus;
    LS::ColorRGBA bgColor(1.0f, 0.0f, 0.0f, 1.0f);
    Window->SetBackgroundColor(bgColor);

    Window->RegisterKeyboardDown(std::bind(&gt::dx11::DX11CubeApp::OnKeyboardDown, this, _1));
    Window->RegisterKeyboardUp(std::bind(&gt::dx11::DX11CubeApp::OnKeyboardUp, this, _1));
    RegisterMouseInput(std::bind(&gt::dx11::DX11CubeApp::OnMouseDown, this, _1),
        std::bind(&gt::dx11::DX11CubeApp::OnMouseUp, this, _1), 
        std::bind(&gt::dx11::DX11CubeApp::OnMouseWheel, this, _1), 
        std::bind(&gt::dx11::DX11CubeApp::OnMouseMove, this, _1, _2));

    Window->RegisterWindowEventCallback(std::bind(&gt::dx11::DX11CubeApp::OnWindowEvent, this, _1));
    
    if (auto deviceResult = m_renderer.Initialize(); !deviceResult)
    {
        return deviceResult;
    }

    // Init Cube and Camera //
    InitCube();
    UpdateCamera();

    // Buffer Creation //
    LS::Log::TraceDebug(L"Creating buffers...");

    const auto vbOpt = LS::Platform::Dx11::CreateVertexBuffer(m_renderer.GetDevice(), g_Cube.Verts);
    if (!vbOpt)
    {
        return CreateFailCode("Failed to create vertex buffer");
    }

    if (const auto result = m_bufferCache.Insert("vertex_buffer", vbOpt.value()); !result)
    {
        return result;
    }
    
    const auto ibOpt = LS::Platform::Dx11::CreateIndexBuffer(m_renderer.GetDevice(), g_indexData);
    if (!ibOpt)
    {
        return CreateFailCode("Failed to create index buffer");
    }

    if (const auto result = m_bufferCache.Insert("index_buffer", ibOpt.value()); !result)
    {
        return result;
    }

    // Camera Buffers //
    auto bufferOptional = LS::Platform::Dx11::CreateConstantBuffer(m_renderer.GetDevice(), g_camera.View);
    if (!bufferOptional)
        return CreateFailCode("Failed to create camera view matrix buffer");

    if (const auto result = m_bufferCache.Insert("cam_view", bufferOptional.value()); !result)
    {
        return result;
    }

    bufferOptional = LS::Platform::Dx11::CreateConstantBuffer(m_renderer.GetDevice(), g_camera.Projection);
    if (!bufferOptional)
        return CreateFailCode("Failed to create camera projection matrix buffer");
    
    if (const auto result = m_bufferCache.Insert("cam_proj", bufferOptional.value()); !result)
    {
        return result;
    }

    bufferOptional = LS::Platform::Dx11::CreateConstantBuffer(m_renderer.GetDevice(), g_camera.Mvp);
    if (!bufferOptional)
        return CreateFailCode("Failed to create camera MVP matrix buffer");
    
    if (const auto result = m_bufferCache.Insert("cam_mvp", bufferOptional.value()); !result)
    {
        return result;
    }

    LS::Log::TraceDebug(L"Buffers created!!");

    // Rasterizer Creation // 
    LS::Log::TraceDebug(L"Building rasterizer state...");
    auto rsSolidOpt = CreateRasterizerState(m_renderer.GetDeviceCom(), SolidFill_BackCull_FCW_DCE);
    if (!rsSolidOpt)
    {
        return CreateFailCode("Failed to create rasterizer state");
    }

    rsSolid = rsSolidOpt.value();
    LS::Log::TraceDebug(L"Rasterizer state completed!!");
    // Render Target Creation //
    LS::Log::TraceDebug(L"Building render target view....");
    auto rtvOpt = LS::Win32::CreateRenderTargetViewFromSwapChain(m_renderer.GetDeviceCom(), m_renderer.GetSwapChainCom());
    if (!rtvOpt)
    {
        return CreateFailCode("Failed to create render target from back buffer");
    }
    rtv = rtvOpt.value();
    LS::Log::TraceDebug(L"Render target view created!!");

    // Depth Stencil //
    LS::Log::TraceDebug(L"Building depth stencil....");

    auto dsResult = LS::Win32::CreateDepthStencilViewFromSwapChain(m_renderer.GetDeviceCom(), m_renderer.GetSwapChainCom());
    if (!dsResult)
    {
        return CreateFailCode("Failed to create depth stencil");
    }
    dsView = dsResult.value();
    
    CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    ComPtr<ID3D11DepthStencilState> defaultState;
    auto dss = CreateDepthStencilState(m_renderer.GetDevice(), defaultDepthDesc).value();
    LS::Log::TraceDebug(L"Depth stencil created!!");
    SetDepthStencilState(m_renderer.GetDeviceContextCom().Get(), dss.Get(), 1);

    ReadOBJFile("res/cube_face1.obj");

    Vertex vd;
    std::vector<Vertex> tvd;

    const auto& meshes = m_assLoader.GetMeshes();
    std::cout << meshes.size() << " Meshes\n";

    const auto& mesh = meshes[0];
    LS::Vec4<float> color = { .x = 1.0f, .y = 0.4f, .z = 0.23f, .w = 1.0f };
    for (auto i = 0u; i < mesh.Vertices.size(); ++i)
    {
        vd.Position.x = mesh.Vertices[i].x;
        vd.Position.y = mesh.Vertices[i].y;
        vd.Position.z = mesh.Vertices[i].z;
        vd.Position.w = 1.0f;
        vd.Color = color;
        tvd.push_back(vd);
    }
    g_objIndices.clear();
    g_objIndices.insert(g_objIndices.begin(), mesh.Indices.begin(), mesh.Indices.end());
    
    auto objVbOpt = LS::Platform::Dx11::CreateVertexBuffer(m_renderer.GetDevice(), tvd);
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
    auto objIbOpt = LS::Platform::Dx11::CreateIndexBuffer(m_renderer.GetDevice(), mesh.Indices);
    if (!objIbOpt)
    {
        LS::Log::TraceError(L"Failed to create index buffer.");
        return CreateFailCode("Failed to create index buffer");
    }

    if (const auto result = m_bufferCache.Insert("obj_ib", objIbOpt.value()); !result)
    {
        return result;
    }

    CompileShaders();

    return System::CreateSuccessCode();
}

void gt::dx11::DX11CubeApp::Run()
{
    IsRunning = true;
    Window->Show();
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

    while (Window->IsOpen())
    {
        if (IsPaused)
        {
            std::cout << "Paused app!\n";
            //std::this_thread::sleep_for(100ms);
            continue;
        }
        auto elapsed = g_timer.GetTotalTimeTicked();
        auto deltaTime = g_timer.GetDeltaTime();

        Window->PollEvent();
        g_timer.Tick();

        UpdateMovement();
        RotateCube(deltaTime.count());
        UpdateCubeTransform();
        UpdateCamera();

        PreDraw(m_renderer.GetDeviceContextCom());
        DrawScene(m_renderer.GetDeviceContextCom());
        Present1(m_renderer.GetSwapChainCom().Get(), 1);

        // Update Buffers //
        LS::Platform::Dx11::UpdateSubresource(m_renderer.GetDeviceContextCom().Get(), mvp.Get(), &g_camera.Mvp);
        LS::Platform::Dx11::UpdateSubresource(m_renderer.GetDeviceContextCom().Get(), view.Get(), &g_camera.View);
        LS::Platform::Dx11::UpdateSubresource(m_renderer.GetDeviceContextCom().Get(), proj.Get(), &g_camera.Projection);
    }
}

void gt::dx11::DX11CubeApp::OnKeyboardDown(const LS::InputKeyDown& input)
{
    using enum LS::LS_INPUT_KEY;
    if (input.Key == ESCAPE)
    {
        Window->Close();
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
        IsRunning = false;
        IsPaused = true;
    }
    break;
    case WINDOW_RESIZE_START:
    {
        IsPaused = true;
        break;
    }
    case WINDOW_RESIZE_END:
    {
        const auto width = Window->GetWidth();
        const auto height = Window->GetHeight();
        HandleResize(width, height);
    }
    break;
    case MAXIMIZED_WINDOW:
    {
        const auto width = Window->GetWidth();
        const auto height = Window->GetHeight();
        HandleResize(width, height);
    }
    break;
    }
}

void gt::dx11::DX11CubeApp::PreDraw(ComPtr<ID3D11DeviceContext> context)
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
    SetViewport(context.Get(), static_cast<float>(Window->GetWidth()), static_cast<float>(Window->GetHeight()));

    // Bind to State //
    BindVS(context.Get(), vertShader.Get());
    BindPS(context.Get(), pixShader.Get());
    
    SetVertexBuffer(context.Get(), vb.Get(), 0, sizeof(Vertex));
    SetIndexBuffer(context.Get(), ib.Get());
    
    //SetVertexBuffer(context.Get(), obj_vb.Get(), 0, sizeof(Vertex));
    //SetIndexBuffer(context.Get(), obj_ib.Get());
    
    BindVSConstantBuffer(context.Get(), 0, view.Get());
    BindVSConstantBuffer(context.Get(), 1, proj.Get());
    BindVSConstantBuffer(context.Get(), 2, mvp.Get());
    // Draw Setup //
    ClearRT(context.Get(), rtv.Get(), g_blue);
    ClearDS(context.Get(), dsView.Get());
}

void gt::dx11::DX11CubeApp::DrawScene(ComPtr<ID3D11DeviceContext> context)
{
    //DrawIndexed(context.Get(), (uint32_t)g_objIndices.size());
    DrawIndexed(context.Get(), (uint32_t)g_indexData.size());
}

void gt::dx11::DX11CubeApp::HandleResize(uint32_t width, uint32_t height)
{
    IsPaused = true;
    ClearDeviceDependentResources(m_renderer.GetDeviceContextCom().Get());
    if (rtv)
    {
        rtv = nullptr;
    }
    if (dsView)
    {
        dsView = nullptr;
    }

    m_renderer.Resize(width, height);
    
    auto rtvOpt = LS::Win32::CreateRenderTargetViewFromSwapChain(m_renderer.GetDeviceCom(), m_renderer.GetSwapChainCom());
    if (!rtvOpt)
    {
        LS::Log::TraceError(L"Failed to create render target view from back buffer");
        IsPaused = false;
        return;
    }
    rtv = rtvOpt.value();

    auto dsResult = LS::Win32::CreateDepthStencilViewFromSwapChain(m_renderer.GetDeviceCom(), m_renderer.GetSwapChainCom());
    if (!dsResult)
    {
        LS::Log::TraceError(L"Failed to create depth stencil view from back buffer");
        IsPaused = false;
        return;
    }
    dsView = dsResult.value();
    
    IsPaused = false;
}


void gt::dx11::DX11CubeApp::ReadOBJFile(std::filesystem::path path)
{
    auto result = m_assLoader.Load(path,
        (uint32_t)aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_MakeLeftHanded);
    if (!result)
    {
        std::cout << result.Message();
    }
}

void gt::dx11::DX11CubeApp::SetupPipeline()
{
    LS::Win32::InitCommonStates(m_renderer.GetDeviceCom().Get());

    LS::PipelineDescriptor desc;
    desc.RasterizeState = LS::SolidFill_BackCull_FCCW_DCE;
    desc.BlendState.BlendType = LS::BlendType::OPAQUE_BLEND;
    desc.Topology = LS::PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP;
    desc.DepthStencil = LS::DefaultDepth;
    
    // Vertex and Pixel Shaders
    const auto bc = GetBytecodes();
    desc.Shaders[SHADER_TYPE::VERTEX] = bc[0];
    desc.Shaders[SHADER_TYPE::PIXEL] = bc[1];
    
    //desc.RenderTarget.
    

    auto factory = LS::Win32::D3D11PipelineFactory();
}

void gt::dx11::DX11CubeApp::CompileShaders()
{
    LS::Log::TraceDebug(L"Compiling shaders....");
    // Shader compilation //

    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size())))
    {
        return;
    }

    std::wstring path = std::wstring(modulePath.data());
    auto lastOf = path.find_last_of(L"\\");
    path.erase(lastOf);
    std::wstring vsPath = std::format(L"{}\\VertexShader.cso", path);
    std::wstring psPath = std::format(L"{}\\PixelShader.cso", path);

    auto vsFile = LS::IO::ReadFile(vsPath);
    if (!vsFile)
    {
        return;
    }
    std::vector<std::byte> vsData = vsFile.value();

    auto psFile = LS::IO::ReadFile(psPath);
    if (!psFile)
    {
        return;
    }
    std::vector<std::byte> psData = psFile.value();

    vertShader = m_renderer.CreateVertexShader(vsData);
    if (!vertShader)
    {
        LS::Log::TraceError(L"Failed to create vertex shader with renderer");
        return;
    }

    pixShader = m_renderer.CreatePixelShader(psData);
    if (!pixShader)
    {
        LS::Log::TraceError(L"Failed to create pixel shader");
        return;
    }

    LS::Log::TraceDebug(L"Shader Compilation Complete!!");

    // Input Layout // 
    auto il = m_renderer.BuildInputLayout(vsData);
    if (!il)
    {
        LS::Log::TraceError(L"Failed to create input layout");
        return;
    }

    inputLayout = il.value();
}

auto gt::dx11::DX11CubeApp::GetBytecodes() -> std::array<std::vector<std::byte>, 2>
{
    std::array<std::vector<std::byte>, 2> out;
    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size())))
    {
        return out;
    }

    std::wstring path = std::wstring(modulePath.data());
    auto lastOf = path.find_last_of(L"\\");
    path.erase(lastOf);
    std::wstring vsPath = std::format(L"{}\\VertexShader.cso", path);
    std::wstring psPath = std::format(L"{}\\PixelShader.cso", path);

    Nullable<std::vector<std::byte>> vsFile = LS::IO::ReadFile(vsPath);
    if (!vsFile)
    {
        return out;
    }
    std::vector<std::byte> vsData = vsFile.value();

    Nullable<std::vector<std::byte>> psFile = LS::IO::ReadFile(psPath);
    if (!psFile)
    {
        return out;
    }
    std::vector<std::byte> psData = psFile.value();

    out[0] = vsData;
    out[1] = psData;
    return out;
}