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
import <vector>;
import <format>;

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

constexpr size_t CELL_ROWS = 120;
constexpr size_t CELL_COLS = 150;
constexpr size_t CELL_TOTAL = CELL_ROWS * CELL_COLS;
std::vector<LS::Vec3<float>> g_floor = LS::Geo::Generator::CreateFloorLH<CELL_COLS, CELL_ROWS>();
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
        uint32_t fpsTarget = 30;
        XMVECTOR m_UpVec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR m_LookAtDefault = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        std::array<float, 4> m_clearColor{ 1.0f, 0.43f, 0.02f, 1.0f };
        std::array<float, 4> m_rotArr{ 0.0f, 0.0f, 0.0f, 1.0f };
        std::array<float, 4> m_scaleArr{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::array<float, 4> m_posArr{ 0.0f, 5.0f, 0.0f, 1.0f };
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
        int DssSelection = 0;
        float m_moveSpeed = 0.25f;
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
    if (!LS::Win32::InitHelperStates(m_renderer.GetDevice()))
    {
        return LS::System::CreateFailCode("Failed to create global render states");
    }
    return LS::System::CreateSuccessCode();
}

void gt::dx11::ImGuiDx11::Run()
{
    using namespace std::chrono_literals;

    m_Window->Show();
    ImGuiIO& io = ImGui::GetIO();

    auto currWidth = m_Window->GetWidth();
    auto currHeight = m_Window->GetHeight();
    const char* items[]{ "solid", "wireframe" };
    const char* dssItems[]{ "none", "default", "read", "reverse z", "readReverseZ", "dssDefault" };
    using targetFps = std::ratio<1, 60>;
    using frameDuration = std::chrono::duration<double, targetFps>;
    const frameDuration fps = frameDuration(1s);
    const double drawTarget = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(frameDuration(1)).count();
    uint64_t frameCounter = 0;
    double currentTimeSecs = 0.0;
    double timepoint = 0.0;
    double deltaTimeMs = 0.0;
    double elapsedDeltaMs = 0.0;
    uint64_t framesPerSecond = 0;
    m_clock.Start();
    while (m_Window->IsOpen())
    {
        m_clock.Tick();
        currentTimeSecs = m_clock.TotalTimeAs<double, std::ratio<1, 1>>();
        deltaTimeMs = m_clock.DeltaTimeAs<double, std::milli>();
        elapsedDeltaMs += deltaTimeMs;
        m_Window->PollEvent();
        auto mousePos = m_Window->GetMousePos();
        if (currWidth != m_Window->GetWidth() && currHeight != m_Window->GetHeight())
        {
            m_renderer.Resize(m_Window->GetWidth(), m_Window->GetHeight());
            m_renderer.SetViewport(m_Window->GetWidth(), m_Window->GetHeight());
            currWidth = m_Window->GetWidth();
            currHeight = m_Window->GetHeight();
        }

        Update();

        if (elapsedDeltaMs < drawTarget)
        {
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello Gui");
        ImGui::Text("Width: %d", currWidth);
        ImGui::Text("Height: %d", currHeight);
        ImGui::Text("Mouse Pos: %d x %d", mousePos.x, mousePos.y);
        ImGui::SliderFloat3("Color", m_clearColor.data(), 0.0f, 1.0f);
        ImGui::SliderFloat3("Cube Pos", m_posArr.data(), 0.0f, 100.0f);
        ImGui::SliderFloat3("Cube Rot", m_rotArr.data(), 0.0f, 360.0f);
        ImGui::SliderFloat3("Cube Scale", m_scaleArr.data(), 1.0f, 20.0f);
        ImGui::SliderFloat("Camera Fov", &(m_camera.FovVertical), 30.0f, 120.0f);
        ImGui::SliderFloat("Camera Far", &(m_camera.FarZ), 0.001f, 1000.0f);
        ImGui::SliderFloat("Camera AR", &(m_camera.AspectRatio), 0.85f, 2.16f);
        ImGui::InputFloat("Cam Move Speed", &m_moveSpeed, 0.025f);
        ImGui::Text("Time: %.6f", currentTimeSecs);
        ImGui::Text("Delta Time: %.6f ms", deltaTimeMs);
        ImGui::Text("Draw Target: %.6f ms", drawTarget);
        const auto cpos = m_camera.PositionF3();
        ImGui::Text("Camera Pos: (%.4f, %.4f %.4f)", cpos.x, cpos.y, cpos.z);
        ImGui::Text("Target FPS: %.6f", fps.count());
        ImGui::Text("FPS (actual): %u", framesPerSecond);
        ImGui::Text("Frame Index: %u", m_renderer.GetFrameIndex());

        ImGui::BeginListBox("DM", ImVec2(300.0f, 50.0f));
        ImGui::ListBox("Draw Method", &DrawMethodSelection, items, 2);
        ImGui::EndListBox();
        
        ImGui::BeginListBox("DSS", ImVec2(300.0f, 100.0f));
        ImGui::ListBox("Depth Stencil State", &DssSelection, dssItems, 6);
        ImGui::EndListBox();
        
        ImGui::End();

        Draw();
        frameCounter++;
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
        elapsedDeltaMs = elapsedDeltaMs - drawTarget;
        if (currentTimeSecs - timepoint >= 1.0)
        {
            timepoint += 1.0;
            framesPerSecond = frameCounter;
            frameCounter = 0;
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void gt::dx11::ImGuiDx11::CompileShaders()
{
    LS::Log::TraceDebug(L"Compiling shaders....");
    auto currPath = std::filesystem::current_path();
    LS::Log::TraceDebug(currPath.wstring());
    // Shader compilation //
    std::wstring vsPath = std::format(L"{}\\VertexShader.cso", m_appDir.wstring());
    std::wstring psPath = std::format(L"{}\\PixelShader.cso", m_appDir.wstring());
    std::wstring terrainPath = std::format(L"{}\\TerrainVS.cso", m_appDir.wstring());

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
    const auto floorResult = m_bufferCache.CreateVertexBuffer<LS::Vec3<float>>(g_floor, m_renderer.GetDevice());
    floorKey = floorResult.value_or(max);

    const auto ibResult = m_bufferCache.CreateIndexBuffer(m_cube.Indices, m_renderer.GetDevice());
    ibKey = ibResult.value_or(max);

    // Camera Buffers //
    const auto viewResult = m_bufferCache.CreateConstantBuffer(&(m_camera.View), m_renderer.GetDevice());
    viewKey = viewResult.value_or(max);

    const auto projResult = m_bufferCache.CreateConstantBuffer(&(m_camera.Projection), m_renderer.GetDevice());
    projKey = projResult.value_or(max);

    const auto mvpResult = m_bufferCache.CreateConstantBuffer(&(m_camera.Mvp), m_renderer.GetDevice());
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
    auto dt = m_clock.DeltaTimeAs<double, std::milli>();

    float movespeed = m_moveSpeed * (float)dt;

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
    const float rx = (float)LS::Math::ToRadians(m_rotArr[0]);
    const float ry = (float)LS::Math::ToRadians(m_rotArr[1]);
    const float rz = (float)LS::Math::ToRadians(m_rotArr[2]);
    const XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rx, ry, rz);
    m_cube.Rotation = rotQuat;
    m_cube.Scale = XMVectorSet(m_scaleArr[0], m_scaleArr[1], m_scaleArr[2], 1.0f);
    m_cube.Position = XMVectorSet(m_posArr[0], m_posArr[1], m_posArr[2], 1.0f);

    XMMATRIX scale = XMMatrixScalingFromVector(m_cube.Scale);
    XMMATRIX rot = XMMatrixRotationQuaternion(m_cube.Rotation);
    XMMATRIX translation = XMMatrixTranslationFromVector(m_cube.Position);

    //m_cube.Transform = XMMatrixMultiply(translation, XMMatrixMultiply(scale, rot));
    m_cube.Transform = XMMatrixMultiply(translation, XMMatrixMultiply(rot, scale));
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
}

void gt::dx11::ImGuiDx11::Draw()
{
    const auto viewBuff = m_bufferCache.Get(viewKey).value();
    const auto projBuff = m_bufferCache.Get(projKey).value();
    const auto mvpBuff = m_bufferCache.Get(mvpKey).value();
    const auto vbBuff = m_bufferCache.Get(vbKey).value();
    const auto floorBuff = m_bufferCache.Get(floorKey).value();
    const auto ibBuff = m_bufferCache.Get(ibKey).value();

    m_command.SetViewport(static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));
    switch (DssSelection)
    {
    case 0:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDepthNone().Get(), 1);
        break;
    case 1:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDepthDefault().Get(), 1);
        break;
    case 2:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDepthRead().Get(), 1);
        break;
    case 3:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDepthReverseZ().Get(), 1);
        break;
    case 4:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDepthReadReverseZ().Get(), 1);
        break;
    case 5:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDSDefault().Get(), 1);
        break;
    default:
        m_command.SetDepthStencilState(LS::Win32::GlobalStates()->GetDepthDefault().Get(), 1);
        break;
    }

    m_renderer.Clear(m_clearColor, LS::Win32::DEPTH_STENCIL_MODE::DEFAULT);

    // Draw floor //
    m_command.UpdateConstantBuffer(mvpBuff.Get(), &g_floorTransform);
    switch (DrawMethodSelection)
    {
    case 0:
        m_command.SetRasterizerState(LS::Win32::GlobalStates()->GetSolid().Get());
        break;
    case 1:
        m_command.SetRasterizerState(LS::Win32::GlobalStates()->GetWireframe().Get());
        break;
    default:
        m_command.SetRasterizerState(LS::Win32::GlobalStates()->GetSolid().Get());
        break;
    }

    m_command.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_command.SetInputLayout(m_terrainIL.Get());
    m_command.BindVS(m_terrainShaderVs.Get());
    m_command.BindPS(m_pixShader.Get());
    m_command.SetVertexBuffer(floorBuff.Get(), sizeof(float) * 3);
    std::array<ID3D11Buffer*, 2> buffers2{ viewBuff.Get(), projBuff.Get() };
    m_command.BindVSConstantBuffers(buffers2);
    const uint32_t vertsPerRow = 4 + (2 * (CELL_COLS - 1));

    for (int i = 0u; i < CELL_ROWS; ++i)
    {
        m_command.DrawVerts((uint32_t)vertsPerRow, i * vertsPerRow);
    }

    // Draw Cube //
    m_command.UpdateConstantBuffer(mvpBuff.Get(), &m_camera.Mvp);
    switch (DrawMethodSelection)
    {
    case 0:
        m_command.SetRasterizerState(LS::Win32::GlobalStates()->GetSolid().Get());
        break;
    case 1:
        m_command.SetRasterizerState(LS::Win32::GlobalStates()->GetWireframe().Get());
        break;
    default:
        m_command.SetRasterizerState(LS::Win32::GlobalStates()->GetSolid().Get());
        break;
    }
    m_command.SetPrimTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_command.SetInputLayout(m_inputLayout.Get());
    m_command.BindVS(m_vertShader.Get());
    m_command.BindPS(m_pixShader.Get());
    m_command.SetVertexBuffer(vbBuff.Get(), sizeof(Vertex));
    m_command.SetIndexBuffer(ibBuff.Get());
    std::array<ID3D11Buffer*, 3> buffers{ viewBuff.Get(), projBuff.Get(), mvpBuff.Get() };
    m_command.BindVSConstantBuffers(buffers);

    m_command.DrawIndexed((uint32_t)m_cube.Indices.size());
}

LRESULT gt::dx11::WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Let ImGui handle itself, but we want to still allow rest of default window behavior to go to our current
    // window handler for the rest of the messages we do handle. 
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;
    return 0;
}