# Minecraft-- (aka yet another C++ voxel clone)

A high-performance Minecraft recreation in C++ using the **Vulkan API**. This project is a deep dive into modern graphics programming, voxel engine architecture, and systems engineering.

### 🚀 Current Status

We have moved from a simple "Hello Triangle" to a foundational 3D engine!

- **Vulkan 3D Pipeline**: Complete with Depth Buffering, Uniform Buffer Objects (UBOs), and Descriptor Sets.
- **Voxel Data**: A 16x16x16 `Chunk` structure is implemented with block-type data (Grass, Dirt, Stone).
- **Dynamic Meshing**: A CPU-side mesher that builds vertex data from voxel data and uploads it to the GPU.
- **FPS Camera**: Standard WASD + Mouse controls (Space to fly up, Shift to fly down).
- **Dear ImGui Integration**: A professional UI layer for pause menus and (future) debug overlays.
- **Enhanced Build Pipeline**: `run.sh` now supports parallel builds and provides colorful terminal feedback.

### 🛠️ How to Build

You will need **CMake**, a **C++20 compiler**, and the **Vulkan SDK**.

```bash
./run.sh
```

---

### 🗺️ The Roadmap (Upcoming Tasks)

#### 1. Voxel World & Generation
- [ ] **Height-based Layering**: Implement a proper chunk-fill algorithm:
    - Top layer: **Grass** (Green).
    - Middle layers: **Dirt** (Brown).
    - Deep layers (>6 blocks): **Cobblestone** (Dark Gray).
    - Bottom-most layer: **Bedrock** (Very Dark Gray/Black).
- [ ] **Procedural Terrain**: Integrate a noise library (e.g., FastNoiseLite) for hills and valleys.
- [ ] **Face Culling (Optimization)**: Update the mesher to skip rendering faces that are hidden between two solid blocks.

#### 2. Visuals & Interaction
- [ ] **Texture Mapping**: Implement a Texture Array or Atlas to replace solid vertex colors with block textures.
- [ ] **Block Interaction**: Implement raycasting to allow players to place/remove blocks with a mouse click.
- [ ] **Lighting**: Basic ambient occlusion or directional sunlight.

#### 3. Architecture & Refactoring
- [ ] **Project Modularization**: Move logic out of `main.cpp` into dedicated modules:
    - `Core/`: Basic types and math.
    - `Renderer/`: Vulkan abstractions.
    - `World/`: Chunks and World-gen logic.
    - `Camera/`: Input and View management.
- [ ] **Multiplayer-Ready (Server/Client)**: Refactor the engine to separate the **Simulation (Server)** from the **Rendering (Client)**. Even in single-player, the game should run an internal server to make adding future networking seamless.

#### 4. Advanced Features
- [ ] **Translucent Blocks**: Water and glass rendering with sorted transparency.
- [ ] **Infinite World**: Dynamic chunk loading/unloading as the player moves.
- [ ] **Physics**: Gravity and AABB (Axis-Aligned Bounding Box) collisions.

---

**Note:** The project is currently transitioning from a single-file prototype to a modular engine. 🏗️
