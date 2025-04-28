TARGET = maeve

SRC_C   = glad/glad.c
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
SRC_CPP := $(call rwildcard,src/,*.cpp)

OBJDIR  = build
OBJ_C   = $(addprefix $(OBJDIR)/, $(SRC_C:.c=.o))
OBJ_CPP = $(addprefix $(OBJDIR)/, $(SRC_CPP:.cpp=.o))

default: debug

check-os:
ifeq ($(OS),Windows_NT)
    $(info Building on Windows)
	DEBUG_FLAGS   = -g -DDEBUG -Wall -Wextra -IC:/include -IC:/include/GLFW/include -IC:/include/freetype2/include -std=c++20
	RELEASE_FLAGS = -O2 -IC:/include -IC:/include/GLFW/include -IC:/include/freetype2/include -std=c++20
    LDFLAGS       = -IC:/include -IC:/include/GLFW/include -IC:/include/freetype2/include -LC:/include/GLFW/lib-mingw-w64 -LC:/include/freetype2/build -lglfw3 -lopengl32 -lgdi32 -lfreetype -std=c++20
else
#	Packages: libglm-dev, libglfw-dev, libfreetype6-dev and glad
    $(info Building on Linux OS)
	DEBUG_FLAGS   = -Og -DDEBUG -Wall -Wextra -I$(CURDIR) $(shell pkg-config --cflags freetype2) -std=c++20
	RELEASE_FLAGS = -O2 -I$(CURDIR) $(shell pkg-config --cflags freetype2) -std=c++20
    LDFLAGS       = -I/usr/include/glm/ -I$(CURDIR) -lglfw $(shell pkg-config --libs freetype2) -std=c++20
endif

debug: check-os
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: $(TARGET)

release: check-os
release: CXXFLAGS = $(RELEASE_FLAGS)
release: $(TARGET)

# Link
$(TARGET): $(OBJ_C) $(OBJ_CPP)
	g++ $^ -o $@ $(LDFLAGS)

# Compile c
$(OBJDIR)/%.o: %.c | create-dirs
	@echo ""
	gcc $(CXXFLAGS) -c $< -o $@

# Compile cpp
$(OBJDIR)/%.o: %.cpp | create-dirs
	@echo ""
	g++ $(CXXFLAGS) -c $< -o $@

create-dirs:
	@echo "Creating directories: $(dir $(OBJ_C)) $(dir $(OBJ_CPP))"
	@mkdir -p $(sort $(dir $(OBJ_C) $(OBJ_CPP))) 2>/dev/null