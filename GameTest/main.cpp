#define LS_WINDOWS_BUILD
#include "LSEFramework.h"
#include "LSTimer.h"

import Engine.Common;
import D3D11Lib;
import Win32Lib;
import Util.HLSLUtils;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;

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
std::array<float, 4> g_black = { 0.0f, 0.0f, 0.0f, 1.0f };

static std::array<float, 24> g_positions // POS / UV // Padding added after each entry
{   // POSITION              // UV       // PADDING //
    0.0f, 1.0f, 0.0f, 1.0f,  0.5f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f
};
static std::array<uint32_t, 3> g_indices{ 0, 2, 1 };

static std::array<float, 36> g_positions2
{
    1.0f, 1.0f, 0.20f, 1.0f,  1.0f, 0.0f, //0
    1.0f, 0.0f, 0.20f, 1.0f,  1.0f, 1.0f,//1
    -1.0f, 0.0f, 0.20f, 1.0f, 0.0f, 1.0f,//2
    1.0f, 1.0f, 0.20f, 1.0f,  1.0f, 0.0f,//3
    -1.0f, 1.0f, 0.20f, 1.0f, 0.0f, 0.0f,//4
    -1.0f, 0.0f, 0.20f, 1.0f, 0.0f, 1.0f //5
};
static std::array<uint32_t, 6> g_indices2{ 0, 2, 1, 3, 5, 4 };

constexpr uint32_t SCREEN_WIDTH = 800;
constexpr uint32_t SCREEN_HEIGHT = 700;

//static std::array<uint32_t, 32 * 32> g_textureData;
static std::array<uint32_t, SCREEN_WIDTH* SCREEN_HEIGHT> g_textureData{};


void GpuDraw(ID3D11CommandList** commandList, ID3D11DeviceContext3* context, ID3D11RenderTargetView1* rtv)
{
    context->ClearRenderTargetView(rtv, g_color.data());
    context->FinishCommandList(false, commandList);
}

