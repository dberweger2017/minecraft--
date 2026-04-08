# Minecraft-- (aka yet another C++ voxel clone)

Okay so basically the goal is to make a recreation of Minecraft in C++. I've been wanting to learn Vulkan for a while and figured this is the best way to force myself.

### Current Status
It's actually starting to look like something! I've finally moved past just drawing a square. 

**What's working now:**
- **3D Cube:** It's a real spinning cube now, not just a flat 2D square. Still just a placeholder while I figure out the chunk system.
- **FPS Counter:** Added a little counter in the window title so I can see how bad my code is performing (spoiler: it's actually fine for now).
- **Pause Menu:** You can hit **Esc** to pause the game. It gives you your mouse back and you can click (roughly) in the middle of the screen to resume or slightly below that to quit. It's a bit "invisible" right now since I don't have a UI library, but it works.
- **Fixed Camera:** Finally fixed the camera rotation! It was doing that weird thing where moving right made the world move right too. Now it feels like a normal first-person camera. Use **W/A/S/D** to move and **Space/Shift** to go up/down.
- **Better Build Flow:** Created a `run.sh` script because typing out the cmake commands every time was getting annoying.

### How to build
You're gonna need CMake and the Vulkan SDK. If you're on a Mac like me, it's a bit of a fight with MoltenVK but it works.

To compile and run everything in one go, just do:
```bash
./run.sh
```
(It'll handle the build folder and everything for you).

### Todo
- [x] Make the square into a 3D cube
- [x] Working first-person camera
- [x] Basic pause/quit functionality
- [ ] **Textures!** (The cube is currently just colored faces, need to get some dirt blocks in here)
- [ ] Proper Chunks (theoretical infinite world, maybe?)
- [ ] Translucent blocks (water/glass)

**note:** The code is still a bit of a mess. `HelloApp` is still the main class name because I'm too scared to break the build by renaming it yet `¯\_(ツ)_/¯` If it works don't touch it.
