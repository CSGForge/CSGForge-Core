#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "AABB.hpp"
#include "Plane.hpp"
#include "Face.hpp"

namespace ForgeCore
{
    class Brush
    {
    public:
        Brush() = default;
        ~Brush() = default;

        bool IsDirty();
        void SetClean();
        void RebuildFaces();
        std::vector<Brush *> RebuildIntersections(std::vector<Brush *> brushes);
        AABB GetAABB();
        void SetPlanes(std::vector<Plane> planes);
        std::vector<Plane> GetPlanes();
        std::vector<Face> GetFaces();
        std::vector<glm::vec3> GetVertices();
        std::vector<Brush *> GetIntersections();
        bool PointInPlanes(glm::vec3 point);
        void AddIntersection(Brush *brush);
        void RemoveIntersection(Brush *brush);

    private:
        bool mDirty;
        glm::vec3 mPivot;
        AABB mBoundingBox;
        std::vector<glm::vec3> mVertices;
        std::vector<Plane> mPlanes;
        std::vector<Face> mFaces;
        std::vector<Brush *> mIntersections;
    };
}