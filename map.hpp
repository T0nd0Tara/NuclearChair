#pragma once

#include <vector>
#include <fstream>
#include <cassert>

#include <olcPixelGameEngine.h>
#include "block.hpp"
#include "entity.hpp"

#define MAX_FILLAREA_BLOCKS 100 

class Map
{

    std::vector<std::vector<Block>> m_arr;

    void shift(size_t x, size_t y)
    {
        if (x != 0)
        {
            for (auto& row: m_arr)
            {
                row.resize(row.size() + x);
                for (int i = row.size() - 1; i >= x; i--)
                    row[i] = row[i - x];
                for (int i = x - 1; i >= 0; i--)
                    row[i] = Block();
            }
        }

        if (y != 0)
        {
            m_arr.resize(m_arr.size() + y);
            for (int i = m_arr.size() - 1; i >= y; i--)
                m_arr[i] = m_arr[i - y];
            for (int i = 0; i < y; i++)
                m_arr[i].clear();
        }
    }

    size_t m_nFillAreaBlocks;
    template <typename... T>
    olc::vi2d subFillArea(olc::vi2d pos, T ... args)
    {
        set(pos.x, pos.y, Block(args...));
        pos = pos.max({0,0});
        m_nFillAreaBlocks++;
        if (m_nFillAreaBlocks >= MAX_FILLAREA_BLOCKS) return pos;


        if (get(pos.x + 1, pos.y) == Block()) 
        {
            pos = subFillArea({pos.x + 1, pos.y}, args...);
            pos.x--;
        }
        if (get(pos.x, pos.y + 1) == Block()) {
            pos = subFillArea({pos.x, pos.y + 1}, args...);
            pos.y--;
        }
        if (get(pos.x - 1, pos.y) == Block()) {
            pos = subFillArea({pos.x - 1, pos.y}, args...);
            pos.x++;
        }        
        if (get(pos.x, pos.y - 1) == Block()) {
            pos = subFillArea({pos.x, pos.y - 1}, args...);
            pos.y++;
        }

        return pos;
    }
public:

    Mob player;
    std::vector<Mob> vMobs;

public:
    Map(int width = 0, int height = 0)
    {
        m_arr.emplace_back();
        m_arr.back().emplace_back();

        resize(width, height);
    }

    inline Block get(int x, int y) const noexcept
    {

        if (y < 0 || x < 0
            || y >= m_arr.size()) return Block();
        if (x >= m_arr[y].size()) return Block();
        return m_arr[y][x];
    }
    inline Block get(olc::vi2d pos) const noexcept
    {
        return get(pos.x, pos.y);
    }

    void set(int x, int y, Block b)
    {
        if (x < 0)
        {
            shift(-x, 0);
            x = 0;
        }
        if (y < 0)
        {
            shift(0,-y);
            y = 0;
        }

        if (y >= m_arr.size())
            m_arr.resize(y + 1);
        if (x >= m_arr[y].size())
            m_arr[y].resize(x + 1);
        
        m_arr[y][x] = b;

    }

    template <typename... T>
    inline void fillArea(olc::vi2d pos, T ... args)
    {
        m_nFillAreaBlocks = 0;
        subFillArea(pos, args...);

    }

    void shrink_to_fit()
    {
        if (m_arr.size() == 1 && m_arr.back().size() == 1) return;

        // clearing empty rows
        while (m_arr.size() > 1)
        {
            if (m_arr.back().size() == 0)
            {
                m_arr.pop_back();
                continue;
            }
            bool bRowEmpty = true;
            for (auto& c : m_arr.back())
                if (c.nDecal != 0)
                {
                    bRowEmpty = false;
                    break;
                }
            if (!bRowEmpty) break;
            m_arr.pop_back();
        }
        m_arr.shrink_to_fit();

        // clearing end of each row
        for (auto& row : m_arr)
        {
            while (row.back().nDecal == 0)
                row.pop_back();
            row.shrink_to_fit();
        }
    }

    void resize(int width, int height)
    {
        for (int y=m_arr.size()-1; y<height; y++)
        {
            m_arr.emplace_back();
            for (int x=m_arr[y].size() - 1; x<height; x++)
                m_arr[y].emplace_back();
        }
    }

    void clear()
    {
        for (auto& r : m_arr)
        {
            r.clear();
            r.shrink_to_fit();
        }
        m_arr.clear();
        m_arr.shrink_to_fit();
    }


    friend std::ofstream& operator<<(std::ofstream& f, Map& map)
    {
        map.shrink_to_fit();

        // write player
        f.write((char*)&map.player, sizeof(map.player));
        f << '\n';

        // write mobs
        for (const auto& mob : map.vMobs)
            f.write((char*)&mob, sizeof(mob));
        f << '\n';

        // write map
        for (size_t y = 0; y<map.m_arr.size(); y++)
        {
            for (size_t x=0; x<map.m_arr[y].size(); x++)
            {
                f.write((char*)&map.m_arr[y][x], sizeof(map.m_arr[y][x]));
            }
            f << '\n';
        }

        return f;
    }

    friend std::ifstream& operator>>(std::ifstream& f, Map& map)
    {
        map.clear();
        map.vMobs.clear();
       
        // read player
        if (f.peek() != '\n')
        {
            f.read((char*)&map.player, sizeof(map.player));
            assert(f.peek() == '\n');

        }
        f.ignore();

        // read mobs
        while (f.peek() != '\n')
        {
            map.vMobs.emplace_back();
            f.read((char*)&map.vMobs.back(), sizeof(map.vMobs.back()));
        }
        f.ignore();

        // read map
        while (!f.eof())
        {
            
            map.m_arr.emplace_back();
            while (f.peek() != '\n')
            {
                if (f.tellg() == -1) return f;
                map.m_arr.back().emplace_back();
                f.read((char*)&map.m_arr.back().back(), sizeof(map.m_arr.back().back()));
            }
            // ignores the '\n' char
            f.ignore();
        }

        return f;
    }
};
