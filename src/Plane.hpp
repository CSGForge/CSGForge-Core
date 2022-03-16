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

        // Transformed plane
        glm::vec3 mNormal;
        float mOffset;

        // Base Plane
        glm::vec3 mBaseNormal;
        float mBaseOffset;

        void Transform(glm::mat4 transformation);
        bool FindIntersectionPoint(Plane p1, Plane p2, glm::vec3 &vertexPosition);
        bool operator==(const Plane &p) const;
    };
}