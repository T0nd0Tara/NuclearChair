GCC=g++
LIBS=-I ~/code-libs/olcPixelGameEngine -I ~/code-libs/magic_enum/include 
OLC_FLAGS=-lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++20
WARNING_FLAGS=-Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable 
GCC_FLAGS=$(OLC_FLAGS) $(WARNING_FLAGS)
BUILD_FOLDER=build
SRC_FOLDER=src
    
all: $(BUILD_FOLDER)/editor $(BUILD_FOLDER)/main
    
$(BUILD_FOLDER):
	mkdir -p $(BUILD_FOLDER)

$(BUILD_FOLDER)/includes.so: $(BUILD_FOLDER) $(SRC_FOLDER)/*.hpp
	$(GCC) -shared -o $(BUILD_FOLDER)/includes.so $(SRC_FOLDER)/includes.hpp $(GCC_FLAGS) $(LIBS)


$(BUILD_FOLDER)/main: $(SRC_FOLDER)/main.cpp $(BUILD_FOLDER) $(BUILD_FOLDER)/includes.so 
	$(GCC) -o $(BUILD_FOLDER)/main $(SRC_FOLDER)/main.cpp $(BUILD_FOLDER)/includes.so $(GCC_FLAGS) $(LIBS)

$(BUILD_FOLDER)/editor: $(SRC_FOLDER)/editor.cpp $(BUILD_FOLDER) $(BUILD_FOLDER)/includes.so 
	$(GCC) -o $(BUILD_FOLDER)/editor $(SRC_FOLDER)/editor.cpp $(BUILD_FOLDER)/includes.so $(GCC_FLAGS) $(LIBS)


