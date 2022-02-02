#include "Face.hpp"

namespace ForgeCore
{
    Face::Face(Plane *plane)
    {
        mPlane = plane;
    }

    std::vector<glm::vec3> Face::GetVertices()
    {
        return mVertices;
    }

    std::vector<unsigned int> Face::GetIndices()
    {
        return mIndices;
    }

    void Face::SetVertices(std::vector<glm::vec3> vertices)
    {
        mVertices = vertices;
    }
}