#pragma once

#include <filesystem>
#include <sstream>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <set>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#define OLC_PGEX_TRANSFORMEDVIEW
#include <Extensions/olcPGEX_TransformedView.h>

#include <magic_enum.hpp>

#include "entity.hpp"
#include "block.hpp"
#include "map.hpp"

#define RESOLUTION 4
#define BLOCK_SIZE 32
#define fBLOCK_SIZE ((float)BLOCK_SIZE)
#define MAP_DIR std::string("maps/")

#define MOB_SPRITE_WIDTH  180
#define MOB_SPRITE_HEIGHT 20
#define MOB_SIZE olc::vi2d{MOB_SPRITE_HEIGHT, MOB_SPRITE_HEIGHT}

#define BG_COLOR olc::CYAN
// World Types
enum class WT : int8_t
{
    BLOCKS,
    MOBS
};
