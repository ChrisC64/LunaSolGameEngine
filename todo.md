# What is this?
A simple tracking file to keep my todos in order for what I want to accomplish.
This file is my place to deposit my ideas and future things I want to implemment with my stuff.

Below the list should be goals I have in work and hope to complete if I do not return to it right away (which is... common) I'll list recent events in order by date (latest on top) and remove any that have been completed

# DX11 API

* 4/21/24 **(status: In work)**
    * Clean up library management and build system
    * Utilize vcpkg first before other package managers
        * Graphics Libraries: (For DX11 and DX12) DirectX Headers, DirectX ToolKit (TK), DX Texture.
        * Loading Assets: Assimp and GLTF (if Assimp cannot provide)
        * Sound: Still researching but maybe FMOD? 
    * Build System: CMake (I think it's time to give in and learn it), Notable mention: Premake (maybe I can try both...)
 
## DX11 Goals 
1. Create a Render System that allows us to create jobs (deferred) on different threads
1. Provide usage between immediate and deferred rendering
1. Abstract commands in a way to hopefully simplify some of the mundane work of setting up the device and renderer (aim for no more than 2 lines to initialize)
1. Provide tools to get up and running with a scene in 3 lines (clear screen) and cube in 10 lines (will he do it?! Is this important? I guess we'll see...)

# DX 12 API  
I want to perform some similar steps with getting the API of DX12 I have started going. I haven't worked on it in a while, in part because I've had a memory leak that I believe may be fixed. It's hard to say for sure, because I'm still trying to learn the tools I'm using, but I may need to look for different profiles. 

## DX12 Goals
1. Build a DX12 system that can run on my 1070 GTX (current PC) so I'll be limited
1. Build a DX12 system I can use for tutorials later to share with online
1. Learn about debugging and testing DX12 
1. Learn how to profile with DX12
1. Finally, make a simple game with DX12

# Engine Goals
- Provide a framework that can do the following:  
    * Create A Renderer and Display to a Window with few lines of setup
    * Provide framework for simple input systems (keyboard/mice currently, joypad support later)
    * Learn to build and utilize a sound system 
    * Abstract multi-threaded support
    * Use modern C++ (targeting C++20 currently and using modules)
    * Only for Windows right now
    * Utilize experience with DX12 to learn Vulkan (hopefully!)
    * Though DX11 gets support, aim for DX12 too. (I use DX11 when I get stuck or frustrated as I know it better and need to make progress to overcome my frustration)
    * Implement ImGui for reals! (I am liking it, and hope to utilize more)
    * Provide simple physics engine for collision and movement