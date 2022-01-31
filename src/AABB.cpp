#include "AABB.hpp"

namespace ForgeCore
{
    bool AABB::Intersects(AABB other)
    {
        return (mMin.x <= other.mMax.x && mMax.x >= other.mMin.x &&
                mMin.y <= other.mMax.y && mMax.y >= other.mMin.y &&
                mMin.z <= other.mMax.z && mMax.z >= other.mMin.z);
    }

    void AABB::Update(std::vector<glm::vec3> vertices)
    {
        // Reset min/max
        mMin = glm::vec3(vertices[0]);
        mMax = glm::vec3(vertices[0]);

        for (int i = 1; i < vertices.size(); i++)
        {
            auto v = vertices[i];
            mMin.x = std::min(v.x, mMin.x);
            mMin.y = std::min(v.y, mMin.y);
            mMin.z = std::min(v.z, mMin.z);
            mMax.x = std::max(v.x, mMax.x);
            mMax.y = std::max(v.y, mMax.y);
            mMax.z = std::max(v.z, mMax.z);
        }
    }
}