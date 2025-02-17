# LUNA SOL GAME ENGINE

Welcome to my game engine: Luna Sol! I'm working to develop a small 3D game engine (maybe 2D variant to come later?) for my personal hobbies using what I know at this time, then expanding into unknown territory. I hope to develop a game over time using this and learn more about game development and game engines in the process. 

Short term plans this project will be to develop a few simple games:
* 2D and shader simple demos
* 3D basic systems (scene, input, physics)
* UI handling systems

Most of my focus with this engine will be predmoninantly based on 3D graphics. My goals with the engine are to focus on small, light games for now, and expand over time as I learn. I would like to learn more about software development and utilize this engine for both desktop and mobile. As I have more experience with Direct3D 11, I'm going to start with that, and learn other APIs in the background while working on this engine.

I am also planning to make some tutorial videos on how I constructed this all over time 😅

# Visual Studio and VCPKG
To get started, follow the directions from [MSDN](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild?pivots=shell-powershell). 
Step 1 should be all you need if you've already got Visual Studio set up.

1. Simply clone: `git clone https://github.com/microsoft/vcpkg.git`
1. Run the bootstrap: `cd vcpkg; .\bootstrap-vcpkg.bat`
1. Integrate install: `.\vcpkg.exe integrate install`

# Tools and Requirements

This project plans to use and build with C++ modules. I've only currently tested on MSVC but am looking to move towards
building with other compilers too. I for now am setting up the project structure to be a bit less reliant on Visual Studio solutions
and moving towards making CMAKE builds a thing as well. Given that, right now, this project is expected to build best with
Visual Studio and MSVC for the time being. 

I have removed the use of Nuget and now am relying on vcpkg as my package manager due to its integration with CMAKE and Visual Studio 
being what I am most familiar with. 

