#pragma once

#include <glm/glm.hpp>

namespace ForgeCore
{
    class Plane
    {
    public:
        Plane() = default;
        Plane(glm::vec3 normal, float offset);
        ~Plane() = default;

        glm::vec3 mNormal;
        float mOffset;

        void Transform(glm::mat4 transformation);
        bool FindIntersectionPoint(Plane p1, Plane p2, glm::vec3 &vertexPosition);
        bool operator==(const Plane &p) const;
    };
}