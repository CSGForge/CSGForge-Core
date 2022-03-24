#pragma once

#include <vector>
#include <set>

#include <glm/glm.hpp>
#include <cbop/polygon.h>

#include "AABB.hpp"
#include "Plane.hpp"
#include "Face.hpp"
#include "Vertex.hpp"

namespace ForgeCore
{
    class World;

    struct Transform
    {
        glm::vec3 mTranslation;
        glm::vec3 mScale;
        glm::vec3 mRotation;

        Transform()
        {
            mScale = glm::vec3(1);
            mRotation = glm::vec3(0);
            mTranslation = glm::vec3(0);
        }

        Transform(glm::vec3 scale, glm::vec3 rotation, glm::vec3 translation)
        {
            mScale = scale;
            mRotation = rotation;
            mTranslation = translation;
        }
    };

    enum Operation
    {
        ADDITION,
        SUBTRACTION
    };

    enum RCategory
    {
        INSIDE,
        ALIGNED,
        REVERSE_ALIGNED,
        OUTSIDE
    };

    class Brush
    {
    public:
        Brush(World *world);
        ~Brush() = default;

        void RebuildFaces();
        void RebuildRegions();
        void RebuildIntersections();
        AABB GetAABB();
        World *GetWorld();
        void SetOperation(Operation operation);
        Operation GetOperation();
        void SetPlanes(std::vector<Plane> planes);
        std::vector<Plane> GetPlanes();
        std::vector<Face> GetFaces();
        std::vector<Vertex> GetVertices();
        std::set<Brush *> GetIntersections();
        void SetTransform(Transform transform);
        Transform GetTransform();
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
        std::set<Brush *> mIntersections;
        Operation mOperation;
        Transform mTransform;
        glm::mat4 mTransformMatrix;
    };

    struct Region
    {
        Brush *mBrush;
        RCategory mCategory;
        cbop::Polygon mPolygon;
    };
}