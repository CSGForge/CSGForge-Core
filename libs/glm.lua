project "glm"
    kind "StaticLib"
    language "C"

    includedirs {"glm"}
    files {"glm/glm/**"}

    filter "system:linux"
        defines {"_GLM_X11"}
    filter "system:windows"
        defines {"_GLM_WIN32", "_CRT_SECURE_NO_WARNINGS"}