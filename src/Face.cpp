#include "Face.hpp"

#include "Brush.hpp"

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

        // Build a set of face neighbours. Ugly
        std::vector<Face> ns;
        for (auto v : vertices)
        {
            for (auto f : v.mFaces)
            {
                if (f == this)
                    continue;

                bool unique = true;
                for (auto n : ns)
                {
                    if (f->GetPlane() == n.GetPlane())
                    {
                        unique = false;
                        break;
                    }
                }
                if (unique)
                    ns.push_back(*f);
            }
        }

        mNeighbours = ns;
    }

    std::vector<Face> Face::GetNeighbourFaces()
    {
        return mNeighbours;
    }

    Plane Face::GetPlane()
    {
        return *mPlane;
    }

    void Face::SetRegions(std::vector<Region> regions)
    {
        mRegions = regions;
    }

    std::vector<Region> Face::GetRegions()
    {
        return mRegions;
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