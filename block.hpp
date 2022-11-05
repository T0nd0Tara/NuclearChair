#pragma once

#include <olcPixelGameEngine.h>

struct Block
{
    size_t nDecal;

    bool bVisible;
    bool bBG;

public:
    Block(size_t decal = 0, bool visible = true, bool bg = false)
        : nDecal(decal), bVisible(visible), bBG(bg)
    {
        if (nDecal == 0)
        {
            bVisible = false;
            bBG = true;
        }
    }

    Block(const Block&) = default;

    Block& operator=(Block rhs)
    {
        nDecal = rhs.nDecal;
        bVisible = rhs.bVisible;
        bBG = rhs.bBG;
        return *this;
    }
};
