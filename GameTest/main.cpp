#include <streambuf>
#include <bitset>
#include <utility>
#include "engine/EngineLogDefines.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <windowsx.h>
#include <processenv.h>
#include <shellapi.h>

import LSEngine;

import <iostream>;
import <fstream>;
import <format>;
import <cstdint>;
import <string>;
import <filesystem>;

//import DX11CubeApp;
//import CubeApp;
//import DX12.SimpleWindow;
//import ImGuiWindowTest;
//import DearDx11;

//import MultiPassTestApp;
//import CubeApp;
// TODO: Make a cube appear by loading in a .obj or .gltf reader
// TODO: Consider cleaning up the modules a little
// TODO: Add a library module for calls like CreateDevice(DEVICE_API) and CreatePipelineFactory(DEVICE_API)
//  so these can be implemented and more "properly" handled via the library as we might expect
// TODO: Remove the throws and create an "LSExpected" type like C++ 23 Expected type for error handling
// TODO: Examine current setup in main, and consider how now to make things a little easier on the API side to remove
//  some redeundant code or simplify things
// TODO: Start DX12 renderer
// TODO: Start Vulkan renderer
// TODO: Examin needed Interfaces and build them
constexpr uint32_t SCREEN_WIDTH = 1920;
constexpr uint32_t SCREEN_HEIGHT = 1080;
//
//LS::Ref<LS::LSApp> CreateApp(uint32_t choice)
//{
//    switch (choice)
//    {
//    case 0:
//    {
//        return LS::CreateApp<gt::dx11::DX11CubeApp>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App");
//    }
//    case 1:
//    {
//        return LS::CreateApp<gt::dx12::DX12CubeApp>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX12 Cube App");
//    }
//    case 2:
//    {
//        return LS::CreateApp<gt::dx12::SimpleWindow>(SCREEN_WIDTH, SCREEN_HEIGHT);
//    }
//    case 3:
//    {
//        return LS::CreateApp<gt::dx12::ImGuiSample::ImGuiSample>(SCREEN_WIDTH, SCREEN_HEIGHT);
//    }
//    case 4:
//    {
//        return LS::CreateApp<gt::dx11::ImGuiDx11>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 ImGui App");
//    }
//    default:
//    {
//        return LS::CreateApp<gt::dx11::DX11CubeApp>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App");
//    }
//    }
//}

#ifdef _DEBUG
import Platform.Win32App;
int main(int argc, char* argv[])
{
    /*std::filesystem::path file = std::filesystem::current_path().string() + "log.txt";
    LS::Log::TraceError(L"Hello logger test!");
    LS::Log::TraceDebug(L"My second test!!");
    LS::Log::TraceWarn(L"WARNING!! Boss approaching!");
    LS::Log::Flush();
    std::cout << "Pick an app:\n0 - DX 11 Cube\n1 - DX12 Cube\n2 - Simple DX12 Window\n3 - ImGui Sample\n4 - DX11 ImGui \nDemo Choice: ";
    std::string choice{};
    std::getline(std::cin, choice);
    auto value = std::stoi(choice);

    auto app = CreateApp(value);

    auto args = LS::ParseCommands(argc, argv);
    auto appcode = app->Initialize(args);
    if (!appcode)
    {
        std::cout << appcode.Message() << "\n";
        return -1;
    }
    app->Run();*/
    using namespace LS::Win32;
    InitApp(SCREEN_WIDTH, SCREEN_HEIGHT, L"My new app!");
    LS::Platform::Dx12::RendererDX12 renderer(SCREEN_WIDTH, SCREEN_HEIGHT, 2, LS::Win32::GetHwnd());

    SetCustomWndProc([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            switch (msg)
            {
            case WM_LBUTTONUP:
                std::cout << "LMB Released!\n";
                //LS::Win32::ShowMessageBox(L"Hello MB", L"Message Box Approved");
                return (LRESULT)0;
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    );

    RegisterMouseMove([](double x, double y)
        {
            std::cout << std::format("X: {}, Y: {}\n", x, y);
        });
    auto commandList = renderer.CreateCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, "main_cl").value();

    auto renderFrame = [&]()
        {
            //TODO: I've been away for so long, why did I want to separate the renderer and our frame buffer?
            // Either way, this looks weird and makes no sense, do I want to separate some of the objects here?
            // if so, what does the command list want then? What is the goal here?
            // It is to do some commands, and those commands should be on a particular render target (frame buffer)
            // right?
            //const LS::Platform::Dx12::FrameBufferDxgi& framebuffer = renderer.GetFrameBuffer();
            renderer.BeginCommandList(commandList);
            commandList.Clear({ 0.0f, 0.12f, 0.34f, 1.0f });
            renderer.EndCommandList(commandList);
            renderer.QueueCommand(&commandList);
        };
    
    
    while (IsAppRunning())
    {
        PollApp();
        renderer.WaitOnNextFrame();
        renderFrame();
        const auto fence = renderer.ExecuteCommands();
        renderer.WaitForCommands(fence);
    }
    Shutdown();
}

#else // I know I don't need this, but wondering if maybe I just should consider supporting this?
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    using namespace LS::Win32;

    InitApp(1920, 1080, L"My new app!");

    SetCustomWndProc([](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            switch (msg)
            {
            case WM_LBUTTONUP:
                std::cout << "LMB Released!\n";
                //LS::Win32::ShowMessageBox(L"Hello MB", L"Message Box Approved");
                return (LRESULT)0;
            }
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }
    );

    RegisterMouseMove([](double x, double y)
        {
            std::cout << std::format("X: {}, Y: {}\n", x, y);
        });

    while (IsAppRunning())
    {
        PollApp();
    }
    Shutdown();
}
#endif