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

    Plane Face::GetPlane()
    {
        return *mPlane;
    }

    void Face::Triangulate()
    {
        // Naive triangulation
        // TODO: Replace with something like delaunay
        mIndices.clear();

        unsigned int vertex_count = mVertices.size();
        std::vector<int> indices(vertex_count);

        for (int i = 0; i < vertex_count; i++)
            indices[i] = i;

        while (vertex_count >= 3)
        {
            // Triangle one
            mIndices.push_back(indices[0]);
            mIndices.push_back(indices[1]);
            mIndices.push_back(indices[2]);

            // Early loop exit
            if (vertex_count == 3)
                break;

            // Triangle 2
            mIndices.push_back(indices[2]);
            mIndices.push_back(indices[3]);
            mIndices.push_back(indices[4 % vertex_count]);

            // Removing processed vertices
            indices.erase(indices.begin() + 3);
            indices.erase(indices.begin() + 1);
            vertex_count -= 2;
        }
    }
}