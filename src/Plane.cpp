#include "Plane.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <stdio.h>

namespace ForgeCore
{
    Plane::Plane(glm::vec3 normal, float offset)
    {
        mNormal = normal;
        mOffset = offset;
        mBaseNormal = normal;
        mBaseOffset = offset;
    }

    void Plane::Transform(glm::mat4 transformation)
    {
        glm::vec4 plane{mBaseNormal, mBaseOffset};
        plane = glm::transpose(glm::inverse(transformation)) * plane;
        mNormal = glm::vec3{plane.x, plane.y, plane.z};
        mOffset = plane.w;
    }

    bool Plane::FindIntersectionPoint(Plane p1, Plane p2, glm::vec3 &vertex_position)
    {
        // Find the intersection of these 3 planes (if one exists)
        // `https://en.wikipedia.org/wiki/Cramer's_rule#Explicit_formulas_for_small_systems`

        // Build the matrix used in the denominator
        glm::mat3 m;
        m = glm::row(m, 0, mNormal);
        m = glm::row(m, 1, p1.mNormal);
        m = glm::row(m, 2, p2.mNormal);

        // Epsilon used because floating point precision sucks
        float det_m = glm::determinant(m);
        if (std::abs(det_m) <= 0.001)
            return false;

        glm::mat3 mx(m), my(m), mz(m);
        glm::vec3 offsets(mOffset, p1.mOffset, p2.mOffset);

        vertex_position.x = glm::determinant(glm::column(mx, 0, -offsets)) / det_m;
        vertex_position.y = glm::determinant(glm::column(my, 1, -offsets)) / det_m;
        vertex_position.z = glm::determinant(glm::column(mz, 2, -offsets)) / det_m;
        return true;
    }

    bool Plane::operator==(const Plane &p) const
    {
        return (mOffset == p.mOffset && mNormal.x == p.mNormal.x && mNormal.y == p.mNormal.y && mNormal.z == p.mNormal.z);
    }
}