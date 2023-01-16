#define LS_WINDOWS_BUILD
#include "LSEFramework.h"
#include "LSTimer.h"

import Engine.Common;
import D3D11Lib;
import Win32Lib;
import Util.HLSLUtils;
import Helper.LSCommonTypes;

using namespace LS;
using namespace LS::Win32;
using namespace LS::Utils;
using namespace Microsoft::WRL;
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace DirectX;

#ifdef DEBUG
static const std::string shader_path = R"(build\x64\Debug\)";
#else
static const std::string shader_path = R"(build\x64\Release\)";
#endif

LSTimer<std::uint64_t, 1ul, 1000ul> g_timer;
std::array<float, 4> g_color = { 0.84f, 0.48f, 0.20f, 1.0f };
std::array<float, 4> g_color2 = { 0.0f, 0.74f, 0.60f, 1.0f };
std::array<float, 4> g_color3 = { 0.50f, 0.0f, 0.23f, 1.0f };

static std::array<float, 12> g_positions 
{
    0.0f, 1.0f, 0.20f, 1.0f,
    1.0f, 0.0f, 0.20f, 1.0f,
    -1.0f, 0.0f, 0.20f, 1.0f
};

//static std::array<uint32_t, 32 * 32> g_textureData;
static std::array<uint32_t, 1> g_textureData{ 0xFF32B3FF };

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

    std::cout << "Texture size: " << g_textureData.size() << "\n";
    // Device Setup //
    DeviceD3D11 device;
    device.CreateDevice();
    ComPtr<ID3D11DeviceContext4> immContext = device.GetImmediateContext();
    device.CreateSwapchain(window.get());

    auto rsSolidOptional = CreateRasterizerState2(device.GetDevice().Get(), SolidFill_NoneCull_CCWFront_DCE);
    auto rsWireframeOptional = CreateRasterizerState2(device.GetDevice().Get(), Wireframe_FrontCull_CCWFront_DCE);

    ComPtr<ID3D11RasterizerState2> rsSolid;
    rsSolid.Attach(rsSolidOptional.value_or(nullptr));
    ComPtr<ID3D11RasterizerState2> rsWireframe;
    rsWireframe.Attach(rsWireframeOptional.value_or(nullptr));

    // Create Texture //
    /*for (auto i = 0u; i < g_textureData.size(); ++i)
    {
        g_textureData[i] = 0xFF3200FF;
    }*/

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = 1;
    textureDesc.Height = 1;
    textureDesc.ArraySize = g_textureData.size();
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.MipLevels = 1;
    textureDesc.MiscFlags = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.SampleDesc.Count = 1;

    D3D11_SUBRESOURCE_DATA texData{};
    texData.pSysMem = g_textureData.data();
    texData.SysMemPitch = sizeof(uint32_t);
    ComPtr<ID3D11Texture2D> tex2D;
    auto texResult = device.GetDevice()->CreateTexture2D(&textureDesc, &texData, &tex2D);
    if (FAILED(texResult))
        return -20;

    ComPtr<ID3D11ShaderResourceView> pTexResView;

    D3D11_SHADER_RESOURCE_VIEW_DESC texresView{};
    D3D11_TEX2D_SRV tex2dSRV{};
    tex2dSRV.MipLevels = textureDesc.MipLevels;

    texresView.Format = textureDesc.Format;
    texresView.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    texresView.Texture2D = tex2dSRV;
    auto srvResult = device.GetDevice()->CreateShaderResourceView(tex2D.Get(), &texresView, &pTexResView);
    if (FAILED(srvResult))
    {
        return -21;
    }

    ComPtr<ID3D11SamplerState> pSampler;
    CD3D11_SAMPLER_DESC samplerDesc(CD3D11_DEFAULT{});
    auto samplerResult = device.GetDevice()->CreateSamplerState(&samplerDesc, &pSampler);
    if (FAILED(samplerResult))
        return -22;

    // END TEXTURE STUFF //

    // Render Target //
    ComPtr<ID3D11Texture2D> buffer;
    device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer));
    ComPtr< ID3D11RenderTargetView1> rtView;
    ComPtr< ID3D11RenderTargetView1> rtView2;
    rtView.Attach(CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get()));
    rtView2.Attach(CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get()));

    std::array<ID3D11RenderTargetView*, 2> rtViews = { rtView.Get(), rtView2.Get() };

    // Depth Stencil //
    ComPtr<ID3D11DepthStencilView> dsView;
    auto dsResult = device.CreateDepthStencilViewForSwapchain(rtView.Get(), &dsView);
    if (FAILED(dsResult))
        return -3;

    CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    ComPtr<ID3D11DepthStencilState> defaultState;
    auto dss = CreateDepthStencilState(device.GetDevice().Get(), defaultDepthDesc).value();
    defaultState.Attach(dss);
    HRESULT result;
    // Blend State
    ComPtr<ID3D11BlendState> blendState;
    CD3D11_BLEND_DESC blendDesc(CD3D11_DEFAULT{});

    result = device.CreateBlendState(blendDesc, &blendState);
    if (FAILED(result))
        return -4;

    auto createDeferredContext = [&]() -> ComPtr<ID3D11DeviceContext>
    {
        ComPtr<ID3D11DeviceContext> pDeferredContext;
        result = device.CreateDeferredContext(pDeferredContext.ReleaseAndGetAddressOf());
        if (FAILED(result))
            return nullptr;
        return pDeferredContext;
    };

    auto context1 = createDeferredContext();
    auto context2 = createDeferredContext();

    /*ComPtr<ID3D11CommandList> pCommandList;
    context1->ClearRenderTargetView(rtView.Get(), g_color.data());
    context1->FinishCommandList(false, pCommandList.ReleaseAndGetAddressOf());*/
    
    // BEGIN SHADER FILE OPERATIONS //
    auto vertexShader = L"VertexShader.cso";
    auto pixelShader = L"PixelShader.cso";
    auto pixelShader2 = L"PixelShader2.cso";

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
    auto psPath2 = path + L"\\" + pixelShader2;

    if (std::filesystem::exists(vsPath) && std::filesystem::exists(psPath))
    {
        std::cout << "Shaders exists!\n";
    }
    
    auto readFile = [](std::fstream& stream, std::filesystem::path filePath) -> std::vector<std::byte>
    {
        if (!stream.is_open() && !stream.good())
        {
            throw std::runtime_error("Failed to open file for read\n");
        }

        auto fileSize = std::filesystem::file_size(filePath);

        std::vector<std::byte> shaderData(fileSize);

        stream.read(reinterpret_cast<char*>(shaderData.data()), fileSize);
        stream.close();

        return shaderData;
    };

    std::fstream vsStream{ vsPath, vsStream.in | vsStream.binary };
    auto vsData = readFile(vsStream, vsPath);

    std::fstream psStream(psPath, std::fstream::in | std::fstream::binary);
    auto psData = readFile(psStream, psPath);
    
    std::fstream psStream2(psPath2, std::fstream::in | std::fstream::binary);
    auto psData2 = readFile(psStream2, psPath2);
    // END SHADER FILE OPERATIONS //

    // Compile Shader Objects //
    ComPtr<ID3D11VertexShader> vsShader;
    ComPtr<ID3D11PixelShader> psShader;
    ComPtr<ID3D11PixelShader> psShader2;

    auto vsResult = CompileVertexShaderFromByteCode(device.GetDevice().Get(), vsData, &vsShader);
    if (FAILED(vsResult))
    {
        ThrowIfFailed(vsResult, "Failed to compile vertex shader!\n");
    }
    
    auto psResult = CompilePixelShaderFromByteCode(device.GetDevice().Get(), psData, &psShader);
    if (FAILED(psResult))
    {
        ThrowIfFailed(psResult, "Failed to compile vertex shader!\n");
    }
    
    auto psResult2 = CompilePixelShaderFromByteCode(device.GetDevice().Get(), psData2, &psShader2);
    if (FAILED(psResult2))
    {
        ThrowIfFailed(psResult2, "Failed to compile vertex shader!\n");
    }

    LSShaderInputSignature vsSignature;
    //vsSignature.AddElement(SHADER_DATA_TYPE::UINT, "SV_InstanceID");
    vsSignature.AddElement(SHADER_DATA_TYPE::FLOAT4, "POSITION0");
    //vsSignature.AddElement(SHADER_DATA_TYPE::FLOAT2, "TEXCOORD0");
    auto layout = vsSignature.GetInputLayout();
    auto inputs = BuildFromShaderElements(layout);
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
    CD3D11_BUFFER_DESC bufferDesc(sizeof(g_positions), D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA subData{.pSysMem = g_positions.data(), .SysMemPitch = 0, .SysMemSlicePitch = 0};
    result = device.CreateBuffer(&bufferDesc, &subData, &vertexBuffer);
    if (FAILED(result))
    {
        return -2;
    }

    ComPtr<ID3D11RenderTargetView> rtViewOg = rtView;
    ComPtr<ID3D11RenderTargetView> rtViewOg2 = rtView2;

    // Camera // 
    xmvec posVec = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
    xmvec lookAtVec = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    xmvec upVec = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

    LSCamera camera(window->GetWidth(), window->GetHeight(), posVec, lookAtVec, upVec, 100.0f);
    
    // Informs how the GPU about the buffer types - we have two Matrix and Index Buffers here, the Vertex was created earlier above //
    CD3D11_BUFFER_DESC matBD(sizeof(float) * 16, D3D11_BIND_CONSTANT_BUFFER);
    CD3D11_BUFFER_DESC indexBD(g_indices.size() * sizeof(g_indices.front()), D3D11_BIND_INDEX_BUFFER);
    CD3D11_BUFFER_DESC colorBD(sizeof(float) * 4, D3D11_BIND_CONSTANT_BUFFER);
    
    // Ready the GPU Data //
    D3D11_SUBRESOURCE_DATA viewSRD, projSRD, modelSRD, indexSRD, color1SRD, color2SRD;
    viewSRD.pSysMem = &camera.View;
    projSRD.pSysMem = &camera.Projection;
    indexSRD.pSysMem = g_indices.data();
    color1SRD.pSysMem = g_color.data();
    color2SRD.pSysMem = g_color2.data();

    // Our Model's Translastion/Scale/Rotation Setup //
    xmmat modelScaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
    xmmat modelRotMat = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
    xmmat modelTransMat = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    xmmat modelTransform = XMMatrixIdentity();

    modelTransform = XMMatrixMultiply(modelTransMat, XMMatrixMultiply(modelScaleMat, modelRotMat));
    modelSRD.pSysMem = &modelTransform;

    // Initialize Buffers //
    ComPtr<ID3D11Buffer> viewBuffer, projBuffer, modelBuffer, indexBuffer, color1Buffer, color2Buffer;
    device.CreateBuffer(&matBD, &viewSRD, &viewBuffer);
    device.CreateBuffer(&matBD, &projSRD, &projBuffer);
    device.CreateBuffer(&matBD, &modelSRD, &modelBuffer);
    device.CreateBuffer(&indexBD, &indexSRD, &indexBuffer);
    device.CreateBuffer(&colorBD, &color1SRD, &color1Buffer);
    device.CreateBuffer(&colorBD, &color2SRD, &color2Buffer);

    // Setup states and buffers that only require one call here //
    /*BindVS(immContext.Get(), vsShader.Get());
    BindPS(immContext.Get(), psShader.Get());*/

    // Deferred Contexts //
    BindVS(context1.Get(), vsShader.Get());
    BindVS(context2.Get(), vsShader.Get());

    BindPS(context1.Get(), psShader2.Get());
    BindPS(context2.Get(), psShader2.Get());
    /*context1->PSSetShaderResources(0, 1, &pTexResView);
    context2->PSSetShaderResources(0, 1, &pTexResView);
    context1->PSSetSamplers(0, 1, &pSampler);
    context2->PSSetSamplers(0, 1, &pSampler);*/

    //SetInputlayout(immContext.Get(), pInputLayout.Get());
    SetInputlayout(context1.Get(), pInputLayout.Get());
    SetInputlayout(context2.Get(), pInputLayout.Get());

    std::array<ID3D11Buffer*, 4> buffers{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get(), 
        color1Buffer.Get()};
    
    std::array<ID3D11Buffer*, 4> buffers2{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get(), 
        color2Buffer.Get()};

    //BindVSConstantBuffers(immContext.Get(), 0, buffers);
    BindVSConstantBuffers(context1.Get(), 0, buffers);
    BindVSConstantBuffers(context2.Get(), 0, buffers2);

    UINT stride = sizeof(float) * 4;
    
    /*SetVertexBuffers(immContext.Get(), vertexBuffer.Get(), 1, 0, stride);
    SetIndexBuffer(immContext.Get(), indexBuffer.Get());
    SetRasterizerState(immContext.Get(), rsSolid.Get());
    */
    SetVertexBuffers(context1.Get(), vertexBuffer.Get(), 1, 0, stride);
    SetIndexBuffer(context1.Get(), indexBuffer.Get());
    SetRasterizerState(context1.Get(), rsSolid.Get());
    
    SetVertexBuffers(context2.Get(), vertexBuffer.Get(), 1, 0, stride);
    SetIndexBuffer(context2.Get(), indexBuffer.Get());
    SetRasterizerState(context2.Get(), rsSolid.Get());
    
    FLOAT color[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
    /*SetBlendState(immContext.Get(), blendState.Get());
    SetDepthStencilState(immContext.Get(), defaultState.Get(), 1);
    SetTopology(immContext.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetViewport(immContext.Get(),
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight())
    );*/
    
    SetBlendState(context1.Get(), blendState.Get());
    SetDepthStencilState(context1.Get(), defaultState.Get(), 1);
    SetTopology(context1.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetViewport(context1.Get(),
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight())
    );
    
    SetBlendState(context2.Get(), blendState.Get());
    SetDepthStencilState(context2.Get(), defaultState.Get(), 1);
    SetTopology(context2.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetViewport(context2.Get(),
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight())
    );
    /*SetViewport(context2.Get(),
        static_cast<float>(window->GetWidth() / 2.f),
        static_cast<float>(window->GetHeight() / 2.f)
    );*/

    SetRenderTarget(context1.Get(), rtView.Get(), dsView.Get());
    ClearRT(context1.Get(), rtView.Get(), g_color3);
    ClearDS(context1.Get(), dsView.Get());
    DrawIndexed(context1.Get(), g_indices.size(), 0, 0);

    SetRenderTarget(context2.Get(), rtView2.Get(), dsView.Get());
    ClearRT(context2.Get(), rtView2.Get(), g_color);
    ClearDS(context2.Get(), dsView.Get());
    DrawIndexed(context2.Get(), g_indices.size(), 0, 0);

    ComPtr<ID3D11CommandList> command1, command2;
    auto hr = context1->FinishCommandList(FALSE, &command1);
    if (FAILED(hr))
        return EXIT_FAILURE;
    hr = context2->FinishCommandList(FALSE, &command2);
    if (FAILED(hr))
        return EXIT_FAILURE;

    // Show Window and Start Timer - duh... //
    window->Show();
    g_timer.Start();
    while (window->IsRunning())
    {
        g_timer.Tick();
        /*SetRenderTarget(immContext.Get(), rtViewOg.Get(), dsView.Get());
        ClearRT(immContext.Get(), rtView.Get(), g_color3);
        ClearDS(immContext.Get(), dsView.Get());*/

        if (command1)
        {
            immContext->ExecuteCommandList(command1.Get(), FALSE);
        }
        if (command2)
        {
            immContext->ExecuteCommandList(command2.Get(), FALSE);
        }
        //DrawIndexed(immContext.Get(), g_indices.size(), 0, 0);
        Present1(device.GetSwapChain().Get(), 1);
        window->PollEvent();
        /*if (g_timer.GetTotalTimeTickedIn<std::chrono::seconds>().count() >= 5.0f)
        {
            g_color[0] = 0.0f;
            g_color[1] = 1.0f;
            g_color[2] = 1.0f;
            g_color[3] = 1.0f;
        }*/
    }
}