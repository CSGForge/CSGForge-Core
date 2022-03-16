#include "Brush.hpp"

#include <set>

#include <stdio.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "World.hpp"

namespace ForgeCore
{
    void PushBackIfUnique(std::vector<Vertex> &vs, Vertex v1, float eps)
    {
        auto v1p = v1.mPosition;
        for (int i = 0; i < vs.size(); i++)
        {
            auto v2 = vs[i];
            auto v2p = v2.mPosition;
            if (std::abs(v1p.x - v2p.x) <= eps && std::abs(v1p.y - v2p.y) <= eps && std::abs(v1p.z - v2p.z) <= eps)
            {
                // Adds any faces that aren't already listed on the vertex
                for (auto face : v1.mFaces)
                    if (*std::find(v2.mFaces.begin(), v2.mFaces.end(), face) != face)
                        vs[i].mFaces.push_back(face);
                return;
            }
        }

        vs.push_back(v1);
    }

    std::vector<Vertex> FindPolygonPath(std::vector<Vertex> unsorted_vs, Plane plane)
    {
        std::vector<Vertex> sorted_vs;

        // Build a path along face edges
        auto current = unsorted_vs.begin();
        while (current != unsorted_vs.end())
        {
            Vertex v1 = *current;
            sorted_vs.push_back(v1);
            unsorted_vs.erase(current);

            // Find me an edge!
            for (int j = 0; j < unsorted_vs.size(); j++)
            {
                auto v2 = unsorted_vs[j];
                int shared_faces = 0;
                for (auto face : v1.mFaces)
                    if (*std::find(v2.mFaces.begin(), v2.mFaces.end(), face) == face)
                        shared_faces++;

                // Edges only exist between two vertices if they share two faces
                if (shared_faces == 2)
                {
                    current = unsorted_vs.begin() + j;
                    break;
                }
            }
        }

        // Winding order of the path may be wrong so we need to reverse it
        glm::vec3 v0 = sorted_vs[0].mPosition;
        glm::vec3 v1 = sorted_vs[1].mPosition;
        glm::vec3 v2 = sorted_vs[2].mPosition;
        if (glm::dot(glm::cross(v1 - v0, v2 - v0), plane.mNormal) < 0)
            std::reverse(sorted_vs.begin(), sorted_vs.end());

        return sorted_vs;
    }

    Brush::Brush(World *world)
    {
        mWorld = world;
        mTransform = Transform();
        mTransformMatrix = glm::mat4(1);
    }

    void Brush::RebuildFaces()
    {
        // TODO: Simplify this function somehow. It's over 100 lines long
        // TODO: Check that there's 4+ planes
        // Clear vertices for each face
        int n = mPlanes.size();
        std::vector<std::vector<Vertex>> face_vertices;

        // TODO: Vertices are duplicated for each face right now! Use pointers to avoid this!
        mFaces.clear();
        for (int i = 0; i < n; i++)
        {
            std::vector<Vertex> v;
            mFaces.push_back(Face(&mPlanes[i]));
            face_vertices.push_back(v);
        }

        // Calculate vertices
        std::vector<Vertex> brush_vertices;
        for (int i = 0; i < n - 2; i++)
        {
            for (int j = i + 1; j < n - 1; j++)
            {
                for (int k = j + 1; k < n; k++)
                {
                    glm::vec3 vertex_pos(0);
                    if (!mPlanes[i].FindIntersectionPoint(mPlanes[j], mPlanes[k], vertex_pos))
                        continue;

                    // Check that the found vertex isn't outside of any of the planes
                    if (PointInPlanes(vertex_pos))
                    {
                        std::vector<Face *> faces{&mFaces[i], &mFaces[j], &mFaces[k]};
                        Vertex vertex(vertex_pos, faces);

                        // Add the vertex to the various vertex sets
                        PushBackIfUnique(brush_vertices, vertex, 0.0001);
                        PushBackIfUnique(face_vertices[i], vertex, 0.0001);
                        PushBackIfUnique(face_vertices[j], vertex, 0.0001);
                        PushBackIfUnique(face_vertices[k], vertex, 0.0001);
                    }
                }
            }
        }

        // TODO: Seems messy extracting the vec3's like this idk
        std::vector<glm::vec3> vs;
        for (auto v : brush_vertices)
            vs.push_back(v.mPosition);
        mBoundingBox.Update(vs);
        // TODO: Verify these are correct
        mVertices = brush_vertices;

        // Reorder face vertices
        for (int i = 0; i < n; i++)
        {
            auto sorted_vs = FindPolygonPath(face_vertices[i], mFaces[i].GetPlane());
            mFaces[i].SetVertices(sorted_vs);
        }
    }

