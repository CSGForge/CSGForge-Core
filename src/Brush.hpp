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
        void RebuildFaces();
        AABB GetAABB();
        void SetPlanes(std::vector<Plane> planes);
        std::vector<Plane> GetPlanes();
        std::vector<Face> GetFaces();
        bool PointInPlanes(glm::vec3 point);

    private:
        bool mDirty;
        glm::vec3 mPivot;
        AABB mBoundingBox;
        std::vector<Plane> mPlanes;
        std::vector<Face> mFaces;
        std::vector<Brush *> mIntersections;
    };
}