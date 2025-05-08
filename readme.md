# Maeve  
**WIP Custom Game Engine**

The project's **Makefile** should work on both Windows and Linux, though Windows requires a specific include setup that will be addressed in the future.

---

### **Linux Setup**  
On Linux, you’ll need to install the required libraries.

#### **1. Arch-based Systems (Arch Linux, Manjaro, etc.)**
```bash
sudo pacman -S glfw glm
```

#### **1. Debian-based Systems (Ubuntu, Debian, Mint, etc.)**
```bash
sudo apt update
sudo apt install libglm-dev libglfw3-dev libfreetype6-dev
```

#### 2. Build
```bash
make -j
```

---

### **Windows Setup**  
For Windows, ensure that you place the required libraries in the following directories:

- `C:/include/glm`
- `C:/include/GLFW/include`
- `C:/include/GLFW/lib-mingw-w64`
- `C:/include/freetype2/include`
- `C:/include/freetype2/build`
