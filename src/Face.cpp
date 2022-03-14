#include "Face.hpp"

namespace ForgeCore
{
    Face::Face(Plane *plane)
    {
        mPlane = plane;
    }

    std::vector<Vertex> Face::GetVertices()
    {
        return mVertices;
    }

    std::vector<unsigned int> Face::GetIndices()
    {
        return mIndices;
    }

    void Face::SetVertices(std::vector<Vertex> vertices)
    {
        mVertices = vertices;
    }

    Plane Face::GetPlane() {
        return *mPlane;
    }
}