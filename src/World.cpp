#include "World.hpp"

namespace ForgeCore
{
    bool World::Modified()
    {
        bool was_modified = mModified;
        mModified = false;
        return was_modified;
    }

    Brush *World::AddBrush()
    {
        auto brush = new Brush(this);
        brush->SetOperation(ADDITION); // ! Hack because if I don't I get a segfault in editor????
        mBrushes.push_back(brush);
        mNeedFullRebuild.insert(brush);
        return brush;
    }

    void World::RemoveBrush(Brush *brush)
    {
        for (auto b : brush->GetIntersections())
        {
            mNeedPartialRebuild.insert(b);
            b->RemoveIntersection(brush);
        }
        mBrushes.erase(mBrushes.begin() + GetTime(brush));
        mModified = true;
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
        if (mWorldType == worldType)
            return;

        mWorldType = worldType;
        for (auto b : mBrushes)
            mNeedPartialRebuild.insert(b);
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
            // Anything we previously intersected with needs a partial rebuild
            for (auto i : b->GetIntersections())
                mNeedPartialRebuild.insert(i);

            // Anything we now intersect with needs a partial rebuild
            b->RebuildFaces();
            b->RebuildIntersections();
            for (auto i : b->GetIntersections())
                mNeedPartialRebuild.insert(i);
            mNeedPartialRebuild.insert(b);
        }

        for (auto b : mNeedPartialRebuild)
        {
            b->RebuildRegions();
            b->Triangulate();
        }

        std::set<Brush *> updated_brushes = mNeedPartialRebuild;
        mNeedFullRebuild.clear();
        mNeedPartialRebuild.clear();

        if (updated_brushes.size() != 0)
            mModified = true;

        return updated_brushes;
    }

    void World::RebuildBrush(Brush *brush)
    {
        mNeedFullRebuild.insert(brush);
    }

    void World::SetBrushTime(Brush *brush, unsigned int time)
    {
        if (time >= mBrushes.size())
            return;

        int old_time = GetTime(brush);
        if (time == old_time)
            return;

        mBrushes.erase(mBrushes.begin() + old_time);
        mBrushes.insert(mBrushes.begin() + time, brush);
    }
}