#pragma once

#include <glm/glm.hpp>

namespace ForgeCore
{
    class Face;

    struct Vertex
    {
        glm::vec3 mPosition;
        std::vector<Face *> mFaces;
    };
}