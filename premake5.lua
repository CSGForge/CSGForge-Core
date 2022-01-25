project "CSGForge-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    includedirs {"include"}
    files {
        "src/**.*"
    }

    links {"glm"}

include "libs/glm.lua"