## CSGForge-Core

CSGForge-Core is a library for converting a temporal [Constructive Solid Geometry](https://en.wikipedia.org/wiki/Constructive_solid_geometry) model into polygons for rendering or use as a general 3D model. This library serves as the backend for the [ForgeCSG Editor](https://github.com/CSGForge/ForgeCSG-Editor).

## Getting Started

This repository uses (Premake)[https://premake.github.io/] to generate a build system. The following instructions assume you're also using Premake. To clone the repository (including submodules) run:
```sh
git clone --recurse-submodules https://github.com/CSGForge/CSGForge-Core.git
```

To link this library into your own Premake project you must [include](https://premake.github.io/docs/include/) this repositories `premake.lua`, add it's [includedir](https://premake.github.io/docs/includedirs/) to your project definition, and [link](https://premake.github.io/docs/Linking/) the library. Example:

```
workspace "MyWorkspace"
    startproject "MyProject"
    configurations {"Debug", "Release"}
    
    filter "configurations:Release"
        defines "NDEBUG"
        optimize "FULL"
    
    filter "configurations:Debug"
        defines "DEBUG"
        optimize "Debug"
        symbols "On"

project "MyProject"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    
    includedirs {
        "libs/CSGForge-Core/include",
        "libs/CSGForge-Core/libs/cbop",
        "libs/CSGForge-Core/libs/glm"
    }
    files {"src/**.hpp", "src/**.cpp"}
    links {"CSGForge-Core", "glm", "cbop"}

filter {}
include "libs/CSGForge-Core"
include "libs/CSGForge-Core/libs/cbop.lua"
include "libs/CSGForge-Core/libs/glm.lua"
```

## Usage

```cpp
#include <vector>
#include <glm/glm.hpp>
#include <CSGForge-Core/csg.hpp>

// Helper function to create a plane from a point and a normal
static ForgeCore::Plane make_plane(const glm::vec3 &point, const glm::vec3 &normal)
{
    return ForgeCore::Plane(normal, -glm::dot(point, normal));
}

int main(int argc, char *argv[])
{
    // Basic planes to define a unit cube
    std::vector<ForgeCore::Plane> cube_planes = {
        make_plane(glm::vec3(+1, 0, 0), glm::vec3(+1, 0, 0)),
        make_plane(glm::vec3(-1, 0, 0), glm::vec3(-1, 0, 0)),
        make_plane(glm::vec3(0, +1, 0), glm::vec3(0, +1, 0)),
        make_plane(glm::vec3(0, -1, 0), glm::vec3(0, -1, 0)),
        make_plane(glm::vec3(0, 0, +1), glm::vec3(0, 0, +1)),
        make_plane(glm::vec3(0, 0, -1), glm::vec3(0, 0, -1)),
    };

    // Create a CSG world
    // Worlds can be infinite Air or infinite Solid
    auto world = new ForgeCore::World();
    world->SetWorldType(ForgeCore::AIR_WORLD);

    // Here we create a new world brush and set it's planes
    auto b1 = world->AddBrush();
    b1->SetPlanes(cube_planes);

    // A little more fancy. We make another brush, but this time it's subtractive and
    // it's X-axis scale is halved.
    auto b2 = world->AddBrush();
    b2->SetOperation(ForgeCore::SUBTRACTION);
    b2->SetPlanes(cube_planes);
    b2->SetTransform(ForgeCore::Transform(glm::vec3(0.5f, 1.f, 1.f), glm::vec3(0), glm::vec3(0)));

    // Updating the world performs CSG on any brushes that require updating and returns
    // the set of brushes that were updated
    auto updated = world->Update();
    return EXIT_SUCCESS;
}
```

## License

Distributed under the MIT License. See `LICENSE` for more information.
