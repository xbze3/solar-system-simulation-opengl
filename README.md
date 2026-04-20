# OpenGL Solar System Simulation

This project is an OpenGL-based solar system simulation built using **GLFW**, **GLAD**, and **GLM**.

## Getting Started

### Clone or Download the Repository

```bash
git clone https://github.com/xbze3/solar-system-simulation-opengl
```

Or download the ZIP and extract it.

---

### Open the Project

- Navigate to the project folder
- Open the `.sln` (solution) file in **Visual Studio**

---

### Build and Run

- Select **Debug** or **Release**
- Press **F5** or click **Local Windows Debugger**

---

## Dependencies

All required libraries are already included in the project:

- **GLFW**
- **GLAD**
- **GLM**

No additional installations are required.

---

## Controls

Current camera controls:

- **W** - Move forward
- **S** - Move backward
- **A** - Move left
- **D** - Move right
- **ESC** - Exit

---

## Notes

- The project uses modern OpenGL (Core Profile 3.3)
- GLM is included locally, so no external setup is needed
- Make sure the project builds successfully before running

---

## Troubleshooting

If the project does not build:

- Ensure you opened the `.sln` file (not just the folder)
- Make sure you are using **Visual Studio (not VS Code)**
- Try **Build → Rebuild Solution**

---

## Future Work

- Add sphere rendering (planets)
- Implement orbital motion
- Add lighting and textures
- Improve camera (mouse look)
