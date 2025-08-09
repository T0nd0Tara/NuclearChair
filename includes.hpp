#pragma once

#include <cassert>
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#define OLC_PGEX_TRANSFORMEDVIEW
#include <extensions/olcPGEX_TransformedView.h>

#include <magic_enum.hpp>

#include "block.hpp"
#include "entity.hpp"
#include "map.hpp"

#define PI 3.14159f

#define RESOLUTION 4
#define MAX_FPS 60.0f

#define MAP_DIR std::string("maps/")
#define BLOCK_SIZE 32
#define fBLOCK_SIZE ((float)BLOCK_SIZE)
#define vBLOCK_SIZE                                                            \
  olc::vf2d { fBLOCK_SIZE, fBLOCK_SIZE }

#define MOB_POSES 9
#define MOB_SIZE olc::vi2d{16, 16}
#define MOB_FILE_SIZE                                                          \
  olc::vi2d { MOB_SIZE.x *MOB_POSES, MOB_SIZE.y }

#define BULLET_FILE_SIZE olc::vi2d{16, 16}
#define BULLET_SIZE (BLOCK_SIZE >> 1)
#define fBULLET_SIZE (float)BULLET_SIZE
#define vBULLET_SIZE                                                           \
  olc::vf2d { fBULLET_SIZE, fBULLET_SIZE }

#define TARGET_FILE_SIZE olc::vi2d{13, 13}
#define fTARGET_FILE_SIZE                                                      \
  olc::vf2d { TARGET_FILE_SIZE }
#define TARGET_SIZE (fBLOCK_SIZE / fTARGET_FILE_SIZE)

#define BG_COLOR olc::CYAN
// World Types
enum class WT : int8_t { BLOCKS, MOBS };
