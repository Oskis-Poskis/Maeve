cmake_minimum_required(VERSION 3.5.0)
project(Maeve VERSION 0.1.0 LANGUAGES C CXX)
#cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

#set(CMAKE_CXX_EXTENSIONS off)

project(maeve)

find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES "${SOURCE_DIR}/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} src/main.cpp ../lib/glad/glad.c ../lib/glad/glad.h)

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC glfw glad::glad glm::glm fmt::fmt Freetype::Freetype)