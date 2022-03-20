#pragma once

#include <vector>
#include <set>

#include <glm/glm.hpp>

#include "Plane.hpp"
#include "Vertex.hpp"

namespace ForgeCore
{
    struct Region;

    class Face
    {
    public:
        Face(Plane *plane);
        ~Face() = default;

        void SetVertices(std::vector<Vertex> vertices);
        std::vector<Vertex> GetVertices();
        std::vector<unsigned int> GetIndices();
        std::vector<Face> GetNeighbourFaces();
        Plane GetPlane();
        void SetRegions(std::vector<Region> regions);
        std::vector<Region> GetRegions();
        void Triangulate();

    private:
        Plane *mPlane;
        std::vector<Face> mNeighbours;
        std::vector<Vertex> mVertices;
        std::vector<unsigned int> mIndices;
        std::vector<Region> mRegions;
    };
}