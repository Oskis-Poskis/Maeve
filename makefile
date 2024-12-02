SRC_C   = lib/glad/glad.c
SRC_CPP = $(shell find src -type f -name '*.cpp')

OBJDIR = .obj
OBJ_C = $(addprefix $(OBJDIR)/, $(SRC_C:.c=.o))
OBJ_CPP = $(addprefix $(OBJDIR)/, $(SRC_CPP:.cpp=.o))

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

$(TARGET): $(OBJ_C) $(OBJ_CPP)
	g++ $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: %.c | create-dirs
	gcc $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cpp | create-dirs
	g++ $(CXXFLAGS) -c $< -o $@

create-dirs:
	@mkdir -p $(dir $(OBJ_C) $(dir $(OBJ_CPP)))

clean:
	@find $(OBJDIR) -name "*.o" -exec rm -f {} \;
	@rm -rf $(OBJDIR) $(TARGET)