    void Brush::RebuildRegions()
    {
        // For each plane find intersecting regions for each other world brush
        // These regions are later subtracted from the main face region
        for (int f_idx = 0; f_idx < mFaces.size(); f_idx++)
        {
            auto face = mFaces[f_idx];
            auto plane = face.GetPlane();

            std::vector<std::vector<Vertex>> subtractive_regions;

            for (auto b : mWorld->GetBrushes())
            {
                if (b == this)
                    continue;

                std::vector<Vertex> brush_region;

                // Find intersection points using 2 planes of other brush
                auto planes = b->GetPlanes();
                int n = planes.size();
                for (int i = 0; i < n - 1; i++)
                {
                    for (int j = i + 1; j < n; j++)
                    {
                        // Is there a single intersection point beween these three planes?
                        glm::vec3 vertex_pos(0);
                        if (!plane.FindIntersectionPoint(planes[i], planes[j], vertex_pos))
                            continue;

                        // Is the point found within the other brush?
                        // Doesn't need to be within ourself, just along the plane we're currently concerned about
                        if (b->PointInPlanes(vertex_pos))
                        {
                            auto b_faces = b->GetFaces();
                            std::vector<Face *> vertex_faces{&face, &b_faces[i], &b_faces[j]};
                            Vertex vertex(vertex_pos, vertex_faces);

                            PushBackIfUnique(brush_region, vertex, 0.0001);
                        }
                    }
                }

                if (brush_region.size() > 0)
                {
                    auto sorted_brush_region = FindPolygonPath(brush_region, face.GetPlane());
                    subtractive_regions.push_back(sorted_brush_region);
                }
            }
            mFaces[f_idx].SetRegions(subtractive_regions);
        }
    }

    bool Brush::PointInPlanes(glm::vec3 point)
    {
        // Epsilon used because floating point precision sucks
        for (auto p : mPlanes)
            if ((glm::dot(p.mNormal, point) + p.mOffset) > 0.0001)
                return false;
        return true;
    }

    std::vector<Brush *> Brush::RebuildIntersections(std::vector<Brush *> brushes)
    {
        // Clear out current intersections both ways
        for (auto b : mIntersections)
            b->RemoveIntersection(this);
        mIntersections.clear();

        // Recalculate intersections
        for (auto b : brushes)
        {
            // Can't intersect with ourself or if AABBs don't intersect
            if (b == this || !mBoundingBox.Intersects(b->GetAABB()))
                continue;

            // If any vertices are in the other brushes planes (or vice versa) we have an intersection
            std::vector<Brush *> bs{this, b};
            bool intersects = false;
            for (int i = 0; i < 2; i++)
            {
                if (intersects)
                    continue;
                for (auto v : bs[i]->GetVertices())
                {
                    if (bs[(i + 1) % 2]->PointInPlanes(v.mPosition))
                    {
                        intersects = true;
                        break;
                    }
                }
            }

            // Add an intersection both ways if ones found
            if (intersects)
            {
                AddIntersection(b);
                b->AddIntersection(this);
            }
        }

        return mIntersections;
    }

    void Brush::AddIntersection(Brush *brush)
    {
        // Add the intersection only if it's not already in the intersection list
        if (mIntersections.size() == 0 || *std::find(mIntersections.begin(), mIntersections.end(), brush) != brush)
            mIntersections.push_back(brush);
    }

    void Brush::RemoveIntersection(Brush *brush)
    {
        // Early return assumes brushes are listed once (i.e. mIntersections is a set)
        for (int i = 0; i < mIntersections.size(); i++)
        {
            if (mIntersections[i] == brush)
            {
                mIntersections.erase(mIntersections.begin() + i);
                return;
            }
        }
    }

    AABB Brush::GetAABB()
    {
        return mBoundingBox;
    }

    void Brush::SetPlanes(std::vector<Plane> planes)
    {
        mPlanes = planes;
        for (int i = 0; i < mPlanes.size(); i++)
            mPlanes[i].Transform(mTransformMatrix);
        mWorld->RebuildBrush(this);
    }

    std::vector<Plane> Brush::GetPlanes()
    {
        return mPlanes;
    }

    std::vector<Face> Brush::GetFaces()
    {
        return mFaces;
    }

    std::vector<Vertex> Brush::GetVertices()
    {
        return mVertices;
    }

    std::vector<Brush *> Brush::GetIntersections()
    {
        return mIntersections;
    }

    void Brush::SetTransform(Transform transform)
    {
        // TODO: Rotation lol
        glm::mat4 tmat(1);
        tmat = glm::translate(tmat, transform.mTranslation);
        tmat = glm::scale(tmat, transform.mScale);
        for (int i = 0; i < mPlanes.size(); i++)
            mPlanes[i].Transform(tmat);

        mTransformMatrix = tmat;
        mTransform = transform;
    }

    Transform Brush::GetTransform()
    {
        return mTransform;
    }

    void Brush::Triangulate()
    {
        for (int i = 0; i < mFaces.size(); i++)
            mFaces[i].Triangulate();
    }
}