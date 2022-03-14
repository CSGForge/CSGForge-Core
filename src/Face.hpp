#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Plane.hpp"
#include "Vertex.hpp"

namespace ForgeCore
{
    class Face
    {
    public:
        Face(Plane *plane);
        ~Face() = default;

        void SetVertices(std::vector<Vertex> vertices);
        std::vector<Vertex> GetVertices();
        std::vector<unsigned int> GetIndices();
        Plane GetPlane();
        void Triangulate();

    private:
        Plane *mPlane;
        std::vector<Vertex> mVertices;
        std::vector<unsigned int> mIndices;
    };
}