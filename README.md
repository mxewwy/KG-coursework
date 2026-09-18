# KG Coursework

Coursework project for computer graphics.

## Orbit Visualiser

A Win32 + OpenGL application that visualises an Earth-orbiting satellite.

### Features

- satellite orbit simulation
- adjustable orbital parameters
- orbit trace rendering
- satellite 3D model
- Earth texture and cloud layer
- Milky Way background
- satellite coverage zone
- interactive camera and light
- GLSL shaders

### Controls

- Arrow keys — change semi-major axis and eccentricity
- `+` / `-` — change inclination
- `[` / `]` — change longitude of ascending node
- `;` / `'` — change argument of periapsis
- `9` / `0` — change true anomaly
- `L` — toggle lighting
- `T` — toggle textures
- `A` — toggle transparency
- `M` — toggle Earth shader
- `B` — toggle background
- `C` — toggle coverage zone
- `F` — move the light to the camera position

### Build

Open the Visual Studio project in:

`LAB4+/KGlab/KGlab.sln`

Build with Visual Studio on Windows. The project uses OpenGL, GLU and GLSL shaders.

### Structure

```
LAB4+/
└── KGlab/
    ├── models/
    ├── shaders/
    ├── textures/
    └── *.cpp / *.h
```
