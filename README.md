# Minecraft-- (aka yet another C++ voxel clone)

A high-performance Minecraft recreation in C++ using the **Vulkan API**. This project is a deep dive into modern graphics programming, voxel engine architecture, and systems engineering.

### 🚀 Current Status

We have moved from a simple "Hello Triangle" to a foundational 3D engine!

- **Vulkan 3D Pipeline**: Complete with Depth Buffering, Uniform Buffer Objects (UBOs), and Descriptor Sets.
- **Voxel Data**: A 16x16x512 `Chunk` structure is implemented with block-type data.
- **Dynamic Meshing**: A CPU-side mesher that builds vertex data from voxel data and uploads it to the GPU.
- **FPS Camera**: Standard WASD + Mouse controls with smooth movement and friction.
- **Dear ImGui Integration**: A professional UI layer for menus and debug overlays.
- **Enhanced Build Pipeline**: `run.sh` now supports parallel builds and colorful terminal feedback.

### 🛠️ How to Build

You will need **CMake**, a **C++20 compiler**, and the **Vulkan SDK**.

```bash
./run.sh
```

---

### 🗺️ The Roadmap (Upcoming Tasks)

#### Phase 1: Engine Foundation & Performance (Priority: High)
*Essential work to keep the engine fast and scalable.*
- [ ] **Face Culling**: Update the mesher to skip rendering faces that are hidden between two solid blocks (Crucial for performance).
- [ ] **Project Modularization**: Continue moving logic out of `main.cpp` into dedicated modules (`Core/`, `Renderer/`, `World/`, `Camera/`).
- [ ] **Multiplayer-Ready Architecture**: Separate the **Simulation (Server)** from the **Rendering (Client)** logic to make future networking seamless.

#### Phase 2: World Generation & Visuals (Priority: Medium)
*Transforming the "colored blocks" into a recognizable world.*
- [ ] **Texture Mapping**: Implement a Texture Array or Atlas to replace solid vertex colors with block textures.
- [ ] **Procedural Terrain**: Integrate a noise library (e.g., FastNoiseLite) for realistic hills and valleys.
    - [ ] **Advanced Terrain**: Support for mountains, oceans, and lakes using a seed-based generator.
- [ ] **Vegetation**: Add basic trees (Trunks + Leaves) to the world generation.
- [ ] **Lighting System**: Implement basic ambient occlusion or directional sunlight.
    - [ ] Add lighting system.
- [ ] **UI/UX Polish**:
    - [ ] Loading screen when starting the program (while the terrain loads).

#### Phase 3: Gameplay Mechanics (Priority: Medium)
*Making it an interactive experience.*
- [ ] **Block Interaction**: Implement raycasting to allow players to place/remove blocks with a mouse click.
- [ ] **Physics Engine**: Add gravity and AABB (Axis-Aligned Bounding Box) collision detection against the voxel grid.
    - [ ] Gravity affected player.

#### Phase 4: World Scaling & Advanced Graphics (Priority: Future)
*Scaling the game to "infinite" proportions and adding polish.*
- [ ] **Infinite World**: Dynamic chunk loading/unloading and generation as the player moves.
    - [ ] Chunk generation: Generate chunks when you are close to the edge.
- [ ] **Translucent Blocks**: Sorted transparency for water and glass rendering.

---

**Note:** The project is currently transitioning from a single-file prototype to a modular engine. 🏗️
