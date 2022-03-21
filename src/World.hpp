#pragma once

#include <vector>
#include <set>

#include "Brush.hpp"

namespace ForgeCore
{
    // Enum values correspond to face region categories (3 = OUTSIDE, 0 = INSIDE)
    enum WorldType
    {
        AIR_WORLD = 3,
        SOLID_WORLD = 0
    };

    class World
    {
    public:
        World() = default;
        ~World() = default;

        Brush *AddBrush();
        void RemoveBrush(Brush *brush);
        std::vector<Brush *> GetBrushes();
        int GetBrushCount();
        Brush *GetBrush(int index);
        int GetTime(Brush *b);
        void SetWorldType(WorldType worldType);
        WorldType GetWorldType();
        std::set<Brush *> Update();
        void RebuildBrush(Brush *brush);

    private:
        std::vector<Brush *> mBrushes;
        std::set<Brush *> mNeedFullRebuild;
        std::set<Brush *> mNeedPartialRebuild;
        WorldType mWorldType = AIR_WORLD;
    };
}