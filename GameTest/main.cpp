#define LS_WINDOWS_BUILD
#include "LSEFramework.h"
#include "LSTimer.h"

import Engine.Common;
import D3D11Lib;
import Win32Lib;

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

    auto rsSolidOptional = Win32::CreateRasterizerState2(device.GetDevice().Get(), LS::SolidFill_Front_CCW);
    auto rsWireframeOptional = Win32::CreateRasterizerState2(device.GetDevice().Get(), LS::Wireframe_Back_CCW);

    ComPtr<ID3D11RasterizerState2> rsSolid;
    rsSolid.Attach(rsSolidOptional.value_or(nullptr));
    ComPtr<ID3D11RasterizerState2> rsWireframe;
    rsWireframe.Attach(rsWireframeOptional.value_or(nullptr));

    // Render Target //
    ComPtr<ID3D11Texture2D> buffer;
    device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer));
    ComPtr< ID3D11RenderTargetView1> rtView;
    rtView.Attach(Win32::CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get()));
    ComPtr<ID3D11DeviceContext> pDeferredContext;
    auto result = device.CreateDeferredContext(pDeferredContext.ReleaseAndGetAddressOf());
    if (FAILED(result))
        return -1;
    /*
    ComPtr<ID3D11CommandList> pCommandList;
    pDeferredContext->ClearRenderTargetView(rtView.Get(), g_color.data());
    pDeferredContext->FinishCommandList(false, pCommandList.ReleaseAndGetAddressOf());*/
    
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

    std::vector<std::byte> psData(sizePS);
    psStream.read(reinterpret_cast<char*>(psData.data()), psData.size());
    //Shader profile 5.1 is introduced in D3D12, so we need to make sure the shaders are compiled at 5.0 profile
    ID3D11VertexShader* pVSShader;
    auto vsResult = LS::Win32::CompileVertexShaderFromByteCode(device.GetDevice().Get(), vsData, &pVSShader);
    if (FAILED(vsResult))
    {
        LS::Utils::ThrowIfFailed(vsResult, "Failed to compile vertex shader!\n");
    }
    ID3D11PixelShader* pPSShader;
    auto psResult = LS::Win32::CompilePixelShaderFromByteCode(device.GetDevice().Get(), psData, &pPSShader);
    if (FAILED(psResult))
    {
        LS::Utils::ThrowIfFailed(psResult, "Failed to compile vertex shader!\n");
    }

    ComPtr<ID3D11VertexShader> vsShader;
    ComPtr<ID3D11PixelShader> psShader;
    vsShader.Attach(pVSShader);
    psShader.Attach(pPSShader);

    LS::Win32::BindVS(device.GetImmediateContext().Get(), vsShader.Get());
    LS::Win32::BindPS(device.GetImmediateContext().Get(), psShader.Get());
    //TODO: Add the interfaces to create inputlayouts and set them for our shaders.
    //device.GetImmediateContext()->IASetInputLayout();
    vsStream.close();
    psStream.close();
    window->Show();
    g_timer.Start();
    while (window->IsRunning())
    {
        g_timer.Tick();
        LS::Win32::ClearRT(device.GetImmediateContext().Get(), rtView.Get(), g_color);
        LS::Win32::DrawInstances(device.GetImmediateContext().Get(), 0, 3, 0, 0, 0);
        LS::Win32::Present(device.GetSwapChain().Get());
        window->PollEvent();
        if (g_timer.GetTotalTimeTickedIn<std::chrono::seconds>().count() >= 5.0f)
        {
            std::cout << "Time passed: " << g_timer.GetTotalTimeTickedIn<std::chrono::seconds>().count() << "\n";
            g_color[0] = 0.0f;
            g_color[1] = 1.0f;
            g_color[2] = 1.0f;
            g_color[3] = 1.0f;
        }
    }
}