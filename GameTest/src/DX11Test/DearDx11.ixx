module;
#include "vendor\imgui\imgui.h"
#include "vendor\imgui\backends\imgui_impl_win32.h"
#include "vendor\imgui\backends\imgui_impl_dx11.h"
#include <wrl\client.h>
#include <directxmath/DirectXMath.h>
#include <directxmath/DirectXColors.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

export module DearDx11;

import LSEngine;
import Helper.IO;
import DX11Systems;

import <cstdint>;
import <string_view>;
import <array>;
import <string>;
import <chrono>;

#ifdef DEBUG
const std::string shader_path = R"(build\x64\Debug\)";
#else
const std::string shader_path = R"(build\x64\Release\)";
#endif

namespace gt::dx11
{
    LRESULT WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

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

constexpr size_t CELL_DEPTH = 20;
constexpr size_t CELL_WIDTH = 20;
std::array<LS::Vec3<float>, CELL_DEPTH * CELL_WIDTH> g_floor = LS::Geo::Generator::CreateFloor<CELL_DEPTH, CELL_WIDTH>();;
XMMATRIX g_floorTransform;

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
        uint32_t viewKey, projKey, mvpKey, vbKey, ibKey, floorKey;
        XMVECTOR m_UpVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR m_LookAtDefault = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        std::array<float, 4> m_clearColor{ 1.0f, 0.43f, 0.02f, 1.0f };
        std::array<float, 4> m_rotArr{ 0.0f, 0.0f, 0.0f, 1.0f };
        std::array<float, 4> m_scaleArr{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::array<float, 4> m_posArr{ 0.0f, 0.0f, 0.0f, 1.0f };
        LS::LSDeviceSettings m_settings;
        LS::Win32::RenderD3D11 m_renderer;
        LS::Win32::RenderCommandD3D11 m_command;
        LS::DX::DXCamera m_camera;
        LS::DX::FreeFlyCameraControllerDX m_cameraController;
        LS::Clock m_clock;
        Cube m_cube;
        LS::Platform::Dx11::BufferCache m_bufferCache;
        ComPtr<ID3D11VertexShader> m_vertShader;
        ComPtr<ID3D11PixelShader> m_pixShader;
        ComPtr<ID3D11VertexShader> m_terrainShaderVs;
        ComPtr<ID3D11InputLayout> m_inputLayout;
        ComPtr<ID3D11InputLayout> m_terrainIL;

        int DrawMethodSelection = 0;

        void CompileShaders();
        void CreateBuffers();

        void Update();
        void UpdateCamera();
        void UpdateCube();
        void UpdateFloor();
        void ReadKeyboard();
        void Draw();
    };
}

module : private;

using namespace gt::dx11;

std::array<float, 4> g_red = { 1.0f, 0.0f, 0.0f, 1.0f };
std::array<float, 4> g_green = { 0.0f, 1.0f, 0.0f, 1.0f };
std::array<float, 4> g_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
std::array<float, 4> g_black = { 0.0f, 0.0f, 0.0f, 1.0f };
std::array<float, 4> g_white = { 1.0f, 1.0f, 1.0f, 1.0f };

ImGuiDx11::ImGuiDx11(uint32_t width, uint32_t height, std::wstring_view title) : LSApp(width, height, title),
m_settings(LS::CreateDeviceSettings(m_Window->GetWidth(), m_Window->GetHeight(), LS::DEVICE_API::DIRECTX_11)),
m_renderer(m_settings, m_Window.get()),
m_camera(width, height, XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f), m_LookAtDefault, m_UpVec, 90.0f),
m_cameraController(m_camera)
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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Seems to make the viewports (dockable windows) extend outside bounds of window

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
    m_renderer.SetViewport(m_Window->GetWidth(), m_Window->GetHeight());
    ImGui_ImplDX11_Init(m_renderer.GetDevice(), context);

    m_command = m_renderer.CreateImmediateCommand();
    CompileShaders();
    CreateBuffers();
    return LS::System::CreateSuccessCode();
}

