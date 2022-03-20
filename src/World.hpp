#pragma once

#include <vector>
#include <set>

#include "Brush.hpp"

namespace ForgeCore
{
    class World
    {
    public:
        World() = default;
        ~World() = default;

        Brush *AddBrush();
        std::vector<Brush *> GetBrushes();
        Brush *GetBrush(int index);
        int GetTime(Brush *b);
        std::set<Brush *> Update();
        void RebuildBrush(Brush *brush);

    private:
        std::vector<Brush *> mBrushes;
        std::set<Brush *> mNeedFullRebuild;
        std::set<Brush *> mNeedPartialRebuild;
    };
}