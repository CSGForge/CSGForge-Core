#include "World.hpp"

namespace ForgeCore
{
    Brush *World::AddBrush()
    {
        auto brush = new Brush(this);
        mBrushes.push_back(brush);
        return brush;
    }

    std::vector<Brush *> World::GetBrushes()
    {
        return mBrushes;
    }

    std::set<Brush *> World::Update()
    {
        for (auto b : mNeedFullRebuild)
        {
            b->RebuildFaces();

            // TODO: Check if this is potentially recalculating multiple times in some cases (two intersecting brushes both have a full update)
            auto intersections = b->RebuildIntersections(mBrushes);
            for (auto i : intersections)
                mNeedPartialRebuild.insert(i);
            mNeedPartialRebuild.insert(b);
        }

        for (auto b : mNeedPartialRebuild)
        {
        }

        std::set<Brush *> updated_brushes = mNeedPartialRebuild;
        mNeedFullRebuild.clear();
        mNeedPartialRebuild.clear();
        return updated_brushes;
    }

    void World::RebuildBrush(Brush *brush)
    {
        mNeedFullRebuild.insert(brush);
    }
}