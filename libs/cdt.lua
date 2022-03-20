project "CDT"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"

    includedirs {"CDT/CDT/include"}
    files {
        "CDT/CDT/src/CDT.cpp",
        "CDT/CDT/include/**",
        "CDT/CDT/extras/**"
    }