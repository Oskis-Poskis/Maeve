# Maeve  
Custom game engine hobby project

The project’s Makefile shoooooould work on both Windows and Linux, though Windows requires a specific include setup that will be addressed in the future.

### Windows Setup:  
For Windows, make sure to place the required libraries in the following directories:  
- C:/include/  
- C:/include/GLFW/include  
- C:/include/GLFW/lib-mingw-w64
- C:/include/freetype2/include 
- C:/include/freetype2/build

Include and library paths are specified in the Makefile, like so:  
- -IC:/include/GLFW/include  
- -IC:/include/freetype2/include  
- -LC:/include/GLFW/lib-mingw-w64  
- -LC:/include/freetype2/build

### Linux Setup:  
On Linux, you’ll need to install the following dependencies:  
- libglm-dev  
- libglfw-dev  
- libfreetype6-dev  
- glad

### Build Instructions:  
Once dependencies are set up, simply run:

- `make -j`
- Or `make -j release`