#define LS_WINDOWS_BUILD
#include "LSEFramework.h"
#include "LSTimer.h"

import Engine.Common;
import D3D11Lib;
import Win32Lib;
import Util.HLSLUtils;

using namespace LS;
using namespace Microsoft::WRL;
using namespace std::chrono;
using namespace std::chrono_literals;

#ifdef DEBUG
static const std::string shader_path = R"(build\x64\Debug\)";
#else
static const std::string shader_path = R"(build\x64\Release\)";
#endif

LS::LSTimer<std::uint64_t, 1ul, 1000ul> g_timer;
std::array<float, 4> g_color = { 0.84f, 0.48f, 0.20f, 1.0f };

static std::array<float, 12> g_positions 
{
    0.0f, 1.0f, 0.20f, 1.0f,
    1.0f, 0.0f, 0.20f, 1.0f,
    -1.0f, 0.0f, 0.20f, 1.0f
};

static std::array<uint32_t, 3> g_indices{ 0, 2, 1 };

void GpuDraw(ID3D11CommandList** commandList, ID3D11DeviceContext3* context, ID3D11RenderTargetView1* rtv)
{
    context->ClearRenderTargetView(rtv, g_color.data());
    context->FinishCommandList(false, commandList);
}

int main()
{
    std::cout << "Hello Luna Sol Game Engine!\n";

    Ref<LSWindowBase> window = LS::LSCreateWindow(800u, 700u, L"Hello App");
    // Device Setup //
    Win32::DeviceD3D11 device;
    device.CreateDevice();
    device.CreateSwapchain(window.get());

    auto rsSolidOptional = Win32::CreateRasterizerState2(device.GetDevice().Get(), LS::SolidFill_NoneCull_CCWFront_DCE);
    auto rsWireframeOptional = Win32::CreateRasterizerState2(device.GetDevice().Get(), LS::Wireframe_FrontCull_CCWFront_DCE);

    ComPtr<ID3D11RasterizerState2> rsSolid;
    rsSolid.Attach(rsSolidOptional.value_or(nullptr));
    ComPtr<ID3D11RasterizerState2> rsWireframe;
    rsWireframe.Attach(rsWireframeOptional.value_or(nullptr));

    // Render Target //
    ComPtr<ID3D11Texture2D> buffer;
    device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer));
    ComPtr< ID3D11RenderTargetView1> rtView;
    rtView.Attach(Win32::CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get()));
    // Depth Stencil //
    ComPtr<ID3D11DepthStencilView> dsView;
    auto dsResult = device.CreateDepthStencilViewForSwapchain(rtView.Get(), &dsView);
    if (FAILED(dsResult))
        return -3;

    CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    auto defaultState = Win32::CreateDepthStencilState(device.GetDevice().Get(), defaultDepthDesc).value();
    HRESULT result;
    // Blend State
    ComPtr<ID3D11BlendState> blendState;
    CD3D11_BLEND_DESC blendDesc(CD3D11_DEFAULT{});

    result = device.CreateBlendState(blendDesc, &blendState);
    if (FAILED(result))
        return -4;

    /*ComPtr<ID3D11DeviceContext> pDeferredContext;
    result = device.CreateDeferredContext(pDeferredContext.ReleaseAndGetAddressOf());
    if (FAILED(result))
        return EXIT_FAILURE;*/

    //ComPtr<ID3D11CommandList> pCommandList;
    //pDeferredContext->ClearRenderTargetView(rtView.Get(), g_color.data());
    //pDeferredContext->FinishCommandList(false, pCommandList.ReleaseAndGetAddressOf());
    
    // BEGIN SHADER FILE OPERATIONS //
    auto vertexShader = L"VertexShader.cso";
    auto pixelShader = L"PixelShader.cso";

    std::array<wchar_t, _MAX_PATH> modulePath;
    if (!GetModuleFileName(nullptr, modulePath.data(), modulePath.size()))
    {
        throw std::runtime_error("Failed to find module path\n");
    }

    auto path = std::wstring(modulePath.data());
    auto lastOf = path.find_last_of(L"\\");
    path.erase(lastOf);

    auto vsPath = path + L"\\" + vertexShader;
    auto psPath = path + L"\\" + pixelShader;

    if (std::filesystem::exists(vsPath) && std::filesystem::exists(psPath))
    {
        std::cout << "Shaders exists!\n";
    }

    std::fstream vsStream{ vsPath, vsStream.in | vsStream.binary };
    if (!vsStream.is_open() && !vsStream.good())
    {
        throw std::runtime_error("Failed to open vsStream\n");
    }

    std::fstream psStream(psPath, std::fstream::in | std::fstream::binary);
    if (!psStream.is_open() && !psStream.good())
    {
        throw std::runtime_error("Failed to open psStream\n");
    }

    auto sizeVS = std::filesystem::file_size(vsPath);
    auto sizePS = std::filesystem::file_size(psPath);

    std::vector<std::byte> vsData(sizeVS);
    vsStream.read(reinterpret_cast<char*>(vsData.data()), vsData.size());
    vsStream.close();

    std::vector<std::byte> psData(sizePS);
    psStream.read(reinterpret_cast<char*>(psData.data()), psData.size());
    psStream.close();
    // END SHADER FILE OPERATIONS //

    // Compile Shader Objects //
    ComPtr<ID3D11VertexShader> vsShader;
    ComPtr<ID3D11PixelShader> psShader;
    auto vsResult = LS::Win32::CompileVertexShaderFromByteCode(device.GetDevice().Get(), vsData, &vsShader);
    if (FAILED(vsResult))
    {
        LS::Utils::ThrowIfFailed(vsResult, "Failed to compile vertex shader!\n");
    }
    auto psResult = LS::Win32::CompilePixelShaderFromByteCode(device.GetDevice().Get(), psData, &psShader);
    if (FAILED(psResult))
    {
        LS::Utils::ThrowIfFailed(psResult, "Failed to compile vertex shader!\n");
    }

    LSShaderInputSignature vsSignature;
    //vsSignature.AddElement(SHADER_DATA_TYPE::UINT, "SV_InstanceID");
    vsSignature.AddElement(SHADER_DATA_TYPE::FLOAT4, "POSITION0");
    //vsSignature.AddElement(SHADER_DATA_TYPE::FLOAT2, "TEXCOORD0");
    auto layout = vsSignature.GetInputLayout();
    auto inputs = Utils::BuildFromShaderElements(layout);
    if (!inputs)
    {
        throw std::runtime_error("Failed to create input layout from shader elements\n");
    }

    auto valInputs = inputs.value();
    ComPtr<ID3D11InputLayout> pInputLayout;
    if (FAILED(device.CreateInputLayout(valInputs, vsData, &pInputLayout)))
    {
        throw std::runtime_error("Failed to create input layout from device\n");
    }
    //TODO: Implement missing setups below that don't have an appropriate function call

    ComPtr<ID3D11Buffer> vertexBuffer;
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(g_positions);
    bufferDesc.StructureByteStride = 0;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subData{};
    subData.pSysMem = g_positions.data();
    subData.SysMemPitch = 0;
    subData.SysMemSlicePitch = 0;
    result = device.CreateBuffer(&bufferDesc, &subData, &vertexBuffer);
    if (FAILED(result))
    {
        return -2;
    }

    UINT stride = sizeof(float) * 4;
    UINT offset = 0;

    ComPtr<ID3D11RenderTargetView> rtViewOg = rtView;

    // Camera // 
    xmvec posVec = DirectX::XMVectorSet(0.0f, 0.0f, -15.0f, 1.0f);
    xmvec lookAtVec = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    xmvec upVec = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

    LSCamera camera(window->GetWidth(), window->GetHeight(), posVec, lookAtVec, upVec, 100.0f);
    // Informs how the GPU about the buffer types - we have two Matrix and Index Buffers here, the Vertex was created earlier above //
    CD3D11_BUFFER_DESC matBD(sizeof(float) * 16, D3D11_BIND_CONSTANT_BUFFER);
    CD3D11_BUFFER_DESC indexBD(g_indices.size() * sizeof(g_indices.front()), D3D11_BIND_INDEX_BUFFER);
    // Ready the GPU Data //
    D3D11_SUBRESOURCE_DATA viewSRD, projSRD, modelSRD, indexSRD;
    viewSRD.pSysMem = &camera.m_view;
    projSRD.pSysMem = &camera.m_projection;
    indexSRD.pSysMem = g_indices.data();
    // Our Model's Translastion/Scale/Rotation Setup //
    xmmat modelScaleMat = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
    xmmat modelRotMat = DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
    xmmat modelTransMat = DirectX::XMMatrixTranslation(0.0f, 0.0f, 5.0f);
    xmmat modelTransform = DirectX::XMMatrixIdentity();

    modelTransform = DirectX::XMMatrixMultiply(modelTransMat, DirectX::XMMatrixMultiply(modelScaleMat, modelRotMat));
    modelSRD.pSysMem = &modelTransform;

    // Initialize Buffers //
    ComPtr<ID3D11Buffer> viewBuffer, projBuffer, modelBuffer, indexBuffer;
    device.CreateBuffer(&matBD, &viewSRD, &viewBuffer);
    device.CreateBuffer(&matBD, &projSRD, &projBuffer);
    device.CreateBuffer(&matBD, &modelSRD, &modelBuffer);
    device.CreateBuffer(&indexBD, &indexSRD, &indexBuffer);

    // Setup states and buffers that only require one call here //
    Win32::BindVS(device.GetImmediateContext().Get(), vsShader.Get());
    Win32::BindPS(device.GetImmediateContext().Get(), psShader.Get());
    Win32::SetInputlayout(device.GetImmediateContext().Get(), pInputLayout.Get());

    std::array<ID3D11Buffer*, 3> buffers{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get() };
    device.GetImmediateContextPtr()->VSSetConstantBuffers(0, buffers.size(), buffers.data());
    Win32::SetVertexBuffers(device.GetImmediateContext().Get(), vertexBuffer.Get(), 1, 0, stride);
    device.GetImmediateContextPtr()->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    device.GetImmediateContextPtr()->RSSetState(rsSolid.Get());
    
    FLOAT color[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
    device.GetImmediateContextPtr()->OMSetBlendState(blendState.Get(), color, 0xffffffff);
    device.GetImmediateContextPtr()->OMSetDepthStencilState(defaultState, 1);
    Win32::SetTopology(device.GetImmediateContext().Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Win32::SetViewport(device.GetImmediateContext().Get(),
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight())
    );

    // Show Window and Start Timer - duh... //
    window->Show();
    g_timer.Start();
    while (window->IsRunning())
    {
        g_timer.Tick();
        Win32::SetRenderTarget(device.GetImmediateContext().Get(), rtViewOg.Get(), dsView.Get());
        Win32::ClearRT(device.GetImmediateContext().Get(), rtView.Get(), g_color);
        Win32::ClearDS(device.GetImmediateContext().Get(), dsView.Get());
        Win32::DrawIndexed(device.GetImmediateContext().Get(), g_indices.size(), 0, 0);
        Win32::Present1(device.GetSwapChain().Get(), 1);
        window->PollEvent();
        if (g_timer.GetTotalTimeTickedIn<std::chrono::seconds>().count() >= 5.0f)
        {
            g_color[0] = 0.0f;
            g_color[1] = 1.0f;
            g_color[2] = 1.0f;
            g_color[3] = 1.0f;
        }
    }
}