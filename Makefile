TARGET     := osk-sdl

SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LIBS  := $(shell sdl2-config --libs)

CXX        := g++
CXXFLAGS   := -std=c++14 -Wall -g $(SDL2_CFLAGS)

LIBS       := -lcryptsetup $(SDL2_LIBS) -lSDL2_ttf

SRC_DIR    := src
BIN_DIR    := bin
OBJ_DIR    := obj

SOURCES    := ${wildcard $(SRC_DIR)/*.cpp}
OBJECTS    := $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: directories $(BIN_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo CC $<
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@echo LD $@
	@$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

.PHONY: directories

clean:
	-rm -rfv $(OBJ_DIR) $(BIN_DIR)

directories:
	@mkdir -p ./obj
	@mkdir -p ./bin

