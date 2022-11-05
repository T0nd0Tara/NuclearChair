#pragma once

#define OLC_PGE_APPLICATION
#include <olcPixelGameEngine.h>

#define OLC_PGEX_TRANSFORMEDVIEW
#include <Extensions/olcPGEX_TransformedView.h>

#include <filesystem>
#include <sstream>
#include <string>
#include <iostream>
#include <cassert>

#include "entity.hpp"
#include "block.hpp"
#include "map.hpp"

#define RESOLUTION 4
#define BLOCK_SIZE 8
#define fBLOCK_SIZE ((float)BLOCK_SIZE)
