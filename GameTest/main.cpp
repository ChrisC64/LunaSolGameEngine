#include "LSTimer.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include "engine/EngineLogDefines.h"
#include <format>
#include <cstdint>
#include <bitset>

#ifndef _DEBUG
#include <Windows.h>
#endif // 

#define LS_ENABLE_LOG 1
import Engine.Logger;
import Engine.EngineCodes;
import DX11CubeApp;
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
    auto appcode = gt::App->Initialize(argc, argv);
    if (!LS::IsSuccessCode(appcode))
        return -1;
    gt::App->Run();
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