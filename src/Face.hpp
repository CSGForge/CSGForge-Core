#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Plane.hpp"

namespace ForgeCore
{
    class Face
    {
    public:
        Face(Plane *plane);
        ~Face() = default;

        void SetVertices(std::vector<glm::vec3> vertices);
        std::vector<glm::vec3> GetVertices();
        std::vector<unsigned int> GetIndices();

    private:
        Plane *mPlane;
        std::vector<glm::vec3> mVertices;
        std::vector<unsigned int> mIndices;
    };
}