cmake_minimum_required(VERSION 3.5.0)
project(Maeve VERSION 0.1.0 LANGUAGES C CXX)
cmake_policy(SET CMP0072 NEW)

find_package(OpenGL REQUIRED)
find_package(FreeType REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS off)

# Glob all cpp files in src/ directory
file(GLOB_RECURSE SOURCES src/*.cpp)

# Add the executable with all the sources
add_executable(${PROJECT_NAME} ${SOURCES} ../lib/glad/glad.c ../lib/glad/glad.h)

target_link_libraries(${PROJECT_NAME}
                      glfw
                      OpenGL::GL
                      Freetype::Freetype)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/$<0:>)