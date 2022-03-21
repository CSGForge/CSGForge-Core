#include "World.hpp"

namespace ForgeCore
{
    Brush *World::AddBrush()
    {
        auto brush = new Brush(this);
        brush->SetOperation(ADDITION); // ! Hack because if I don't I get a segfault in editor????
        mBrushes.push_back(brush);
        return brush;
    }

    void World::RemoveBrush(Brush *brush)
    {
        for (auto b : brush->GetIntersections())
            mNeedPartialRebuild.insert(b);

        mBrushes.erase(mBrushes.begin() + GetTime(brush));
    }

    std::vector<Brush *> World::GetBrushes()
    {
        return mBrushes;
    }

    Brush *World::GetBrush(int index)
    {
        if (index >= 0 && index < mBrushes.size())
            return mBrushes[index];
        return nullptr;
    }

    int World::GetBrushCount()
    {
        return mBrushes.size();
    }

    int World::GetTime(Brush *b)
    {
        for (int i = 0; i < mBrushes.size(); i++)
            if (mBrushes[i] == b)
                return i;
        return -1;
    }

    void World::SetWorldType(WorldType worldType)
    {
        mWorldType = worldType;
    }

    WorldType World::GetWorldType()
    {
        return mWorldType;
    }

    std::set<Brush *> World::Update()
    {
        // Full rebuilds occur when a brushes planes have been edited
        for (auto b : mNeedFullRebuild)
        {
            b->RebuildFaces();
            auto intersections = b->RebuildIntersections(mBrushes);
            for (auto i : intersections)
                mNeedPartialRebuild.insert(i);
            mNeedPartialRebuild.insert(b);
        }

        for (auto b : mNeedPartialRebuild)
        {
            // Avoid recalculating intersections
            if (!mNeedFullRebuild.contains(b))
                b->RebuildIntersections(mBrushes);

            b->RebuildRegions();
            b->Triangulate();
            // TODO: Actual 2D boolean polygon operations on faces
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