#pragma once

#include <vector>
#include <set>

#include <glm/glm.hpp>

#include "Plane.hpp"
#include "Vertex.hpp"

namespace ForgeCore
{
    struct Region;
    class Brush;

    class Face
    {
    public:
        Face(Brush *brush, Plane *plane);
        ~Face() = default;

        void SetVertices(std::vector<Vertex> vertices);
        std::vector<Vertex> GetVertices();
        std::vector<glm::vec3> GetTriangleVertices();
        std::vector<unsigned int> GetIndices();
        std::vector<Face> GetNeighbourFaces();
        Plane GetPlane();
        void SetRegions(std::vector<Region> regions);
        std::vector<Region> GetRegions();
        void Triangulate();

    private:
        Brush *mBrush;
        Plane *mPlane;
        std::vector<Face> mNeighbours;
        std::vector<Vertex> mVertices;
        std::vector<glm::vec3> mTriangleVertices;
        std::vector<unsigned int> mIndices;
        std::vector<Region> mRegions;
    };
}