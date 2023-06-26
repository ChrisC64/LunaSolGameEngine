module;
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>
#include <fstream>
#include <iostream>
#include <format>

#include <wrl/client.h>
#include <directxmath/DirectXMath.h>
#include <directxmath/DirectXColors.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "LSTimer.h"

export module DX11CubeApp;

export import Engine.Common;
export import D3D11Lib;
export import Platform.Win32Window;
export import Util.HLSLUtils;
export import Helper.LSCommonTypes;
export import Helper.PipelineFactory;
import Engine.Logger;
import Helper.IO;
import GeometryGenerator;
import MathLib;

export namespace gt
{
    using namespace Microsoft::WRL;
    using namespace LS;
    using namespace LS::Win32;
    using namespace LS::Utils;
    using namespace std::chrono;
    using namespace std::chrono_literals;
    using namespace DirectX;

    constexpr auto SCREEN_WIDTH = 1920u;
    constexpr auto SCREEN_HEIGHT = 1080u;

#ifdef DEBUG
    const std::string shader_path = R"(build\x64\Debug\)";
#else
    const std::string shader_path = R"(build\x64\Release\)";
#endif

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
    LS::ENGINE_CODE Init();
    void Run();

    auto App = CreateAppRef(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App", std::move(Init), std::move(Run));

    LS::Win32::DeviceD3D11 g_device;
    Cube g_Cube;
    XMVECTOR g_UpVec = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    XMVECTOR g_LookAtDefault = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DX::DXCamera g_camera(SCREEN_WIDTH, SCREEN_HEIGHT, XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f), g_LookAtDefault, g_UpVec, 100.0f);
    constexpr auto g_indexData = Geo::Generator::CreateCubeIndexArray();

    auto CreateVertexShader(const LS::Win32::DeviceD3D11& device, ComPtr<ID3D11VertexShader>& shader, std::vector<std::byte>& byteCode) -> bool
    {
        auto vsResult = CompileVertexShaderFromByteCode(device.GetDevice().Get(), byteCode, &shader);
        if (FAILED(vsResult))
        {
            return false;
        }
        return true;
    }

    auto CreatePixelShader(const LS::Win32::DeviceD3D11& device, ComPtr<ID3D11PixelShader>& shader, std::vector<std::byte>& byteCode) -> bool
    {
        auto psResult = CompilePixelShaderFromByteCode(device.GetDevice().Get(), byteCode, &shader);
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
        /*auto interpolation = elapsed / 1000.0f;
        auto start = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
        auto end = XMQuaternionRotationRollPitchYaw(0.0f, 360.0f, 0.0f);
        auto slerp = XMQuaternionSlerp(start, end, interpolation);

        g_Cube.Rotation = XMQuaternionRotationRollPitchYawFromVector(slerp);*/
        if (elapsed == 0)
            return;
        auto radians = LS::Math::ToRadians(10 / elapsed);
        XMVECTOR rotationAxis = XMVectorSet(0.450f, 1.0f, 0.250f, 1.0f);
        auto rotQuat = XMQuaternionRotationAxis(rotationAxis, radians);
        g_Cube.Rotation = XMQuaternionMultiply(g_Cube.Rotation, rotQuat);

    }

    void UpdateCamera()
    {
        g_camera.Mvp = g_Cube.Transform;
        g_camera.UpdateProjection();
        g_camera.UpdateView();
    }

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
    void PreDraw(ComPtr<ID3D11DeviceContext4>& context);
    void DrawScene(ComPtr<ID3D11DeviceContext4>& context);
    //TODO: Create common colors in Engine Common or make color conceptl
    std::array<float, 4> g_red = { 1.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 4> g_green = { 0.0f, 1.0f, 0.0f, 1.0f };
    std::array<float, 4> g_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    std::array<float, 4> g_black = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 4> g_white = { 1.0f, 1.0f, 1.0f, 1.0f };

}

module : private;

LS::ENGINE_CODE gt::Init()
{
    using enum LS::ENGINE_CODE;

    auto& window = App->Window;

    g_device.CreateDevice();

    auto immContext = g_device.GetImmediateContext();

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
    
    CD3D11_BUFFER_DESC vbDesc(32 * 8, D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA subData;
    subData.pSysMem = g_Cube.Verts.data();
    subData.SysMemPitch = 0;
    subData.SysMemSlicePitch = 0;

    bool bufferResult = g_device.CreateBuffer(&vbDesc, &subData, &vertexBuffer);
    if (FAILED(bufferResult))
    {
        LS::Log::TraceError(L"Failed to create vertex buffer");
        return RESOURCE_CREATION_FAILED;
    }

    // Index Buffer //
    
    CD3D11_BUFFER_DESC indexBD(static_cast<UINT>(g_indexData.size()) * sizeof(g_indexData.front()), D3D11_BIND_INDEX_BUFFER);
    D3D11_SUBRESOURCE_DATA indexSbd;
    indexSbd.pSysMem = g_indexData.data();
    indexSbd.SysMemPitch = 0;
    indexSbd.SysMemSlicePitch = 0;
    bufferResult = g_device.CreateBuffer(&indexBD, &indexSbd, &indexBuffer);
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

    g_device.CreateBuffer(&matBd, &viewSd, &g_viewBuffer);
    g_device.CreateBuffer(&matBd, &projSd, &g_projBuffer);
    g_device.CreateBuffer(&matBd, &modelSd, &g_modelBuffer);

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

    CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    ComPtr<ID3D11DepthStencilState> defaultState;
    auto dss = CreateDepthStencilState(g_device.GetDevice().Get(), defaultDepthDesc).value();
    defaultState.Attach(dss);
    LS::Log::TraceDebug(L"Depth stencil created!!");

    return LS_SUCCESS;
}

void gt::Run()
{
    App->Window->Show();
    g_timer.Start();
    auto context = g_device.GetImmediateContext();
    while (App->Window->IsOpen())
    {
        auto elapsed = g_timer.GetTotalTimeTicked();
        auto deltaTime = g_timer.GetDeltaTime();
        App->Window->PollEvent();
        g_timer.Tick();
        
        RotateCube(deltaTime.count());
        UpdateCubeTransform();
        UpdateCamera();

        std::cout << std::format("Elapsed: {}\n\tDelta: {}\n", elapsed.count(), deltaTime.count());
        PreDraw(context);
        DrawScene(context);
        Present1(g_device.GetSwapChain().Get(), 1);


        // Update Buffers //
        LS::Win32::UpdateSubresource(context.Get(), g_modelBuffer.Get(), 0, &g_camera.Mvp);
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