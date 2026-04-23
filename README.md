# OpenGL Solar System Simulation

This project is an interactive **3D solar system simulator** built using **modern OpenGL (Core Profile 3.3)** with **GLFW**, **GLAD**, and **GLM**.

It demonstrates real-time rendering, camera controls, transformations, lighting, and hierarchical motion (planets and moons).

---

## Features

### 🌌 Rendering

- Procedurally generated **UV sphere mesh** (used for all planets)
- Per-fragment lighting (ambient + diffuse)
- Reusable mesh system (one sphere, multiple draws)

### 🪐 Solar System

- Multiple planets rendered using a **data-driven system (`Body` struct + vector)**
- Planets:
    - Orbit the Sun (revolution)
    - Rotate on their own axis (rotation)
- **Moon orbits Earth** (hierarchical transformation)
- **Saturn has a ring** (custom ring mesh)
- **Orbit paths visualized** using circular line rendering

### 🎮 Camera System

- First-person style camera:
    - Mouse controls **yaw & pitch**
    - WASD controls movement
- Supports **free-flight movement** (including vertical movement)
- Toggle mouse capture:
    - **TAB** → lock/unlock cursor
- Smooth movement using **delta time**

### 📐 Transformations

- Model → View → Projection pipeline
- Each object uses:
    - Translation (orbit)
    - Rotation (spin)
    - Scale (size)

### 📊 Projection

- Perspective camera with adjustable FOV
- Dynamic aspect ratio handling (window resize safe)

---

## Getting Started

### Clone the Repository

```bash
git clone https://github.com/xbze3/solar-system-simulation-opengl
```

---

### Open the Project

- Navigate to the project folder
- Open the `.sln` file in **Visual Studio**

---

### Build and Run

- Select **Debug** or **Release**
- Press **F5** or click **Local Windows Debugger**

---

## Dependencies

All dependencies are included locally:

- GLFW (window + input)
- GLAD (OpenGL loader)
- GLM (math library)

No additional installation required.

---

## Controls

| Key   | Action                |
| ----- | --------------------- |
| W     | Move forward          |
| S     | Move backward         |
| A     | Move left             |
| D     | Move right            |
| Mouse | Look around           |
| TAB   | Toggle cursor capture |
| ESC   | Exit                  |

---

## Implementation Highlights

### Data-Driven Planet System

Planets are defined using a struct:

```cpp
struct Body {
    std::string name;
    float scaledSize;
    float orbitRadius;
    float rotationSpeed;
    float revolutionSpeed;
    glm::vec3 color;
};
```

---

### Reusable Mesh Strategy

- One sphere mesh is generated and reused for all planets
- Reduces memory usage and improves performance

---

### Orbit Mechanics

Each planet’s position is computed using:

```cpp
x = cos(angle) * radius
z = sin(angle) * radius
```

---

### Moon System

- Moon position is calculated relative to Earth’s position
- Demonstrates hierarchical transformation

---

### Saturn Ring

- Custom ring mesh (inner + outer radius)
- Rendered separately from the sphere
- Slight tilt applied

---

### Orbit Visualization

- Circular line loops used to visualize planetary paths

---

## Notes

- Uses **modern OpenGL pipeline**
- Designed for clarity and learning
- Uses a **visual (non-realistic) scale**

---

## Troubleshooting

- Open the `.sln` file
- Use Visual Studio
- Rebuild solution if needed

---

## Future Improvements

- Textures
- Skybox
- Better lighting
- Elliptical orbits
- Zoom control

---

## License

Educational use.
