module;
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

export module DX11CubeApp;
import <array>;
import <cstdint>;
import <filesystem>;
import <optional>;
import <vector>;
import <fstream>;
import <iostream>;
import <format>;
import <compare>;
import <cmath>;
import <unordered_map>;
import <mutex>;
import <condition_variable>;
import <barrier>;
import <thread>;
import <iterator>;
import <string>;
import <ranges>;
import <functional>;
import <algorithm>;

import LSEngine;
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
        LS::Win32::RenderCommandD3D11 m_command;
        LS::Platform::Dx11::BufferCache m_bufferCache;
        uint32_t view, proj, mvp, vb, ib, obj_vb, obj_ib, vsKey, psKey;
        LS::Platform::Dx11::ResMap<Microsoft::WRL::ComPtr<ID3D11VertexShader>> m_vsMap;
        LS::Platform::Dx11::ResMap<Microsoft::WRL::ComPtr<ID3D11PixelShader>> m_psMap;

        void CompileShaders();
        auto GetBytecodes() -> std::array<std::vector<std::byte>, 2>;
        void ReadVsFile(std::filesystem::path path) noexcept;
        void ReadPsFile(std::filesystem::path path) noexcept;
    public:
        void PreDraw(ComPtr<ID3D11DeviceContext> context);
        void DrawScene(ComPtr<ID3D11DeviceContext> context);
        void HandleResize(uint32_t width, uint32_t height);
        void Update(uint64_t dt);
        void Draw();
        void OnMouseDown(const LS::Input::InputMouseDown& input);
        void OnMouseUp(const LS::Input::InputMouseUp& input);
        void OnMouseMove(uint32_t x, uint32_t y);
        void OnMouseWheel(const LS::Input::InputMouseWheelScroll& input);
        void OnWindowEvent(LS::WINDOW_EVENT ev);
        void ReadOBJFile(std::filesystem::path path);
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
    LS::Clock g_clock;
    ComPtr<ID3D11VertexShader> vertShader;
    ComPtr<ID3D11PixelShader> pixShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11InputLayout> objIL;

    std::unordered_map<LS::Input::KEYBOARD, bool> m_keyboardMap;
    LS::Serialize::WavefrontObj m_objFile;
    LS::Serialize::AssimpLoader m_assLoader;

    void SetCameraPosition(float px, float py, float pz)
    {
        g_camera.Position = XMVectorSet(px, py, pz, 1.0f);
    }

    void UpdateCubeTransform()
    {
        XMMATRIX scale = XMMatrixScalingFromVector(g_Cube.Scale);
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

        g_Cube.Indices = Geo::Generator::CreateCubeIndexArray();
    }

    void RotateCube(uint64_t elapsed)
    {
        if (elapsed == 0)
            return;
        static float accumulated = 0.0f;
        accumulated += elapsed * 0.000001f;
        float angles = 360.0f * accumulated;
        if (angles >= 360.0f)
        {
            angles = 360.0f;
        }

        float radians = (float)LS::Math::ToRadians(angles);
        XMVECTOR rotationAxis = XMVectorSet(angles * 0.5f, angles, angles * 0.25f, 1.0f);
        auto norm = XMQuaternionNormalize(rotationAxis);
        auto rotQuat = XMQuaternionRotationNormal(norm, radians);
        g_Cube.Rotation = rotQuat;
        if (accumulated >= 1.0f)
        {
            accumulated = 0.0f;
        }
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
        using enum LS::Input::KEYBOARD;
        LS::Vec3F movement;
        auto dt = g_clock.GetDeltaTimeUs();
        float movespeed = 300.0f / dt;

        if (LS::Win32::IsKeyPressed(W))
        {
            movement.z += movespeed;
        }
        if (LS::Win32::IsKeyPressed(S))
        {
            movement.z += -movespeed;
        }
        if (LS::Win32::IsKeyPressed(A))
        {
            movement.x += -movespeed;
        }
        if (LS::Win32::IsKeyPressed(D))
        {
            movement.x += movespeed;
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
    std::vector<uint32_t> g_objIndices;

}

module : private;
using namespace gt;
using namespace LS;

gt::dx11::DX11CubeApp::DX11CubeApp(uint32_t width, uint32_t height, std::wstring_view title) : LSApp(width, height, title),
m_settings(LS::CreateDeviceSettings(m_Window->GetWidth(), m_Window->GetHeight(), LS::DEVICE_API::DIRECTX_11)),
m_renderer(m_settings, m_Window.get()),
m_command()
{
}

auto gt::dx11::DX11CubeApp::Initialize(SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode
{
    LS::Colors::RGBA bgColor(1.0f, 0.0f, 0.0f, 1.0f);
    m_Window->SetBackgroundColor(bgColor);
    RegisterMouseInput(std::bind(&gt::dx11::DX11CubeApp::OnMouseDown, this, _1),
        std::bind(&gt::dx11::DX11CubeApp::OnMouseUp, this, _1),
        std::bind(&gt::dx11::DX11CubeApp::OnMouseWheel, this, _1),
        std::bind(&gt::dx11::DX11CubeApp::OnMouseMove, this, _1, _2));

    m_Window->RegisterWindowEventCallback(std::bind(&gt::dx11::DX11CubeApp::OnWindowEvent, this, _1));

    if (auto deviceResult = m_renderer.Initialize(); !deviceResult)
    {
        return deviceResult;
    }

    m_command = m_renderer.CreateImmediateCommand();
    // Init Cube and Camera //
    InitCube();
    UpdateCamera();

    // Buffer Creation //
    LS::Log::TraceDebug(L"Creating buffers...");
    uint32_t max = 0xFFFFFFFF;
    const auto vbResult = m_bufferCache.CreateVertexBuffer(g_Cube.Verts.data(), sizeof(Vertex) * g_Cube.Verts.size(), m_renderer.GetDevice());
    vb = vbResult.value_or(max);

    const auto ibResult = m_bufferCache.CreateIndexBuffer(g_Cube.Indices, m_renderer.GetDevice());
    ib = ibResult.value_or(max);

    // Camera Buffers //
    const auto viewResult = m_bufferCache.CreateConstantBuffer(&(g_camera.View), m_renderer.GetDevice());
    view = viewResult.value_or(max);

    const auto projResult = m_bufferCache.CreateConstantBuffer(&(g_camera.Projection), m_renderer.GetDevice());
    proj = projResult.value_or(max);

    const auto mvpResult = m_bufferCache.CreateConstantBuffer(&(g_camera.Mvp), m_renderer.GetDevice());
    mvp = mvpResult.value_or(max);

    LS::Log::TraceDebug(L"Buffers created!!");

    m_command.SetDepthBufferMode(LS::Win32::DEPTH_BUFFER_MODE::DEFAULT);

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

    const auto objVbResult = m_bufferCache.CreateVertexBuffer<Vertex>(tvd, m_renderer.GetDevice());
    obj_vb = objVbResult.value_or(max);

    // Index Buffer //
    const auto objIbResult = m_bufferCache.CreateIndexBuffer(g_objIndices, m_renderer.GetDevice());
    obj_ib = objIbResult.value_or(max);

    if (vb == max || ib == max || view == max
        || proj == max || mvp == max || obj_vb == max || obj_ib == max)
        return System::CreateFailCode("Error when creating buffer");

    CompileShaders();

    return System::CreateSuccessCode();
}

void gt::dx11::DX11CubeApp::Run()
{
    m_State = LS::APP_STATE::RUNNING;
    m_Window->Show();
    g_clock.Start();

    while (m_Window->IsOpen())
    {
        if (LS::Win32::IsKeyPressed(LS::Input::KEYBOARD::ESCAPE))
        {
            m_Window->Close();
            break;
        }

        if (m_State == LS::APP_STATE::PAUSED)
        {
            std::cout << "Paused app!\n";
            continue;
        }

        g_clock.Tick();
        uint64_t dt = g_clock.GetDeltaTimeUs();
        m_Window->PollEvent();
        Update(dt);
        Draw();
    }
}

static LS::Vec2<uint32_t> g_lastPoint;
static bool IsLMBDown = false;

void gt::dx11::DX11CubeApp::OnMouseDown(const LS::Input::InputMouseDown& input)
{
    if (input.Button == LS::Input::MOUSE_BUTTON::LMB)
    {
        std::cout << "LMB Down!\n";
        g_lastPoint.x = input.X;
        g_lastPoint.y = input.Y;
        IsLMBDown = true;
    }
}

void gt::dx11::DX11CubeApp::OnMouseUp(const LS::Input::InputMouseUp& input)
{
    if (input.Button == LS::Input::MOUSE_BUTTON::LMB)
    {
        std::cout << "LMB Up!\n";
        g_lastPoint.x = input.X;
        g_lastPoint.y = input.Y;
        IsLMBDown = false;
    }
}

void gt::dx11::DX11CubeApp::OnMouseMove(uint32_t x, uint32_t y)
{
    //if (IsLMBDown)
    if (LS::Win32::IsMousePress(LS::Input::MOUSE_BUTTON::LMB))
    {
        int lx = x - g_lastPoint.x;
        int ly = y - g_lastPoint.y;
        auto dt = g_clock.GetDeltaTimeUs();
        float mx = lx * 0.03f;
        float my = ly * 0.02f;
        // Normalize between screen size //
        g_cameraController.RotateYaw(mx * (dt * 0.001f));
        g_cameraController.RotatePitch(-my * (dt * 0.001f));
        g_lastPoint.x = x;
        g_lastPoint.y = y;
    }
}

static double deltaCounter = 50.0;
void gt::dx11::DX11CubeApp::OnMouseWheel(const LS::Input::InputMouseWheelScroll& input)
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

void gt::dx11::DX11CubeApp::OnWindowEvent(LS::WINDOW_EVENT ev)
{
    using enum LS::WINDOW_EVENT;
    switch (ev)
    {
    case CLOSE_WINDOW:
    {
        m_State = LS::APP_STATE::CLOSED;
    }
    break;
    case WINDOW_RESIZE_START:
    {
        m_State = LS::APP_STATE::PAUSED;
        break;
    }
    case WINDOW_RESIZE_END:
    {
        const auto width = m_Window->GetWidth();
        const auto height = m_Window->GetHeight();
        HandleResize(width, height);
    }
    break;
    case MAXIMIZED_WINDOW:
    {
        const auto width = m_Window->GetWidth();
        const auto height = m_Window->GetHeight();
        HandleResize(width, height);
    }
    break;
    }
}

void gt::dx11::DX11CubeApp::PreDraw(ComPtr<ID3D11DeviceContext> context)
{
    const auto viewBuff = m_bufferCache.Get(view).value();
    const auto projBuff = m_bufferCache.Get(proj).value();
    const auto mvpBuff = m_bufferCache.Get(mvp).value();
    const auto vbBuff = m_bufferCache.Get(vb).value();
    const auto ibBuff = m_bufferCache.Get(ib).value();
    const auto obj_vbBuff = m_bufferCache.Get(obj_vb).value();
    const auto obj_ibBuff = m_bufferCache.Get(obj_ib).value();

    // Command Usage //
    m_command.SetCullMethod(LS::Win32::CULL_METHOD::CULL_BACKFACE_CC);
    m_command.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_command.SetInputLayout(inputLayout.Get());
    m_command.SetViewport(static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));
    m_command.BindVS(vertShader.Get());
    m_command.BindPS(pixShader.Get());
    m_command.SetVertexBuffer(vbBuff.Get(), sizeof(Vertex));
    m_command.SetIndexBuffer(ibBuff.Get());
    std::array<ID3D11Buffer*, 3> buffers{ viewBuff.Get(), projBuff.Get(), mvpBuff.Get() };
    m_command.BindVSConstantBuffers(buffers);

    m_renderer.Clear(g_blue, LS::Win32::DEPTH_STENCIL_MODE::DEFAULT);

    // Set States and Objects //
    //SetRenderTarget(context.Get(), rtv.Get(), dsView.Get());
    //SetRasterizerState(context.Get(), rsSolid.Get());
    //SetTopology(context.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //SetInputlayout(context.Get(), inputLayout.Get());
    //SetViewport(context.Get(), static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));
    // Bind to State //
    /*BindVS(context.Get(), vertShader.Get());
    BindPS(context.Get(), pixShader.Get());*/
    // Cube //
    /*SetVertexBuffer(context.Get(), vb.Get(), 0, sizeof(Vertex));
    SetIndexBuffer(context.Get(), ib.Get());*/
    // Blender Obj //
    /*SetVertexBuffer(context.Get(), obj_vb.Get(), 0, sizeof(Vertex));
    SetIndexBuffer(context.Get(), obj_ib.Get());*/
    /*std::vector<ID3D11Buffer*> buffers{ view.Get(), proj.Get(), mvp.Get() };
    BindVSConstantBuffers(context.Get(), 0, buffers);*/
    // Draw Setup //
    /*ClearRT(context.Get(), rtv.Get(), g_blue);
    ClearDS(context.Get(), dsView.Get());*/
}

void gt::dx11::DX11CubeApp::DrawScene(ComPtr<ID3D11DeviceContext> context)
{
    m_command.DrawIndexed((uint32_t)g_Cube.Indices.size());
}

void gt::dx11::DX11CubeApp::HandleResize(uint32_t width, uint32_t height)
{
    m_State = LS::APP_STATE::PAUSED;

    ClearDeviceDependentResources(m_renderer.GetDeviceContextCom().Get());
    m_renderer.Resize(width, height);
    m_State = LS::APP_STATE::RUNNING;
}

void gt::dx11::DX11CubeApp::Update(uint64_t dt)
{
    UpdateMovement();
    RotateCube(dt);
    UpdateCubeTransform();
    UpdateCamera();
}

void gt::dx11::DX11CubeApp::Draw()
{
    auto viewOpt = m_bufferCache.Get(view);
    auto projOpt = m_bufferCache.Get(proj);
    auto mvpOpt = m_bufferCache.Get(mvp);
    if (!viewOpt || !projOpt || !mvpOpt)
    {
        LS::Log::TraceError(L"Failed to obtain buffers for this operation");
        return;
    }

    const auto viewBuff = viewOpt.value();
    const auto projBuff = projOpt.value();
    const auto mvpBuff = mvpOpt.value();

    // Update Buffers //
    m_command.UpdateConstantBuffer(mvpBuff.Get(), &g_camera.Mvp);
    m_command.UpdateConstantBuffer(viewBuff.Get(), &g_camera.View);
    m_command.UpdateConstantBuffer(projBuff.Get(), &g_camera.Projection);

    PreDraw(m_renderer.GetDeviceContextCom());
    DrawScene(m_renderer.GetDeviceContextCom());
    Present1(m_renderer.GetSwapChainCom().Get(), 1);
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

auto gt::dx11::DX11CubeApp::GetBytecodes()->std::array<std::vector<std::byte>, 2>
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

void gt::dx11::DX11CubeApp::ReadVsFile(std::filesystem::path path) noexcept
{
    LS::Nullable<std::vector<std::byte>> result = LS::IO::ReadFile(path);
    if (!result)
    {
        LS::Log::TraceError(L"Failed to create shader: " + path.wstring());
        return;
    }

    const auto shader = m_renderer.CreateVertexShader(result.value());
    //vsKey = m_vsMap.Add(shader);
}

void gt::dx11::DX11CubeApp::ReadPsFile(std::filesystem::path path) noexcept
{
    LS::Nullable<std::vector<std::byte>> result = LS::IO::ReadFile(path);
    if (!result)
    {
        LS::Log::TraceError(L"Failed to create shader: " + path.wstring());
        return;
    }

    const auto shader = m_renderer.CreatePixelShader(result.value());
    //psKey = m_psMap.Add(shader);
}
