#include "Brush.hpp"

// TODO: Remove unneccesary includes
#include <set>
#include <stdio.h>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <cbop/booleanop.h>

#include "World.hpp"

namespace ForgeCore
{
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

    float DistFromLine(glm::vec2 a, glm::vec2 b, glm::vec2 c)
    {
        // Z component of cross product of A->B and A-C
        // See: https://stackoverflow.com/questions/1560492
        return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
    }

    void QuickHull(std::vector<glm::vec2> vs, glm::vec2 a, glm::vec2 b, std::vector<glm::vec2> &out_vs)
    {
        // Recursion base case
        if (vs.size() == 0)
            return;

        // Find furthest point from segment AB
        glm::vec2 c;
        float max_dist = 0;
        for (auto v : vs)
        {
            auto dist = DistFromLine(a, b, v);
            if (dist > max_dist)
            {
                max_dist = dist;
                c = v;
            }
        }

        // Add C to the hull between A and B
        for (int i = 0; i < out_vs.size(); i++)
        {
            if (out_vs[i] == a)
            {
                out_vs.insert(out_vs.begin() + i + 1, c);
                break;
            }
        }

        // Find points on left of AC and CB
        std::vector<glm::vec2> s1, s2;
        for (auto v : vs)
        {
            if (v == a || v == b || v == c)
                continue;

            if (DistFromLine(a, c, v) > 0.0001)
                s1.push_back(v);
            else if (DistFromLine(c, b, v) > 0.0001)
                s2.push_back(v);
        }

        // Recurse
        QuickHull(s1, a, c, out_vs);
        QuickHull(s2, c, b, out_vs);
    }

    std::vector<glm::vec2> FindConvexHull(std::vector<glm::vec2> in_vs)
    {
        std::vector<glm::vec2> out_vs;

        // Find left and right most point indices
        int min_x = 0, max_x = 0;
        for (int i = 1; i < in_vs.size(); i++)
        {
            if (in_vs[i].x < in_vs[min_x].x)
                min_x = i;
            if (in_vs[i].x > in_vs[max_x].x)
                max_x = i;
        }

        // Add a and b to hull
        auto a = in_vs[min_x];
        auto b = in_vs[max_x];
        out_vs.push_back(a);
        out_vs.push_back(b);

        // Divide in_vs into s1 and s2
        std::vector<glm::vec2> s1, s2;
        for (auto v : in_vs)
        {
            if (v == a || v == b)
                continue;

            auto dist = DistFromLine(a, b, v);
            if (dist > 0.0001)
                s1.push_back(v);
            else if (dist < 0.0001)
                s2.push_back(v);
        }

        // Recursively add hull points
        QuickHull(s1, a, b, out_vs);
        QuickHull(s2, b, a, out_vs);

        return out_vs;
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
            mFaces.push_back(Face(this, &mPlanes[i]));
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

        // Set face vertices
        for (int i = 0; i < n; i++)
            mFaces[i].SetVertices(face_vertices[i]);
    }

