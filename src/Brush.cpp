#include "Brush.hpp"

#include <set>

#include <glm/gtc/matrix_access.hpp>

namespace ForgeCore
{
    void PushBackIfUnique(std::vector<glm::vec3> &vs, glm::vec3 v1, float eps)
    {
        for (auto v2 : vs)
            if (std::abs(v1.x - v2.x) <= eps && std::abs(v1.y - v2.y) <= eps && std::abs(v1.z - v2.z) <= eps)
                return;
        vs.push_back(v1);
    }

    bool Brush::IsDirty()
    {
        return mDirty;
    }

    void Brush::RebuildFaces()
    {
        // TODO: Check that there's 4+ planes
        // Clear vertices for each face
        int n = mPlanes.size();
        std::vector<std::vector<glm::vec3>> face_vertices;

        mFaces.clear();
        for (int i = 0; i < n; i++)
        {
            std::vector<glm::vec3> v;
            mFaces.push_back(Face(&mPlanes[i]));
            mFaces[i].SetVertices(v);
            face_vertices.push_back(v);
        }

        // Calculate vertices
        std::vector<glm::vec3> brush_vertices;
        for (int i = 0; i < n - 2; i++)
        {
            for (int j = i + 1; j < n - 1; j++)
            {
                for (int k = j + 1; k < n; k++)
                {
                    // Find the intersection of these 3 planes (if one exists)
                    // `https://en.wikipedia.org/wiki/Cramer's_rule#Explicit_formulas_for_small_systems`
                    auto p1 = mPlanes[i];
                    auto p2 = mPlanes[j];
                    auto p3 = mPlanes[k];

                    // Build the matrix used in the denominator
                    glm::mat3 m;
                    m = glm::row(m, 0, p1.mNormal);
                    m = glm::row(m, 1, p2.mNormal);
                    m = glm::row(m, 2, p3.mNormal);

                    // Epsilon used because floating point precision sucks
                    float det_m = glm::determinant(m);
                    if (std::abs(det_m) <= 0.001)
                        continue;

                    glm::mat3 mx(m), my(m), mz(m);
                    glm::vec3 offsets(p1.mOffset, p2.mOffset, p3.mOffset);
                    glm::vec3 vertex(
                        glm::determinant(glm::column(mx, 0, -offsets)) / det_m,
                        glm::determinant(glm::column(my, 1, -offsets)) / det_m,
                        glm::determinant(glm::column(mz, 2, -offsets)) / det_m);

                    // Check that the found vertex isn't outside of any of the planes
                    bool outside = false;
                    for (auto p : mPlanes)
                    {
                        if ((glm::dot(p.mNormal, vertex) + p.mOffset) > 0.001)
                        {
                            outside = true;
                            break;
                        }
                    }
                    if (!outside)
                    {
                        // Add the vertex to the various vertex sets
                        PushBackIfUnique(brush_vertices, vertex, 0.0001);
                        PushBackIfUnique(face_vertices[i], vertex, 0.0001);
                        PushBackIfUnique(face_vertices[j], vertex, 0.0001);
                        PushBackIfUnique(face_vertices[k], vertex, 0.0001);
                    }
                }
            }
            // TODO: Work out face vertex indices to be nice
            mFaces[i].SetVertices(face_vertices[i]);
        }

        // Update AABB
        mBoundingBox.Update(brush_vertices);
    }

    AABB Brush::GetAABB()
    {
        return mBoundingBox;
    }

    void Brush::SetPlanes(std::vector<Plane> planes)
    {
        mPlanes = planes;
        mDirty = true;
    }

    std::vector<Plane> Brush::GetPlanes()
    {
        return mPlanes;
    }

    std::vector<Face> Brush::GetFaces()
    {
        return mFaces;
    }
}