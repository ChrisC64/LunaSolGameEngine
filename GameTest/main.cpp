#include <streambuf>
#include <bitset>
#include <utility>
#include "engine/EngineLogDefines.h"

#ifndef _DEBUG
#include <Windows.h>
#include <processenv.h>
#include <shellapi.h>
#endif // 

import LSEngine;

import <iostream>;
import <fstream>;
import <format>;
import <cstdint>;
import <string>;
import <filesystem>;

import DX11CubeApp;
import CubeApp;
import DX12.SimpleWindow;
import ImGuiWindowTest;
import DearDx11;

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

LS::Ref<LS::LSApp> CreateApp(uint32_t choice)
{
    switch (choice)
    {
    case 0:
    {
        return LS::CreateApp<gt::dx11::DX11CubeApp>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App");
    }
    case 1:
    {
        return LS::CreateApp<gt::dx12::DX12CubeApp>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX12 Cube App");
    }
    case 2:
    {
        return LS::CreateApp<gt::dx12::SimpleWindow>(SCREEN_WIDTH, SCREEN_HEIGHT);
    }
    case 3:
    {
        return LS::CreateApp<gt::dx12::ImGuiSample::ImGuiSample>(SCREEN_WIDTH, SCREEN_HEIGHT);
    }
    case 4:
    {
        return LS::CreateApp<gt::dx11::ImGuiDx11>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 ImGui App");
    }
    default:
    {
        return LS::CreateApp<gt::dx11::DX11CubeApp>(SCREEN_WIDTH, SCREEN_HEIGHT, L"DX11 Cube App");
    }
    }
}

#ifdef _DEBUG
int main(int argc, char* argv[])
{
    std::filesystem::path file = std::filesystem::current_path().string() + "log.txt";
    LS::Log::TraceError(L"Hello logger test!");
    LS::Log::TraceDebug(L"My second test!!");
    LS::Log::TraceWarn(L"WARNING!! Boss approaching!");
    LS::Log::Flush();
    std::cout << "Pick an app:\n0 - DX 11 Cube\n1 - DX12 Cube\n2 - Simple DX12 Window\n3 - ImGui Sample\n4 - DX11 ImGui DemoChoice: ";
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
    app->Run();
}

#else // I know I don't need this, but wondering if maybe I just should consider supporting this?
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    auto app = CreateApp(2);
    auto appcode = app->Initialize(nullptr);
    if (!appcode)
    {
        return WM_QUIT;
    }
    app->Run();
    return WM_QUIT;
}
#endif