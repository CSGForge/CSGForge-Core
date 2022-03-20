project "CSGForge-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    includedirs {"include", "libs/glm", "libs/cbop"}
    files {
        "src/**.*"
    }

    links {"glm", "cbop"}

include "libs/glm.lua"
include "libs/cbop.lua"