## Building with CMAKE and VCPKG - Where to Get/Minimum Version support 
- Requires CMake 3.28 [CMake](https://cmake.org/)
- Requires Vcpkg [Github](https://github.com/microsoft/vcpkg)  

# CMake Integration (Partial)
As mentioned above, only the LunaSolProject will build. I use Vcpkg as my package manager. To build you'll need both. 
Follow the directions on how to install to your system before moving forward.

## Vcpkg - First Things
Vcpkg is used in manifest mode here, which is what we'll go with. This requires you to make a `CMakeUsersPreset.json` file 
inside `{RootDir}/LunaSolGameEngine`

A sample below is given to show what is needed:
```
{
    "version": 8,
    "configurePresets": [
        {
            "name": "windows-x64-debug",
            "description": "Builds Windows X64 in debug mode",
            "inherits": [
                "windows-x64"
            ],
            "environment": {
                "VCPKG_ROOT": "path/to/vcpkg"
            },
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
                "VCPKG_INSTALLED_DIR": "{path/to/vcpkg_installed}",
                "VCPKG_MANIFEST_DIR": "{path/to/vcpkg.json}"
            }
        }
    ]
}
```

If you copy and paste the above, just simply set the variables:
1. VCPKG_ROOT - directory to where you have your vcpkg downloaded and where the vcpkg.exe is located
1. VCPKG_INSTALLED_DIR - This is where you have your installs. If you're using manfiest and want to install locally, omit this, or give the path to where you installed your libraries. 
1. VCPKG_MANIFEST_DIR - Path to the `vcpkg.jsgon` this is located in the project's root directory for now, because it's shared with GameTest project. So `../` should suffice for now.

## CMAKE Build 
This only builds with MSVC right now and with Windows x64. I'm working to expand later when I'll try to test and build this with GCC/Clang later.
After all is set and done with the `CMakeUserPresets.json` you should be able to run cmake as follows:

1. Open your shell to the `{RootDir}/LunaSolEngine` where the top level `CMakeLists.txt` file should be for this project
1. Create a folder named `build` in LunaSolEngine folder - `{RootDir}/LunaSolEngine/build` (take care to note this project will be LunaSolEngine as root, it so it should be LunaSolEngine/LunaSolEngine/build)
1. From `{RootDir}LunaSolEngine` Run `cmake -B build -S . --preset windows-x64-debug`
1. From `{RootDir}LunaSolEngine`  Run `cmake --build build`

Hopefully everything works out and this builds!

## C++ 20 Baseline - Using Modules
I am working on building this project utilizing modules, especially for the main library. Demos and 
other tests may use the traditional file formats, but overall I am not focused on maintaining specific rules
for test projects in how they're configured or built, just what they demonstrate. For now, files with the `.ixx` extension
are the module files, due to them being the defaulted name when creating them in Visual Studio. I may opt to name them in the 
future something else, but for now this is not going to change.

With modules, I am going to work on a few rules for the following below:

* Named Modules `e.g. export module foobar` - These will be both the name of the file and the module itself. 
* Module Partition `e.g. export module foobar:bah` - I'm not expecting to use these much, but rules would be dot seperator for any partitions so the example would be Foobar.Bah.ixx
* Private Module Fragments `module : private;` - Support for this isn't there in all compilers, and I may move from using this, but for now they exist within the same file. I do not seperate implementations in a seperate file.
* Global Module Fragment - Used for `#include` and other preprocessor statements.

### Goals for Module Usage

I'm not planning to utilize header units currently. It seems to be a bit unreliable and floods the output on MSVC right now (while also not being widely supported) and overall I just don't know if there's a usage for it right now. I plan to roll many of my own modules for everything in this library. My goals and hopes for it are not about potential future benefits when build tools improve but to:
1. Improve clarity and understanding of each module and its purpose in the design
1. Supplement the use of partitions to hopefully break up systems into more smaller chunks that build into a larger system for easier maintainability
1. Utilize private module fragments to reduce bloat of code files. (Simply put no h/cpp separation especially if I feel it's "small enough" to implement.)

Currently I'm using the `.ixx` extension because I'm using Visual Studio as my main development tool. This is because MSVC seems farthest along in support with C++ 20 modules. However, it also is very slow and prone to breaking because of these changes. Over the several updates, I've noticed improvements, so here is hoping it continues to improve for the better. 🤞🏼

### Module Partitions 

As for module partitions, I currently also use them in the `.ixx` file extension as well. I've seen reason to also include them into `.cpp` or maybe something else. I haven't quite yet determined how I want to continue with that, and so this could change later down the line. 

#### Global Module Fragments

Due to not going with Header Units right now, that means we'll need to use the **Global Module Fragment** in our code. This looks like the following
```cpp
module;
#include <vector> // normal std libraries
#include "my_header.h"// headers of my own
#define NECESSARY_DEFINES // like those needed for Windows.h
export module foo;
```

This is how I plan to support includdes in my modules, and I won't be making use of a precompiled header in this project as well. This is because a precompiled header has been a bit wonky to use, or maybe I just lack experience still. I will revisit this down the road if I ever find out how and see improvements to using it. 

### Module Naming Scheme
When I started, I started with just using a dot notation as my separator and grouping them into categories. `i.e. Data.*** and Math.***` and while that seemed fine overall. I started taking a liking more to partitions as a means of a collective group of modules that provide some functionality within a set of features. `i.e. D3D12Lib that contains various partitions (:) for setting up device, command lists, command queues, etc.`

For now, all files like everything else are **UpperCamelCase** and that will not change. Module files will still respect that, but listed down below are how I will declare and refer to the modules with the `export module [name]` declaration.

I had previously gone working around various naming schemes, but now I'm going to settle on a particular case and hopefully I could refactor with little trouble down the road. 

For now, the modules themselves will match the module name files. In other words the file names and module declartions should always match. 

**RIGHT**

**MyLib.ixx**
```cpp
export module MyLib;
```

**WRONG**

**MyLib.ixx** 
```cpp
export module MyFooBar; // file name and module name differ
```

### To Partition or Not Partition ##
Simply put I would like to utilize partitions a little more than none-partition files. I believe when dealing with larger systems that could bloat a single file, it would be nice to break them up. That way all I need to do is insure that my one module file exports all imports of the partition and then users can simply import that one module file to utilize. 

```cpp
export module D3D12Lib;
export import :Device;
export import :CommandLists;
export import :CommandQueue;
//etc. 
```

The instances where that doesn't make sense, I'll create modules with declarations and utilize private module fragments instead of declaring the changes inline. I don't know if this comes with any sort of compilation benefits, though in theory it should if tooling could ever get there. However, for now I want to keep the files clean with declarations above the private module fragment and implementations within the private module fragment. 

```cpp
export module Foo

export void MyFoo(); // declaration

module : private; // private module fragment

export void MyFoo()
{
    // implementation here
}
```

When doing partitions, these private module fragments cannot happen within partitioned segments. This unfortunately would lead me to having to declare and implement the private module fragment WITHIN the primary module interface. That means I would have to implement the details in one file, which I do not want to do. That's why I know it may not be optimal in terms of compilation, and maybe down the line an issue, but I'm going this route more for maintainability and to reduce bloat in a single file.

**Note: I plan to move this section later into another README group, but for now just dropping my thoughts here**