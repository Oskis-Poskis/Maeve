CXX = g++
CC  = gcc

SRC = $(shell find src -type f -name '*.cpp') lib/glad/glad.c
OBJ  := $(SRC:.cpp=.o)
OBJ  := $(OBJ:c.=.o)

TARGET = maeve

DEBUG_FLAGS = -g -DDEBUG -Wall -Wextra $(shell pkg-config --cflags freetype2) -std=c++23
RELEASE_FLAGS = -O2 $(shell pkg-config --cflags freetype2) -std=c++23

LDFLAGS = -lglfw -lglm $(shell pkg-config --libs freetype2)

MAKEFLAGS += --no-print-directory

all: release

debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

release: CXXFLAGS = $(RELEASE_FLAGS)
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	@find . -name "*.o" -exec rm -f {} \;
	@rm -f $(TARGET)