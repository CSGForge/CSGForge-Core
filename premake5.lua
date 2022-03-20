project "CSGForge-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    includedirs {"include", "libs/glm", "libs/cbop", "libs/CDT/CDT/include"}
    files {
        "src/**.*"
    }

    links {"glm", "cbop", "CDT"}

include "libs/glm.lua"
include "libs/cbop.lua"
include "libs/cdt.lua"