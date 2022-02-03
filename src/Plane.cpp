#include "Plane.hpp"

namespace ForgeCore
{
    void Plane::Transform(glm::mat4 transformation) {
        glm::vec4 plane{mNormal, mOffset};
        plane = glm::transpose(glm::inverse(transformation)) * plane;
        mNormal = glm::vec3{plane.x, plane.y, plane.z};
        mOffset = plane.w;
    }
}