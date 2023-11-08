#include "LSTimer.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include "engine/EngineLogDefines.h"
#include <format>
#include <cstdint>
#include <bitset>
#include <string>

#ifndef _DEBUG
#include <Windows.h>
#endif // 

import Engine.Logger;
import Engine.EngineCodes;
import DX11CubeApp;
import CubeApp;
import DX12.SimpleWindow;
import Engine.App;
import LSData;
import Helper.LSCommonTypes;

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

Ref<LS::LSApp> CreateApp(uint32_t choice)
{
    switch(choice)
    {
    case 0:
    {
        Ref<LS::LSApp> app(new gt::dx11::DX11CubeApp(800, 600, L"DX11 Cube App"));
        return app;
    }
    case 1:
    {
        //Ref<LS::LSApp> app = std::make_unique<gt::dx12::DX12CubeApp>(800, 600, L"DX12 Cube App");
        Ref<LS::LSApp> app(new gt::dx12::DX12CubeApp(800, 600, L"DX12 Cube App"));
        return app;
    }
    case 2:
    {
        Ref<LS::LSApp> app(new gt::dx12::SimpleWindow(800, 700));
        return app;
    }
    default:
    {
        Ref<LS::LSApp> app(new gt::dx11::DX11CubeApp(800, 600, L"DX11 Cube App"));
        return app;
    }
    }
}

#ifdef _DEBUG
int main(int argc, char* argv[])
{
    /*std::ofstream myFile("ErrorFile.txt", std::ios::binary);
    if (!myFile.is_open())
    {
        std::cerr << "Failed to open file!";
        return -1;
    }
    std::cerr.rdbuf(myFile.rdbuf());

    std::cerr << "An output to file will occur for cerr\n";*/
    LS::Log::InitLog();

    std::cout << "Pick an app:\n0 - DX 11 Cube\n1 - DX12 Cube\n2 - Simple DX12 Window\nChoice: ";
    std::string choice{};
    std::getline(std::cin, choice);
    auto value = std::stoi(choice);

    auto app = CreateApp(value);
    auto appcode = app->Initialize(argc, argv);
    if (!appcode)
    {
        std::cout << appcode.Message() << "\n";
        return -1;
    }
    app->Run();
}

#else // I know I don't need this, but wondering if maybe I just should consider supporting this?
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    LS::Log::TraceError(L"What is this message?!");
    LS::Log::TraceError(std::format(L"A format string appears! {}", 42));
    auto appcode = gt::App->Initialize();
    if (!LS::IsSuccessCode(appcode))
        return -1;

    gt::App->Run();

    return WM_QUIT;
}
#endif