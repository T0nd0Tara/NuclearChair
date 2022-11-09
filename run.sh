#!/bin/bash

g++ -I ~/code-libs/olcPixelGameEngine \
    -I ~/code-libs/magic_enum/include \
    -o main main.cpp \
    -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++20 -ggdb \
    -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable \
    && ./main