    std::vector<Region> Categorise(std::vector<Region> rs, Region rb)
    {
        // Operation table used for finding new category
        // Indexed using [rb.operation][ra.category][rb.category]
        static RCategory op_table[2][4][4] = {{{INSIDE, INSIDE, INSIDE, INSIDE},
                                               {INSIDE, ALIGNED, INSIDE, ALIGNED},
                                               {INSIDE, INSIDE, REVERSE_ALIGNED, REVERSE_ALIGNED},
                                               {INSIDE, ALIGNED, REVERSE_ALIGNED, OUTSIDE}},
                                              {{OUTSIDE, REVERSE_ALIGNED, ALIGNED, INSIDE},
                                               {OUTSIDE, OUTSIDE, ALIGNED, ALIGNED},
                                               {OUTSIDE, REVERSE_ALIGNED, OUTSIDE, REVERSE_ALIGNED},
                                               {OUTSIDE, OUTSIDE, OUTSIDE, OUTSIDE}}};

        std::vector<Region> new_rs;
        std::vector<Region> rcs;
        bool rb_swapped = false;
        for (auto ra : rs)
        {
            cbop::Polygon new_ra, new_rb, new_rc;
            cbop::compute(ra.mPolygon, rb.mPolygon, new_ra, cbop::DIFFERENCE);
            cbop::compute(rb.mPolygon, ra.mPolygon, new_rb, cbop::DIFFERENCE);
            cbop::compute(ra.mPolygon, rb.mPolygon, new_rc, cbop::INTERSECTION);

            if (new_ra.ncontours() != 0)
            {
                ra.mPolygon = new_ra;
                new_rs.push_back(ra);
            }

            Region rc;
            if (new_rc.ncontours() != 0)
            {
                rc = rb;
                rc.mPolygon = new_rc;
                rc.mCategory = op_table[rc.mBrush->GetOperation()][ra.mCategory][rb.mCategory];
            }

            if (new_rb.ncontours() == 0)
            {
                rb = rc;
                rb_swapped = true;
            }
            else
            {
                rcs.push_back(rc);
                rb.mPolygon = new_rb;
            }
        }

        // If we've not swapped rb we haven't modified it's category
        // This means we need to operate against the void
        if (!rb_swapped)
            rb.mCategory = op_table[rb.mBrush->GetOperation()][rb.mBrush->GetWorld()->GetWorldType()][rb.mCategory];

        // Add all the new regions to the end
        for (auto rc : rcs)
            new_rs.push_back(rc);
        new_rs.push_back(rb);

        return new_rs;
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
        auto f_vs = f.GetVertices();
        for (auto v : f_vs)
            if (b1->PointInPlanes(v.mPosition))
                PushBackIfUnique(vs, v, 0.0001);
        // if (vs.size() == f_vs.size())
        //     return vs;

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

    Region BuildRegion(Brush *b, Face rf, std::vector<Vertex> rvs)
    {
        Plane rp = rf.GetPlane();

        Region r;
        r.mBrush = b;

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
        auto loc_o = rf_vs[0].mPosition;            // Use the first face vertex as origin
        auto loc_x = rf_vs[1].mPosition - loc_o;    // Local X axis
        auto loc_y = glm::cross(rp.mNormal, loc_x); // Local Y axis

        // Normalise axis
        loc_x /= glm::length(loc_x);
        loc_y /= glm::length(loc_y);

        std::vector<glm::vec2> lvs;
        for (auto v : rvs)
        {
            auto p = v.mPosition - loc_o;
            lvs.push_back({glm::dot(p, loc_x), glm::dot(p, loc_y)});
        }
        lvs = FindConvexHull(lvs);

        cbop::Contour contour;
        for (auto v : lvs)
            contour.add({v.x, v.y});
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
            auto face_region = BuildRegion(this, face, face.GetVertices());
            auto plane = face.GetPlane();
            bool before_self = true;
            for (auto b : mIntersections)
            {
                // If we've just past ourself in the timing, insert face region
                if (before_self != mWorld->GetTime(b) < mWorld->GetTime(this))
                {
                    regions = Categorise(regions, face_region);
                    before_self = false;
                }

                // TODO: Move onto next brush if intersecting before self and AABB is entirely contained

                // Find any intersection points between this face and the brush
                auto vs = FindIntersectingVertices(face, this, b);
                if (vs.size() == 0) // If there's no intersecting vertices just move on to the next brush
                    continue;

                // Create a new region
                regions = Categorise(regions, BuildRegion(b, face, vs));
            }

            // This only happens if there are no intersections on this face
            // or all the intersecting brushes are before ourself
            if (before_self)
                regions = Categorise(regions, face_region);

            mFaces[f_idx].SetRegions(regions);
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

    World *Brush::GetWorld()
    {
        return mWorld;
    }

    void Brush::SetOperation(Operation operation)
    {
        mOperation = operation;
        mWorld->RebuildBrush(this);
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
        mWorld->RebuildBrush(this);
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