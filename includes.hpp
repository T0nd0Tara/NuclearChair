#pragma once

#include <filesystem>
#include <sstream>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <set>
#include <chrono>
#include <thread>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#define OLC_PGEX_TRANSFORMEDVIEW
#include <Extensions/olcPGEX_TransformedView.h>

#include <magic_enum.hpp>

#include "entity.hpp"
#include "block.hpp"
#include "map.hpp"

#define PI 3.14159f

#define RESOLUTION 4
#define MAX_FPS 60.0f

#define BLOCK_SIZE 32
#define fBLOCK_SIZE ((float)BLOCK_SIZE)
#define MAP_DIR std::string("maps/")

#define MOB_SPRITE_WIDTH  (16 * 9)
#define MOB_SPRITE_HEIGHT 16
#define MOB_SIZE olc::vi2d{MOB_SPRITE_HEIGHT, MOB_SPRITE_HEIGHT}

#define TARGET_FILE_SIZE olc::vi2d{13,13}
#define fTARGET_FILE_SIZE olc::vf2d{TARGET_FILE_SIZE}
#define TARGET_SIZE (fBLOCK_SIZE / fTARGET_FILE_SIZE) 

#define BG_COLOR olc::CYAN
// World Types
enum class WT : int8_t
{
    BLOCKS,
    MOBS
};
