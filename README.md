# Minecraft-- (aka yet another C++ voxel clone)

A high-performance Minecraft recreation in C++ using the **Vulkan API**. This project is a deep dive into modern graphics programming, voxel engine architecture, and systems engineering.

### 🚀 Current Status

We have moved from a simple "Hello Triangle" to a foundational 3D engine!

- **Vulkan 3D Pipeline**: Complete with Depth Buffering, Uniform Buffer Objects (UBOs), and Descriptor Sets.
- **Voxel Data**: A 16x16x512 `Chunk` structure is implemented with block-type data.
- **Optimized Dynamic Meshing**: A CPU-side mesher with **Face Culling** (only rendering visible faces).
- **Dynamic World Generation**: Real-time chunk spawning within an 8-chunk radius around the player.
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
- [x] **Face Culling**: Optimized the mesher to skip rendering faces that are hidden between two solid blocks.
- [ ] **Project Modularization**: Continue moving logic out of `main.cpp` into dedicated modules (`Core/`, `Renderer/`, `World/`, `Camera/`).
- [ ] **Multiplayer-Ready Architecture**: Separate the **Simulation (Server)** from the **Rendering (Client)** logic to make future networking seamless.

#### Phase 2: World Generation & Visuals (Priority: Medium)
*Transforming the "colored blocks" into a recognizable world.*
- [ ] **Texture Mapping**: Implement a Texture Array or Atlas to replace solid vertex colors with block textures.
- [ ] **Procedural Terrain**: Integrate a noise library (e.g., FastNoiseLite) for realistic hills and valleys.
    - [ ] **Advanced Terrain**: Support for mountains, oceans, and lakes using a seed-based generator.
- [ ] **Vegetation**: Add basic trees (Trunks + Leaves) to the world generation.
- [ ] **Lighting System**: Implement basic ambient occlusion or directional sunlight.

#### Phase 3: Gameplay Mechanics (Priority: Medium)
*Making it an interactive experience.*
- [ ] **Block Interaction**: Implement raycasting to allow players to place/remove blocks with a mouse click.
- [ ] **Physics Engine**: Add gravity and AABB (Axis-Aligned Bounding Box) collision detection against the voxel grid.

#### Phase 4: World Scaling & Advanced Graphics (Priority: Future)
*Scaling the game to "infinite" proportions and adding polish.*
- [x] **Dynamic Chunk Generation**: Chunks are now spawned and meshed based on player proximity.
- [ ] **Infinite World**: Dynamic chunk unloading (despawning) when moving away.
- [ ] **Translucent Blocks**: Sorted transparency for water and glass rendering.

---

**Note:** The project is currently transitioning from a single-file prototype to a modular engine. 🏗️
