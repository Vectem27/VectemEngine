# Vectem Engine

A homemade game engine using Vulkan and GLFW.

## Linux dependencies

**How to install Linux development dependencies :**

Ubuntu / Debian

```bash
sudo apt install libxcb1-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-xinerama0-dev libxcb-xfixes0-dev \
                 libxcb-shape0-dev libxcb-sync-dev libwayland-dev wayland-protocols libxkbcommon-dev libx11-dev \
                 libxrandr-dev libxi-dev libxxf86vm-dev libxcursor-dev libxinerama-dev
```

Arch Linux

```bash
sudo pacman -S libxcb wayland wayland-protocols libxkbcommon libx11 libxrandr libxi libxcursor libxinerama
```
     
Fedora

```bash
sudo dnf install libxcb-devel \
                 wayland-devel wayland-protocols-devel libxkbcommon-devel \
                 libX11-devel libXrandr-devel libXi-devel \
                 libXcursor-devel libXinerama-devel
```