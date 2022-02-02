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
            // TODO: Brush needs to be set to clean at some point

            b->RebuildFaces();
            updated_brushes.insert(b);
        }
        return updated_brushes;
    }
}