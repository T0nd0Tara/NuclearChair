#pragma once

#include <olcPixelGameEngine.h>

struct Object
{
    int nDecal;
    olc::vf2d pos;
    
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
    olc::Pixel tint = olc::WHITE;
    static std::vector<int> vMobHp;
    static constexpr float fMaxTintChange = 0.5f;

    Mob(olc::vf2d start_pos = {0,0}, int decal = -1, int hp = 0)
        : Object(start_pos, decal), nHp(hp)
    {
        if (hp == 0)
            nHp = vMobHp[decal];
    }

    inline bool isAlive() const
    {
        return nHp > 0;
    }

    inline void updateTint(float fElapsedTime)
    {
        tint += olc::WHITE * fElapsedTime / fMaxTintChange;
    }

    friend std::ifstream& operator >> (std::ifstream& f, Mob& mob)
    { 
        f.read((char*)&mob.nDecal, sizeof(mob.nDecal));
        f.read((char*)&mob.pos, sizeof(mob.pos));
        f.read((char*)&mob.nHp, sizeof(mob.nHp));
        return f;      
    }      
    friend std::ofstream& operator << (std::ofstream& f, const Mob& mob)
    {      
        f.write((char*)&mob.nDecal, sizeof(mob.nDecal));
        f.write((char*)&mob.pos, sizeof(mob.pos));
        f.write((char*)&mob.nHp, sizeof(mob.nHp));
        return f;   
    } 

};

std::vector<int> Mob::vMobHp{0};
