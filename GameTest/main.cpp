#define LS_WINDOWS_BUILD
#include "LSEFramework.h"

import Platform.Win32Window;
import D3D11.Device;
import D3D11.RenderD3D11;
import D3D11.HelperStates;
import Engine.LSTimer;

using namespace LS;
using namespace Microsoft::WRL;
using namespace std::chrono;
using namespace std::chrono_literals;

LS::LSTimer<uint64_t, 1, 1000> g_timer;
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
    LSSwapchainInfo swapchain{
        .BufferSize = 2,
        .Width = window->GetWidth(),
        .Height = window->GetHeight(),
        .PixelFormat = LS::PIXEL_FORMAT::RGBA_8,
        .IsStereoScopic = false,
        .MSCount = 1,
        .MSQuality = 0
    };
    device.CreateSwapchain(reinterpret_cast<HWND>(window->GetHandleToWindow()), swapchain);
    // Draw States //
    LSDrawState solid{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    LSDrawState wireframe{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    auto rsSolidOptional = Win32::CreateRasterizerState2(device.GetDevice().Get(), solid);
    auto rsWireframeOptional = Win32::CreateRasterizerState2(device.GetDevice().Get(), wireframe);

    ComPtr<ID3D11RasterizerState2> rsSolid;
    rsSolid.Attach(rsSolidOptional.value_or(nullptr));
    ComPtr<ID3D11RasterizerState2> rsWireframe;
    rsWireframe.Attach(rsWireframeOptional.value_or(nullptr));

    // Render Target //
    ComPtr<ID3D11Texture2D> buffer;
    device.GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&buffer));
    ID3D11RenderTargetView1* rtView = Win32::CreateRenderTargetView1(device.GetDevice().Get(), buffer.Get());

    ComPtr<ID3D11DeviceContext> pDeferredContext;
    auto result = device.CreateDeferredContext(pDeferredContext.ReleaseAndGetAddressOf());
    if (FAILED(result))
        return -1;

    
    ComPtr<ID3D11CommandList> pCommandList;
    pDeferredContext->ClearRenderTargetView(rtView, g_color.data());
    pDeferredContext->FinishCommandList(false, pCommandList.ReleaseAndGetAddressOf());

    window->Show();
    g_timer.Start();
    while (window->IsRunning())
    {
        g_timer.Tick();
        LS::Win32::ClearRT(device.GetImmediateContext().Get(), rtView, g_color);
        //device.GetImmediateContext()->ExecuteCommandList(pCommandList.Get(), false);
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
    Utils::ComRelease(&rtView);
}