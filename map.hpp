#pragma once

#include <vector>
#include <fstream>

#include <olcPixelGameEngine.h>
#include "block.hpp"

class Map
{
    std::vector<std::vector<Block>> m_arr;

public:
    Map(int width = 0, int height = 0)
    {
        resize(width, height);

    }

    inline Block get(int x, int y) const noexcept
    {

        if (y < 0 || x < 0
            || y >= m_arr.size()) return Block();
        if (x >= m_arr[y].size()) return Block();
        return m_arr[y][x];
    }

    void shrink_to_fit()
    {
        // clearing empty rows
        while (true)
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
        for (int y=0; y<height; y++)
        {
            m_arr.emplace_back();
            for (int x=0; x<height; x++)
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

    inline const std::vector<Block>& operator[](int i) const
    {
        return m_arr[i];
    }

    friend std::ofstream& operator<<(std::ofstream& f, Map& map)
    {
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
