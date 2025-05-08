# Maeve  
Custom game engine hobby project WIP

The project’s Makefile shoooooould work on both Windows and Linux, though Windows requires a specific include setup that will be addressed in the future.

### Windows Setup:  
For Windows, make sure to place the required libraries in the following directories:  
- C:/include/glm
- C:/include/GLFW/include  
- C:/include/GLFW/lib-mingw-w64
- C:/include/freetype2/include 
- C:/include/freetype2/build

### Linux Setup:  
On Linux, you’ll need to install the following libs:

Arch:
- `glfw`
- `glm`
    
Debian:
    `libglm-dev`
    `libglfw3-dev`
    `libfreetype6-dev`

### Build Instructions:  
Once dependencies are set up, simply run:

- `make -j`
- Or `make -j release`