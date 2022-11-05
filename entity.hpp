#pragma once

#include <olcPixelGameEngine.h>

class Object
{
protected:
    int m_nDecal;
public:
    olc::vi2d pos;
    
    Object(olc::vi2d start_pos = {0,0}, int decal = -1)
        : m_nDecal(decal), pos(start_pos)
    {
    }
};

class Mob : public Object
{
public:
    int nHp;
    Mob(olc::vi2d start_pos = {0,0}, int decal = -1, int hp = 0)
        : Object(start_pos, decal), nHp(hp)
    {
    }

    inline bool isAlive() const
    {
        return nHp > 0;
    }
};
