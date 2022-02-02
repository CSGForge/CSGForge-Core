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
        std::set<Brush *> Update();

    private:
        std::vector<Brush *> mBrushes;
    };
}