int main()
{
    std::cout << "Hello Luna Sol Game Engine!\n";

    Ref<LSWindowBase> window = LS::LSCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, L"Hello App");

    std::cout << "Texture size: " << g_textureData.size() << "\n";
    // Device Setup //
    DeviceD3D11 device;
    device.CreateDevice();

    ComPtr<ID3D11DeviceContext4> immContext = device.GetImmediateContext();
    device.CreateSwapchain(window.get());
    //device.CreateSwapchainAsTexture(window.get());

    auto rsSolidOptional = CreateRasterizerState2(device.GetDevice().Get(), SolidFill_NoneCull_CCWFront_DCE);
    auto rsWireframeOptional = CreateRasterizerState2(device.GetDevice().Get(), Wireframe_FrontCull_CCWFront_DCE);

    ComPtr<ID3D11RasterizerState2> rsSolid;
    rsSolid.Attach(rsSolidOptional.value_or(nullptr));
    ComPtr<ID3D11RasterizerState2> rsWireframe;
    rsWireframe.Attach(rsWireframeOptional.value_or(nullptr));

    // Create Texture //
    for (auto i = 0; i < g_textureData.size(); ++i)
    {
        g_textureData[i] = 0xFFFF00FF;
    }

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = SCREEN_WIDTH;
    textureDesc.Height = SCREEN_HEIGHT;
    textureDesc.ArraySize = 1;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.MipLevels = 1;
    textureDesc.MiscFlags = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.SampleDesc.Count = 1;

    D3D11_SUBRESOURCE_DATA texData{};
    texData.pSysMem = &g_textureData;
    texData.SysMemPitch = sizeof(uint32_t) * textureDesc.Width;
    texData.SysMemSlicePitch = 0;
    ComPtr<ID3D11Texture2D> pRenderTargetTexture;
    auto texResult = device.GetDevice()->CreateTexture2D(&textureDesc, NULL, &pRenderTargetTexture);
    if (FAILED(texResult))
        return -20;

    ComPtr<ID3D11ShaderResourceView> pRTShaderResView;

    D3D11_SHADER_RESOURCE_VIEW_DESC texresView{};
    D3D11_TEX2D_SRV tex2dSRV{};
    tex2dSRV.MipLevels = textureDesc.MipLevels;

    texresView.Format = textureDesc.Format;
    texresView.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    texresView.Texture2D = tex2dSRV;
    auto srvResult = device.GetDevice()->CreateShaderResourceView(pRenderTargetTexture.Get(), &texresView, &pRTShaderResView);
    if (FAILED(srvResult))
    {
        return -21;
    }

    ComPtr<ID3D11SamplerState> pSampler;
    CD3D11_SAMPLER_DESC samplerDesc(CD3D11_DEFAULT{});
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    auto samplerResult = device.GetDevice()->CreateSamplerState(&samplerDesc, &pSampler);
    if (FAILED(samplerResult))
        return -22;

    // END TEXTURE STUFF //

    // Render Target //
    ComPtr<ID3D11Texture2D> buffer;
    device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer));
    D3D11_TEXTURE2D_DESC swpDesc;
    buffer->GetDesc(&swpDesc);
    ComPtr<ID3D11RenderTargetView1> defaultRTView;
    ComPtr<ID3D11RenderTargetView1> pTextureRTView;
    defaultRTView.Attach(CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get()));
    // Render Target as Texture2D // 
    D3D11_RENDER_TARGET_VIEW_DESC1 pTextureRT{};
    pTextureRT.Format = textureDesc.Format;
    pTextureRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    D3D11_TEX2D_RTV1 texRTV{};
    texRTV.MipSlice = 0;
    texRTV.PlaneSlice = 0;
    pTextureRT.Texture2D = texRTV;
    auto rtvTexture = CreateRenderTargetView1(device.GetDevice().Get(), pRenderTargetTexture.Get(), &pTextureRT);

    pTextureRTView.Attach(rtvTexture);

    std::array<ID3D11RenderTargetView*, 2> rtViews = { defaultRTView.Get(), pTextureRTView.Get() };

    // Depth Stencil //
    ComPtr<ID3D11DepthStencilView> dsView;
    auto dsResult = device.CreateDepthStencilViewForSwapchain(defaultRTView.Get(), &dsView);
    if (FAILED(dsResult))
        return -3;

    CD3D11_DEPTH_STENCIL_DESC defaultDepthDesc(CD3D11_DEFAULT{});
    ComPtr<ID3D11DepthStencilState> defaultState;
    auto dss = CreateDepthStencilState(device.GetDevice().Get(), defaultDepthDesc).value();
    defaultState.Attach(dss);
    HRESULT result;
    // Blend State
    ComPtr<ID3D11BlendState> blendState;
    CD3D11_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    D3D11_RENDER_TARGET_BLEND_DESC rtbd{};
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.RenderTarget[0] = rtbd;
    blendDesc.RenderTarget[1] = rtbd;
    blendDesc.RenderTarget[2] = rtbd;
    blendDesc.RenderTarget[3] = rtbd;
    blendDesc.RenderTarget[4] = rtbd;
    blendDesc.RenderTarget[5] = rtbd;
    blendDesc.RenderTarget[6] = rtbd;
    blendDesc.RenderTarget[7] = rtbd;

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

    // BEGIN SHADER FILE OPERATIONS //
    auto vertexShader = L"VertexShader.cso";
    auto vertexShader2 = L"VertexShader2.cso";
    auto pixelShader = L"PixelShader.cso";
    auto pixelShader2 = L"PixelShader2.cso";
    auto texturePS = L"TextureShader.cso";

    std::array<wchar_t, _MAX_PATH> modulePath{};
    if (!GetModuleFileName(nullptr, modulePath.data(), modulePath.size()))
    {
        throw std::runtime_error("Failed to find module path\n");
    }

    auto path = std::wstring(modulePath.data());
    auto lastOf = path.find_last_of(L"\\");
    path.erase(lastOf);

    auto vsPath = path + L"\\" + vertexShader;
    auto vsPath2 = path + L"\\" + vertexShader2;
    auto psPath = path + L"\\" + pixelShader;
    auto psPath2 = path + L"\\" + pixelShader2;
    auto texPSPath = path + L"\\" + texturePS;

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
    // Vertex Shaders //
    std::fstream vsStream{ vsPath, vsStream.in | vsStream.binary };
    auto vsData = readFile(vsStream, vsPath);
    std::fstream vsStream2{ vsPath2, vsStream2.in | vsStream2.binary };
    auto vsData2 = readFile(vsStream2, vsPath2);
    //Pixel Shaders //
    std::fstream psStream(psPath, std::fstream::in | std::fstream::binary);
    auto psData = readFile(psStream, psPath);
    std::fstream psStream2(psPath2, std::fstream::in | std::fstream::binary);
    auto psData2 = readFile(psStream2, psPath2);
    std::fstream texturePSStream(texPSPath, std::fstream::in | std::fstream::binary);
    auto texPSData = readFile(texturePSStream, texPSPath);

    // END SHADER FILE OPERATIONS //

    // Compile Shader Objects //
    ComPtr<ID3D11VertexShader> vsShader;
    ComPtr<ID3D11VertexShader> vsShader2;
    ComPtr<ID3D11PixelShader> psShader;
    ComPtr<ID3D11PixelShader> psShader2;
    ComPtr<ID3D11PixelShader> texPS;

    auto vsResult = CompileVertexShaderFromByteCode(device.GetDevice().Get(), vsData, &vsShader);
    if (FAILED(vsResult))
    {
        ThrowIfFailed(vsResult, "Failed to compile vertex shader!\n");
    }

    vsResult = CompileVertexShaderFromByteCode(device.GetDevice().Get(), vsData2, &vsShader2);
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

    auto psResult3 = CompilePixelShaderFromByteCode(device.GetDevice().Get(), texPSData, &texPS);
    if (FAILED(psResult3))
    {
        ThrowIfFailed(psResult3, "Failed to compile vertex shader!\n");
    }

    LSShaderInputSignature vsSignature;
    //vsSignature.AddElement(SHADER_DATA_TYPE::UINT, "SV_InstanceID");
    vsSignature.AddElement(SHADER_DATA_TYPE::FLOAT4, "POSITION0");
    ShaderElement elementTex = {
        .ShaderData = SHADER_DATA_TYPE::FLOAT2,
        .SemanticName = "TEXCOORD",
        .SemanticIndex = 0,
        .OffsetAligned = D3D11_APPEND_ALIGNED_ELEMENT,
        .InputSlot = 0,
        .InputClass = INPUT_CLASS::VERTEX,
        .InstanceStepRate = 0
    };

    ShaderElement elementTexPad = {
        .ShaderData = SHADER_DATA_TYPE::FLOAT2,
        .SemanticName = "PADDING",
        .SemanticIndex = 0,
        .OffsetAligned = D3D11_APPEND_ALIGNED_ELEMENT,
        .InputSlot = 0,
        .InputClass = INPUT_CLASS::VERTEX,
        .InstanceStepRate = 0
    };

    auto layout = vsSignature.GetInputLayout();
    auto inputPos = BuildFromShaderElements(layout);
    auto valInputs = inputPos.value();
    if (!inputPos)
    {
        throw std::runtime_error("Failed to create input layout from shader elements\n");
    }
    ComPtr<ID3D11InputLayout> pInputLayoutPos;
    if (FAILED(device.CreateInputLayout(valInputs, vsData2, &pInputLayoutPos)))
    {
        throw std::runtime_error("Failed to create input layout from device\n");
    }

    vsSignature.AddElement(elementTex);
    vsSignature.AddElement(elementTexPad);
    auto layout2 = vsSignature.GetInputLayout();
    auto inputPosTex = BuildFromShaderElements(layout2);
    auto valInputs2 = inputPosTex.value();
    ComPtr<ID3D11InputLayout> pInputLayoutPosTex;
    if (FAILED(device.CreateInputLayout(valInputs2, vsData, &pInputLayoutPosTex)))
    {
        throw std::runtime_error("Failed to create input layout from device\n");
    }
    //TODO: Implement missing setups below that don't have an appropriate function call

    ComPtr<ID3D11Buffer> vertexBuffer;
    CD3D11_BUFFER_DESC bufferDesc(sizeof(float) * g_positions.size(), D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA subData{ .pSysMem = g_positions.data(), .SysMemPitch = 0, .SysMemSlicePitch = 0 };
    result = device.CreateBuffer(&bufferDesc, &subData, &vertexBuffer);
    if (FAILED(result))
    {
        return -2;
    }

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
    D3D11_SUBRESOURCE_DATA viewSRD{}, projSRD{}, modelSRD{}, indexSRD{}, color1SRD{}, color2SRD{};
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

    // Deferred Contexts //
    BindVS(context1.Get(), vsShader.Get());
    //BindVS(context2.Get(), vsShader.Get());

    BindPS(context1.Get(), psShader.Get());
    //BindPS(context2.Get(), texPS.Get());
    //context1->PSSetShaderResources(0, 1, pRTShaderResView.GetAddressOf());
    //context2->PSSetShaderResources(0, 1, pRTShaderResView.GetAddressOf());
    //context1->PSSetSamplers(0, 1, pSampler.GetAddressOf());
    //context2->PSSetSamplers(0, 1, pSampler.GetAddressOf());

    SetInputlayout(context1.Get(), pInputLayoutPosTex.Get());
    //SetInputlayout(context2.Get(), pInputLayoutPosTex.Get());

    std::array<ID3D11Buffer*, 4> buffers{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get(),
        color1Buffer.Get() };

    std::array<ID3D11Buffer*, 4> buffers2{ viewBuffer.Get(), projBuffer.Get(), modelBuffer.Get(),
        color2Buffer.Get() };

    BindVSConstantBuffers(context1.Get(), 0, buffers);
    //BindVSConstantBuffers(context2.Get(), 0, buffers2);

    UINT stride = sizeof(float) * 8;

    SetVertexBuffers(context1.Get(), vertexBuffer.Get(), 1, 0, stride);
    SetIndexBuffer(context1.Get(), indexBuffer.Get());
    SetRasterizerState(context1.Get(), rsSolid.Get());

    /*SetVertexBuffers(context2.Get(), vertexBuffer.Get(), 1, 0, stride);
    SetIndexBuffer(context2.Get(), indexBuffer.Get());
    SetRasterizerState(context2.Get(), rsSolid.Get());
    */
    FLOAT color[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

    SetBlendState(context1.Get(), blendState.Get());
    SetDepthStencilState(context1.Get(), defaultState.Get(), 1);
    SetTopology(context1.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetViewport(context1.Get(),
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight())
    );

    /*SetBlendState(context2.Get(), blendState.Get());
    SetDepthStencilState(context2.Get(), defaultState.Get(), 1);
    SetTopology(context2.Get(), D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetViewport(context2.Get(),
        static_cast<float>(window->GetWidth()),
        static_cast<float>(window->GetHeight())
    );*/
    /*SetViewport(context2.Get(),
        static_cast<float>(window->GetWidth() / 2.f),
        static_cast<float>(window->GetHeight() / 2.f)
    );*/
    // 1st Pass - Use Texture as Render Target // 
    SetRenderTarget(context1.Get(), pTextureRTView.Get(), dsView.Get());
    ClearRT(context1.Get(), pTextureRTView.Get(), g_color3);
    ClearDS(context1.Get(), dsView.Get());
    DrawIndexed(context1.Get(), g_indices.size(), 0, 0);

    // 2nd Pass - Use default Render Target, and bind Texture as resource //
    BindVSConstantBuffer(context1.Get(), 3, color2Buffer.Get());
    BindPS(context1.Get(), psShader2.Get());
    SetRenderTarget(context1.Get(), defaultRTView.Get(), dsView.Get());
    ClearRT(context1.Get(), defaultRTView.Get(), g_color3);
    ClearDS(context1.Get(), dsView.Get());
    context1->PSSetShaderResources(0, 1, pRTShaderResView.GetAddressOf());
    context1->PSSetSamplers(0, 1, pSampler.GetAddressOf());
    DrawIndexed(context1.Get(), g_indices.size(), 0, 0);

    /*SetRenderTarget(context2.Get(), pTextureRTView.Get(), dsView.Get());
    ClearRT(context2.Get(), pTextureRTView.Get(), g_color);
    ClearDS(context2.Get(), dsView.Get());
    DrawIndexed(context2.Get(), g_indices.size(), 0, 0);
    UnbindRenderTarget(context2.Get());
    context2->PSSetShaderResources(0, 1, pRTShaderResView.GetAddressOf());
    context2->PSSetSamplers(0, 1, pSampler.GetAddressOf());
    DrawIndexed(context2.Get(), g_indices.size(), 0, 0);*/
    // Obtain render target buffer and set as texture //
    // Create Render Texture (texture to write to) //
    //D3D11_TEXTURE2D_DESC renderTD{};
    //DXGI_SWAP_CHAIN_DESC swapchainDesc;
    //auto swapDesc = device.GetSwapChain()->GetDesc(&swapchainDesc);
    //renderTD.Width = swapchainDesc.BufferDesc.Width;
    //renderTD.Height = swapchainDesc.BufferDesc.Height;
    //renderTD.ArraySize = 1;
    //renderTD.BindFlags = 0;
    //renderTD.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //renderTD.Format = swapchainDesc.BufferDesc.Format;
    //renderTD.MipLevels = 1;
    //renderTD.MiscFlags = 0;
    //renderTD.Usage = D3D11_USAGE_STAGING;
    //renderTD.SampleDesc = swapchainDesc.SampleDesc;

    //ComPtr<ID3D11Texture2D> pStagingTexture;
    //texResult = device.GetDevice()->CreateTexture2D(&renderTD, nullptr, &pStagingTexture);
    //if (FAILED(texResult))
    //    return -50;

    //// Back buffer - we cannot read from the back buffer, need to create a texture to read from //
    //ComPtr<ID3D11Texture2D> pSourceBB;
    //auto bbResult = device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&pSourceBB));
    //if (FAILED(bbResult))
    //    return -88;

    //D3D11_TEXTURE2D_DESC backbuffDesc;
    //pSourceBB->GetDesc(&backbuffDesc);

    //backbuffDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    //backbuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    //backbuffDesc.Usage = D3D11_USAGE_DEFAULT;

    //bbResult = device.GetDevice()->CreateTexture2D(&backbuffDesc, nullptr, &pSourceBB);
    //if (FAILED(bbResult))
    //    return -89;

    //context2->CopyResource(pStagingTexture.Get(), pSourceBB.Get());

    ComPtr<ID3D11CommandList> command1, command2;
    auto hr = context1->FinishCommandList(FALSE, &command1);
    if (FAILED(hr))
        return EXIT_FAILURE;

    /*hr = context2->FinishCommandList(FALSE, &command2);
    if (FAILED(hr))
        return EXIT_FAILURE;*/

        // Show Window and Start Timer - duh... //
    window->Show();
    g_timer.Start();
    while (window->IsRunning())
    {
        g_timer.Tick();
        if (command1)
        {
            immContext->ExecuteCommandList(command1.Get(), false);
        }
        /*if (command2)
        {
            immContext->ExecuteCommandList(command2.Get(), true);
        }*/
        Present1(device.GetSwapChain().Get(), 1);
        window->PollEvent();
    }
}