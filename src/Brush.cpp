#include "Brush.hpp"

// TODO: Remove unneccesary includes
#include <set>
#include <stdio.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include "World.hpp"

namespace ForgeCore
{
    std::vector<glm::vec2> PointsInRegion(std::vector<glm::vec2> r_vs, std::vector<glm::vec2> vs)
    {
        // Regions are convex, so we can check if a point is inside it by forming triangles and checking against them
        std::vector<glm::vec2> vs_in_r;
        auto v0 = r_vs[0];
        auto v1 = r_vs[1];
        for (auto p : vs)
        {
            for (int k = 2; k < r_vs.size(); k++)
            {
                // See: http://totologic.blogspot.com/2014/01/accurate-point-in-triangle-test.html
                auto v2 = r_vs[2];
                float zz = ((v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y));
                float a = ((v1.y - v2.y) * (p.x - v2.x) + (v2.x - v1.x) * (p.y - v2.y)) / zz;
                float b = ((v2.y - v0.y) * (p.x - v2.x) + (v0.x - v2.x) * (p.y - v2.y)) / zz;
                if (0 <= a && a <= 0 && 0 <= b && b <= 1 && a + b <= 1)
                    vs_in_r.push_back(p);
            }
        }
        return vs_in_r;
    }

    int PushBackIfUnique(std::vector<Vertex> &vs, Vertex v1, float eps)
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
                return i;
            }
        }

        vs.push_back(v1);
        return vs.size() - 1;
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
            mFaces[i].SetVertices(FindPolygonPath(face_vertices[i], mFaces[i].GetPlane()));
    }

    std::vector<Region> Categorise(std::vector<Vertex> &verts, std::vector<Region> rs, Region rb)
    {
        // Operation table used for finding new category
        // Indexed using [rb.operation][ra.category][rb.category]
        static RCategory op_table[2][4][4] = {{{INSIDE, INSIDE, INSIDE, INSIDE},
                                               {INSIDE, OUTSIDE, INSIDE, ALIGNED},
                                               {INSIDE, INSIDE, OUTSIDE, REVERSE_ALIGNED},
                                               {INSIDE, OUTSIDE, OUTSIDE, OUTSIDE}},
                                              {{OUTSIDE, REVERSE_ALIGNED, ALIGNED, INSIDE},
                                               {OUTSIDE, OUTSIDE, ALIGNED, ALIGNED},
                                               {OUTSIDE, REVERSE_ALIGNED, OUTSIDE, REVERSE_ALIGNED},
                                               {OUTSIDE, OUTSIDE, OUTSIDE, OUTSIDE}}};

        auto contour = cbop::Contour();

        // TODO: Replace with proper categorisation
        rs.push_back(rb);
        return rs;
    }

    std::vector<Vertex> FindIntersectingVertices(Face f, Brush *b0, Brush *b1)
    {
        // This is essentially cutting the face using the planes of b0 and b1
        // TODO: See if doing that explicitly is faster

        std::vector<Vertex> vs;
        auto f_plane = f.GetPlane();
        auto b1_planes = b1->GetPlanes();
        auto b1_faces = b1->GetFaces();
        int n = b1_planes.size();

        // Three types of intersecting vertices exist:
        //  (1) Face bounding vertices that are within the other brush
        for (auto v : f.GetVertices())
            if (b1->PointInPlanes(v.mPosition))
                PushBackIfUnique(vs, v, 0.0001);

        //  (2) Vertices that are on a face and lie along a face edge of the other brush
        for (int i = 0; i < n - 1; i++)
            for (int j = i + 1; j < n; j++)
            {
                glm::vec3 vertex_pos(0);
                if (f_plane.FindIntersectionPoint(b1_planes[i], b1_planes[j], vertex_pos) && b0->PointInPlanes(vertex_pos) && b1->PointInPlanes(vertex_pos))
                    PushBackIfUnique(vs, {vertex_pos, {&f, &b1_faces[i], &b1_faces[j]}}, 0.0001);
            }

        //  (3) Vertices that lie along a face edge and are on a face of the other brush
        auto ns = f.GetNeighbourFaces();
        for (int i = 0; i < ns.size(); i++)
            for (int j = 0; j < n; j++)
            {
                glm::vec3 vertex_pos(0);
                if (f_plane.FindIntersectionPoint(ns[i].GetPlane(), b1_planes[j], vertex_pos) && b0->PointInPlanes(vertex_pos) && b1->PointInPlanes(vertex_pos))
                    PushBackIfUnique(vs, {vertex_pos, {&f, &ns[i], &b1_faces[j]}}, 0.0001);
            }

        return vs;
    }

    Region BuildRegion(std::vector<Vertex> &vs, Brush *b, Face rf, std::vector<Vertex> rvs)
    {
        Plane rp = rf.GetPlane();

        Region r;
        r.mBrush = b;
        for (auto v : rvs)
            r.mIndices.push_back(PushBackIfUnique(vs, v, 0.0001));

        // A region is only (reverse) aligned if all vertices are on a plane
        // Aligned if matching normals, reverse aligned if opposite normals
        // We know the region has a normal matching the plane so just compare that against other brush plane
        // If not (reverse) aligned then it's inside (an intersection exists, it can't be outside)
        r.mCategory = INSIDE;

        auto un1 = rp.mNormal / glm::length(rp.mNormal);
        for (auto p : b->GetPlanes())
        {
            int num_aligned = 0;
            for (auto v : rvs)
                if (std::abs(p.mNormal.x * v.mPosition.x + p.mNormal.y * v.mPosition.y + p.mNormal.z * v.mPosition.z + p.mOffset) < 0.0001)
                    num_aligned++;

            if (num_aligned == rvs.size())
            {
                auto un2 = p.mNormal / glm::length(p.mNormal);
                r.mCategory = (un1 == un2) ? ALIGNED : REVERSE_ALIGNED;
                break;
            }
        }

        // Convert vertices to local plane coordinates for later polygon clipping
        // Initially found regions will only have a single, external, contour
        // See: https://stackoverflow.com/questions/26369618/getting-local-2d-coordinates-of-vertices-of-a-planar-polygon-in-3d-space
        auto rf_vs = rf.GetVertices();
        auto loc_o = un1 * (-rp.mOffset);        // Point on plane closest to origin
        auto loc_x = rf_vs[0].mPosition - loc_o; // Local X axis
        auto loc_y = glm::cross(un1, loc_x);     // Local Y axis

        // Normalise axis
        loc_x /= glm::length(loc_x);
        loc_y /= glm::length(loc_y);

        cbop::Contour contour;
        for (auto v : rvs)
        {
            auto p = v.mPosition - loc_o;
            contour.add({glm::dot(p, loc_x), glm::dot(p, loc_y)});
        }
        r.mPolygon.push_back(contour);

        return r;
    }

    void Brush::RebuildRegions()
    {
        // If there's no intersections we can just be quick and painless
        // TODO: Early insert and return of face regions if no intersections

        for (int f_idx = 0; f_idx < mFaces.size(); f_idx++)
        {
            std::vector<Region> regions;
            auto face = mFaces[f_idx];
            std::vector<Vertex> intersecting_verts = face.GetVertices();
            auto plane = face.GetPlane();
            bool before_self = true;
            for (auto b : mIntersections)
            {
                // If we've just past ourself in the timing, insert face region
                if (before_self != mWorld->GetTime(b) < mWorld->GetTime(this))
                {
                    auto region = BuildRegion(intersecting_verts, this, face, face.GetVertices());
                    regions = Categorise(intersecting_verts, regions, region);
                    before_self = false;
                }

                // TODO: Move onto next brush if intersecting before self and AABB is entirely contained

                // Find any intersection points between this face and the brush
                auto vs = FindIntersectingVertices(face, this, b);
                if (vs.size() == 0) // If there's no intersecting vertices just move on to the next brush
                    continue;

                // Create a new region
                auto region = BuildRegion(intersecting_verts, b, face, FindPolygonPath(vs, plane));
                regions = Categorise(intersecting_verts, regions, region);
            }
            mFaces[f_idx].SetRegions(regions, intersecting_verts);
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

    Operation Brush::GetOperation()
    {
        return mOperation;
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