#include "LSEFramework.h"
#include "LSTimer.h"

import MultiPassTestApp;

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
int main(int argc, char* argv[])
{
    gt::App->Initialize(argc, argv);
    gt::App->Run();
}