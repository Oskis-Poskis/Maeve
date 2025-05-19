[![YouTube](https://img.shields.io/badge/YouTube-Subscribe-red?logo=youtube)](https://www.youtube.com/@OskarKassander/videos)
[![Website](https://img.shields.io/badge/Website-Visit-blue?logo=google-chrome)](https://kassander.com)

# Maeve  
Maeve is a passion project based on my personal ideas and desires for a 3D engine and scene editor. The goal is to make a minimal UI experience that provides maximum screenspace and an efficient workflow using intuitive shortcuts. It is also a learning experience for C++ and build systems and there will probably be a rewrite in the future

Game engine essentials such as animations, scripting, audio, physics, ai etc is a long way to go and right now I'm just defining the principles and design language while building the core.

Feature list and todo list available at end of readme.

The project's **Makefile** should work on both Windows and Linux, though Windows requires a specific include setup that will be addressed in the future...

---

### **Linux Setup**  
On Linux, you’ll need to install the required libraries, if your distro doesn't come with them by default:
```
glfw3, glm, freetype2
```

#### **1. Arch-based Systems (Arch Linux, Manjaro, etc.)**
```bash
sudo pacman -S glfw glm
```

#### **1. Debian-based Systems (Ubuntu, Debian, Mint, etc.)**
```bash
sudo apt update
sudo apt install libglm-dev libglfw3-dev libfreetype6-dev
```

#### **2. One of the following libraries/tools for native file dialogs (used by `tinyfiledialogs`)**
- **zenity** — GTK-based (recommended)
- **AppleScript** — macOS only
- **kdialog** — for KDE-based systems
- **yad** — Yet Another Dialog
- **Xdialog** — old X11 dialog system
- **matedialog**, **shellementary**, or **qarma**
- **python (2 or 3)** + **tkinter** (+ optional **python-dbus**) — uses Tk GUI

#### 3. Build
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

---

### **Known Issues**  
Using alt to move the camera can break unless you disable the setting "Disable Mouse/Touchpad while typing". Not certain of this setting outside of gnome..

---

### **Doto list**
- Multi node selection
- Wireframe mode
- Preview GBuffers?
- Create directional light type
    - Move dir shadow mapping into light instead of it being global
    - Create dir light "C" menu
- Create spotlight type
- Point light shadows
- Implement material system
    - Add texture loading to asset manager
- Create object "C" menu
- Fix PCSS shadow mapping to work with cascades
- Grid drawing
    - Some kindof grid drawing for axis when translating objects
- Offload file dialog to other thread to avoid freezing application
- Implement bloom
- Implement SSAO
- Implement some anti-aliasing method, FXAA, SMAA etc
- Implement post-processing controls + color grading

### **Done**
- .obj file parsing ✅
- Basic GBuffer setup for deferred rendering, viewspace pos reconstruction in shading stage ✅
- PBR shading ✅
- Text rendering with freetype2 ✅
- Per mesh BVH ✅
- Object outline for selection ✅
- Node selection using BVH raycast (single at a time for now), both lights and objects ✅
- Object manipulation (translate, rotate, scale) with per axis control with hotkeys ✅
- Auto instanced meshes ✅
- Directional light shadow mapping ✅
- Fixed update for input ✅