#pragma once

#include <glm/glm.hpp>

namespace ForgeCore
{
    class Plane
    {
    public:
        Plane() = default;
        ~Plane() = default;

        glm::vec3 mNormal;
        float mOffset;

        void Transform(glm::mat4 transformation);
    };
}