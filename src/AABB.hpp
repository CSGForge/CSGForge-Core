#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace ForgeCore
{
    class AABB
    {
    public:
        AABB() = default;
        ~AABB() = default;

        bool Intersects(AABB other);
        void Update(std::vector<glm::vec3> vertices);
        void Spew();

    private:
        glm::vec3 mMin;
        glm::vec3 mMax;
    };
}