void gt::dx11::ImGuiDx11::Run()
{
    m_Window->Show();
    ImGuiIO& io = ImGui::GetIO();

    auto currWidth = m_Window->GetWidth();
    auto currHeight = m_Window->GetHeight();
    m_clock.Start();

    const char* items[]{ "solid", "wireframe" };
    while (m_Window->IsOpen())
    {
        m_Window->PollEvent();
        m_clock.Tick();
        auto mousePos = m_Window->GetMousePos();
        if (currWidth != m_Window->GetWidth() && currHeight != m_Window->GetHeight())
        {
            m_renderer.Resize(m_Window->GetWidth(), m_Window->GetHeight());
            m_renderer.SetViewport(m_Window->GetWidth(), m_Window->GetHeight());
            currWidth = m_Window->GetWidth();
            currHeight = m_Window->GetHeight();
        }
        Update();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello Gui");
        ImGui::Text("Width: %d", currWidth);
        ImGui::Text("Height: %d", currHeight);
        ImGui::Text("Mouse Pos: %d x %d", mousePos.x, mousePos.y);
        ImGui::SliderFloat3("Color", m_clearColor.data(), 0.0f, 1.0f);
        ImGui::SliderFloat3("Cube Pos", m_posArr.data(), 0.0f, 1.0f);
        ImGui::SliderFloat3("Cube Rot", m_rotArr.data(), 0.0f, 360.0f);
        ImGui::SliderFloat3("Cube Scale", m_scaleArr.data(), 1.0f, 2.0f);
        ImGui::Text("Time: %d", m_clock.GetTotalTimeSecs());
        ImGui::Text("Delta Time: %.6f ms", m_clock.GetDeltaTimeUs() / 100'000.0f);
        const auto cpos = m_camera.PositionF3();
        ImGui::Text("Camera Pos: (%.4f, %.4f %.4f)", cpos.x, cpos.y, cpos.z);
        ImGui::BeginListBox("Draw Mode", ImVec2(300.0f, 50.0f));
        ImGui::ListBox("Foo", &DrawMethodSelection, items, 2);
        ImGui::EndListBox();
        ImGui::End();

        m_renderer.Clear(m_clearColor);
        Draw();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Uncomment for use if the flag ViewportsEnable is set in Initialize()
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable && m_Window->IsOpen())
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

void gt::dx11::ImGuiDx11::CompileShaders()
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
    std::wstring terrainPath = std::format(L"{}\\TerrainVS.cso", path);

    auto readFile = [](std::filesystem::path path) -> std::vector<std::byte>
        {
            if (auto result = LS::IO::ReadFile(path); result)
            {
                return result.value();
            }
            return std::vector<std::byte>{};
        };

    std::vector<std::byte> vsData = readFile(vsPath);
    std::vector<std::byte> psData = readFile(psPath);
    std::vector<std::byte> terrainData = readFile(terrainPath);
    assert(vsData.size() > 0 && "Data was empty, cannot continue");
    assert(psData.size() > 0 && "Data was empty, cannot continue");
    assert(terrainData.size() > 0 && "Data was empty, cannot continue");

    m_vertShader = m_renderer.CreateVertexShader(vsData);
    if (!m_vertShader)
    {
        LS::Log::TraceError(L"Failed to create vertex shader with renderer");
        return;
    }

    m_pixShader = m_renderer.CreatePixelShader(psData);
    if (!m_pixShader)
    {
        LS::Log::TraceError(L"Failed to create pixel shader");
        return;
    }

    m_terrainShaderVs = m_renderer.CreateVertexShader(terrainData);
    if (!m_terrainShaderVs)
    {
        LS::Log::TraceError(L"Failed to create terrain shader");
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

    m_inputLayout = il.value();

    auto til = m_renderer.BuildInputLayout(terrainData);
    if (!til)
    {
        LS::Log::TraceError(L"Failed to create input layout");
        return;
    }

    m_terrainIL = til.value();

}

void gt::dx11::ImGuiDx11::CreateBuffers()
{
    m_cube.Position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    m_cube.Scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
    m_cube.Rotation = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    UpdateCube();

    auto [verts, indices] = LS::Geo::Generator::CreateCubeVertsAndIndices(1.0f);

    for (auto i = 0u; i < verts.size(); ++i)
    {
        LS::Vec3<float> v = verts[i];
        m_cube.Verts[i].Position = LS::Vec4<float>{ v.x, v.y, v.z, 1.0f };
        m_cube.Verts[i].Color = LS::Vec4<float>{ 1.0f, 0.33f, 0.234f, 1.0f };
    }

    m_cube.Indices = Geo::Generator::CreateCubeIndexArray();

    LS::Log::TraceDebug(L"Creating buffers for Cube (DearDx11)");
    uint32_t max = 0xFFFFFFFF;
    const auto vbResult = m_bufferCache.CreateVertexBuffer(m_cube.Verts.data(), m_cube.Verts.size() * sizeof(m_cube.Verts[0]), m_renderer.GetDevice());
    vbKey = vbResult.value_or(max);
    //TODO: Need to implement better solution to obtaining object data (pointer to) and size
    const auto floorResult = m_bufferCache.CreateVertexBuffer(g_floor.data(), g_floor.size() * sizeof(g_floor[0]), m_renderer.GetDevice());
    floorKey = floorResult.value_or(max);

    const auto ibResult = m_bufferCache.CreateIndexBuffer(m_cube.Indices, m_renderer.GetDevice());
    ibKey = ibResult.value_or(max);

    // Camera Buffers //
    const auto viewResult = m_bufferCache.CreateConstantBuffer(&(m_camera.View), sizeof(m_camera.View), m_renderer.GetDevice());
    viewKey = viewResult.value_or(max);

    const auto projResult = m_bufferCache.CreateConstantBuffer(&(m_camera.Projection), sizeof(m_camera.Projection), m_renderer.GetDevice());
    projKey = projResult.value_or(max);

    const auto mvpResult = m_bufferCache.CreateConstantBuffer(&(m_camera.Mvp), sizeof(m_camera.Mvp), m_renderer.GetDevice());
    mvpKey = mvpResult.value_or(max);

    LS::Log::TraceDebug(L"Buffers created for Cube (DearDx11)!!");
}

void gt::dx11::ImGuiDx11::Update()
{
    UpdateCube();
    UpdateFloor();
    UpdateCamera();
}

void gt::dx11::ImGuiDx11::UpdateCamera()
{
    using enum LS::Input::KEYBOARD;
    LS::Vec3F movement;
    auto dt = m_clock.GetDeltaTimeUs() / 100'000.0f;
    float movespeed = 1.0f * dt;

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
    if (LS::Win32::IsKeyPressed(Q))
    {
        movement.y += -movespeed;
    }
    if (LS::Win32::IsKeyPressed(E))
    {
        movement.y += movespeed;
    }

    m_cameraController.Walk(movement.z);
    m_cameraController.Strafe(movement.x);
    m_cameraController.Rise(movement.y);

    m_camera.Mvp = m_cube.Transform;
    m_cameraController.UpdateProjection();
    m_camera.UpdateView();
    XMFLOAT3 rightVec;
    XMFLOAT3 upVec;
    XMFLOAT3 forwardVec;
    XMStoreFloat3(&rightVec, m_camera.Right);
    XMStoreFloat3(&upVec, m_camera.Up);
    XMStoreFloat3(&forwardVec, m_camera.Forward);

    auto viewOpt = m_bufferCache.Get(viewKey);
    auto projOpt = m_bufferCache.Get(projKey);
    auto mvpOpt = m_bufferCache.Get(mvpKey);

    if (!viewOpt || !projOpt || !mvpOpt)
    {
        LS::Log::TraceError(L"Failed to obtain buffers for this operation");
        return;
    }

    const auto viewBuff = viewOpt.value();
    const auto projBuff = projOpt.value();
    const auto mvpBuff = mvpOpt.value();

    // Update Buffers //
    m_command.UpdateConstantBuffer(mvpBuff.Get(), &m_camera.Mvp);
    m_command.UpdateConstantBuffer(viewBuff.Get(), &m_camera.View);
    m_command.UpdateConstantBuffer(projBuff.Get(), &m_camera.Projection);
}

void gt::dx11::ImGuiDx11::UpdateCube()
{
    const float rx = LS::Math::ToRadians(m_rotArr[0]);
    const float ry = LS::Math::ToRadians(m_rotArr[1]);
    const float rz = LS::Math::ToRadians(m_rotArr[2]);
    const XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
    m_cube.Rotation = rotQuat;
    m_cube.Scale = XMVectorSet(m_scaleArr[0], m_scaleArr[1], m_scaleArr[2], 1.0f);
    m_cube.Position = XMVectorSet(m_posArr[0], m_posArr[1], m_posArr[2], 1.0f);

    XMMATRIX scale = XMMatrixScalingFromVector(m_cube.Scale);
    XMMATRIX rot = XMMatrixRotationQuaternion(m_cube.Rotation);
    XMMATRIX translation = XMMatrixTranslationFromVector(m_cube.Position);

    m_cube.Transform = XMMatrixMultiply(translation, XMMatrixMultiply(scale, rot));
}

void gt::dx11::ImGuiDx11::UpdateFloor()
{
    const XMMATRIX scale = XMMatrixScalingFromVector(XMVectorSet(m_scaleArr[0], m_scaleArr[1], m_scaleArr[2], 1.0f));
    const XMMATRIX rotation = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
    const XMMATRIX translation = XMMatrixTranslationFromVector(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));

    g_floorTransform = XMMatrixMultiply(translation, XMMatrixMultiply(scale, rotation));
}

void gt::dx11::ImGuiDx11::ReadKeyboard()
{
    using enum LS::Input::KEYBOARD;
}

void gt::dx11::ImGuiDx11::Draw()
{
    const auto viewBuff = m_bufferCache.Get(viewKey).value();
    const auto projBuff = m_bufferCache.Get(projKey).value();
    const auto mvpBuff = m_bufferCache.Get(mvpKey).value();
    const auto vbBuff = m_bufferCache.Get(vbKey).value();
    const auto floorBuff = m_bufferCache.Get(floorKey).value();
    const auto ibBuff = m_bufferCache.Get(ibKey).value();

    // Command Usage //
    switch (DrawMethodSelection)
    {
    case 0:
        m_command.SetCullMethod(LS::Win32::CULL_METHOD::CULL_BACKFACE_CC);
        break;
    case 1:
        m_command.SetCullMethod(LS::Win32::CULL_METHOD::WIREFRAME);
        break;
    default:
        m_command.SetCullMethod(LS::Win32::CULL_METHOD::CULL_BACKFACE_CC);
        break;
    }
    m_command.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_command.SetInputLayout(m_inputLayout.Get());
    m_command.SetViewport(static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));
    m_command.BindVS(m_vertShader.Get());
    m_command.BindPS(m_pixShader.Get());
    m_command.SetVertexBuffer(vbBuff.Get(), sizeof(Vertex));
    m_command.SetIndexBuffer(ibBuff.Get());
    std::array<ID3D11Buffer*, 3> buffers{ viewBuff.Get(), projBuff.Get(), mvpBuff.Get() };
    m_command.BindVSConstantBuffers(buffers);

    m_renderer.Clear(m_clearColor, LS::Win32::DEPTH_STENCIL_MODE::DEFAULT);

    m_command.DrawIndexed((uint32_t)m_cube.Indices.size());

    m_camera.Mvp = g_floorTransform;
    m_command.UpdateConstantBuffer(mvpBuff.Get(), &m_camera.Mvp);

    //m_command.SetCullMethod(LS::Win32::CULL_METHOD::CULL_NONE);
    m_command.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_command.SetInputLayout(m_terrainIL.Get());
    m_command.BindVS(m_terrainShaderVs.Get());
    m_command.BindPS(m_pixShader.Get());
    m_command.SetVertexBuffer(floorBuff.Get(), sizeof(LS::Vec3F));
    m_command.SetIndexBuffer(nullptr);
    std::array<ID3D11Buffer*, 2> buffers2{ viewBuff.Get(), projBuff.Get() };
    m_command.BindVSConstantBuffers(buffers2);

    for (int i = 0u; i < CELL_DEPTH; ++i)
    {
        m_command.DrawVerts((uint32_t)CELL_WIDTH, i * CELL_DEPTH);
    }
}

LRESULT gt::dx11::WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Let ImGui handle itself, but we want to still allow rest of default window behavior to go to our current
    // window handler for the rest of the messages we do handle. 
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;
    return 0;
}