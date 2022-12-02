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

    window->Show();
    g_timer.Start();
    while (window->IsRunning())
    {
        g_timer.Tick();
        LS::Win32::ClearRT(device.GetImmediateContext().Get(), rtView.Get(), g_color);
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