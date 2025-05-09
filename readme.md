# Maeve  
**WIP Custom Game Engine**

The project's **Makefile** should work on both Windows and Linux, though Windows requires a specific include setup that will be addressed in the future.

---

### **Known Issues**  
Using alt as to move the camera can lock sometimes unless you disable the setting "Disable Mouse/Touchpad while typing". Not certain of this setting outside of gnome..

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
