#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "AABB.hpp"
#include "Plane.hpp"
#include "Face.hpp"
#include "Vertex.hpp"

namespace ForgeCore
{
    class World;

    class Brush
    {
    public:
        Brush(World *world);
        ~Brush() = default;

        void RebuildFaces();
        void RebuildRegions();
        std::vector<Brush *> RebuildIntersections(std::vector<Brush *> brushes);
        AABB GetAABB();
        void SetPlanes(std::vector<Plane> planes);
        std::vector<Plane> GetPlanes();
        std::vector<Face> GetFaces();
        std::vector<Vertex> GetVertices();
        std::vector<Brush *> GetIntersections();
        bool PointInPlanes(glm::vec3 point);
        void AddIntersection(Brush *brush);
        void RemoveIntersection(Brush *brush);
        void Triangulate();

    private:
        World *mWorld;
        glm::vec3 mPivot;
        AABB mBoundingBox;
        std::vector<Vertex> mVertices;
        std::vector<Plane> mPlanes;
        std::vector<Face> mFaces;
        std::vector<Brush *> mIntersections;
    };
}