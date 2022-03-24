#include "Face.hpp"

#include <cbop/polygon.h>
#include <cbop/booleanop.h>
#include <CDT.h>

#include "Brush.hpp"

namespace ForgeCore
{
    Face::Face(Brush *brush, Plane *plane)
    {
        mBrush = brush;
        mPlane = plane;
    }

    std::vector<Vertex> Face::GetVertices()
    {
        return mVertices;
    }

    std::vector<glm::vec3> Face::GetTriangleVertices()
    {
        return mTriangleVertices;
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

    unsigned int AddVsAndEs(cbop::Polygon polygon, int c_idx, std::vector<CDT::V2d<double>> &vs, std::vector<CDT::Edge> &es, unsigned int offset)
    {
        auto contour = polygon.contour(c_idx);

        // Add vs and es from self
        for (unsigned int i = 0; i < contour.nvertices(); i++)
        {
            auto v = contour.vertex(i);
            vs.push_back({v.x(), v.y()});
            es.push_back({offset + i, offset + (i + 1) % contour.nvertices()});
        }
        offset += contour.nvertices();

        // Add vs and es from holes recursively
        for (unsigned int i = 0; i = contour.nholes(); i++)
            offset = AddVsAndEs(polygon, contour.hole(i), vs, es, offset);
        return offset;
    }

    void Face::Triangulate()
    {
        mIndices.clear();
        mTriangleVertices.clear();

        // Create a final polygon from our regions
        auto target_cat = mBrush->GetOperation() == SUBTRACTION ? REVERSE_ALIGNED : ALIGNED;
        cbop::Polygon polygon, new_polygon;
        for (auto r : mRegions)
        {
            // We don't care about regions that aren't from our brush or are the wrong category
            if (r.mBrush != mBrush || r.mCategory != target_cat)
                continue;

            cbop::compute(polygon, r.mPolygon, new_polygon, cbop::UNION);

            polygon = new_polygon;
            new_polygon.clear();
        }

        // Early return if there's nothing left of the face
        if (polygon.ncontours() == 0)
            return;

        // Use constrained delaunay triangulation to triangulate the polygon
        CDT::Triangulation<double> cdt;

        // Get all the vertices/edges
        // cbop leaves extra contours and stuff on union
        // Need to just deal with contours that are children
        std::vector<CDT::V2d<double>> vs;
        std::vector<CDT::Edge> es;
        unsigned int offset = 0;
        for (unsigned int i = 0; i < polygon.ncontours(); i++)
        {
            if (!polygon.contour(i).external())
                continue;
            offset = AddVsAndEs(polygon, i, vs, es, offset);
        }

        // Compute triangulation and update
        CDT::RemoveDuplicatesAndRemapEdges(vs, es);
        cdt.insertVertices(vs);
        cdt.insertEdges(es);
        cdt.eraseOuterTrianglesAndHoles();
        vs = cdt.vertices; // CDT shuffles vertices so make sure we have the right order
        for (auto t : cdt.triangles)
        {
            // TODO: Order probably needs reversing on subtractive brushes
            if (mBrush->GetOperation() == ADDITION)
            {
                mIndices.push_back(t.vertices[2]);
                mIndices.push_back(t.vertices[1]);
                mIndices.push_back(t.vertices[0]);
            }
            else
            {
                mIndices.push_back(t.vertices[0]);
                mIndices.push_back(t.vertices[1]);
                mIndices.push_back(t.vertices[2]);
            }
        }

        // Translate vertices back to 3d and set them
        auto un = mPlane->mNormal / glm::length(mPlane->mNormal);
        auto loc_o = un * (-mPlane->mOffset / glm::length(mPlane->mNormal));
        auto loc_x = mVertices[0].mPosition - loc_o;
        auto loc_y = glm::cross(mPlane->mNormal, loc_x);
        loc_x /= glm::length(loc_x);
        loc_y /= glm::length(loc_y);
        for (auto v : vs)
            mTriangleVertices.push_back(loc_o + (float)v.x * loc_x + (float)v.y * loc_y);
    }
}