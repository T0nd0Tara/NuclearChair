#pragma once

#include <olcPixelGameEngine.h>

struct Object
{
    int nDecal;
    olc::vi2d pos;
    
    Object(olc::vi2d start_pos = {0,0}, int decal = -1)
        : nDecal(decal), pos(start_pos)
    {
    }
};

enum Mobs : int
{
    NONE = 0,
    PLAYER,
    ORC,
    SKELETON,
    TROLL,
};

struct Mob : public Object
{
    int nHp;
    Mob(olc::vi2d start_pos = {0,0}, int decal = -1, int hp = 0)
        : Object(start_pos, decal), nHp(hp)
    {
    }

    inline bool isAlive() const
    {
        return nHp > 0;
    }

    static int mobHp(int nMob)
    {
        if (nMob == NONE) return 0;
        return 100;
    }
};
