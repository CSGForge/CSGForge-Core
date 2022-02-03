#include "World.hpp"

namespace ForgeCore
{
    Brush *World::AddBrush()
    {
        auto brush = new Brush();
        mBrushes.push_back(brush);
        return brush;
    }

    std::vector<Brush *> World::GetBrushes()
    {
        return mBrushes;
    }

    std::set<Brush *> World::Update()
    {
        std::set<Brush *> updated_brushes;
        for (auto b : mBrushes)
        {
            if (!b->IsDirty())
                continue;

            b->RebuildFaces();
            auto intersections = b->RebuildIntersections(mBrushes);
            b->SetClean();

            // Add to the updated brush list
            updated_brushes.insert(b);
            // TODO: Add intersections to updated brushes (if they're not already in there)
        }
        return updated_brushes;
    }
}