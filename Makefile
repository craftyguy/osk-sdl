SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LIBS  := $(shell sdl2-config --libs)

CXX        := g++
CXXFLAGS   := -std=c++14 -Wall -g $(SDL2_CFLAGS)

LIBS       := -lcryptsetup $(SDL2_LIBS) -lSDL2_ttf

SOURCES    := ${wildcard *.cpp}
OBJECTS    := $(SOURCES:%.cpp=%.o)

all: osk-sdl

%.o: %.cpp
	@echo CC $<
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

osk-sdl: $(OBJECTS)
	@echo LD $@
	@$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

clean:
	-rm -fv *.o osk-sdl
