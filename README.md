# Minecraft-- (aka yet another C++ voxel clone)

Okay so basically the goal is to make a recreation of Minecraft in C++. I've been wanting to learn Vulkan for a while and figured this is the best way to force myself.

Right now it literally just draws a bouncing square because the Vulkan boilerplate took me like 3 days to get working lol. 

But **eventually** it'll have:
- Chunks (theoretically infinite?? idk how hard that actually is)
- Blocks (dirt, stone, maybe glass if I can figure out translucent sorting)
- A camera that doesn't make you sick
- Multiplayer??? (jk probably not)

### How to build
You're gonna need CMake and the Vulkan SDK. If you're on a Mac like me, good luck fighting with MoltenVK portability stuff, it's a pain in the ass but it works eventually. 

Just standard cmake stuff:
```bash
mkdir build
cd build
cmake ..
make
```

### Current Status
- [x] Copy paste Vulkan tutorial code
- [x] Draw a square
- [ ] Make the square into a 3D cube
- [ ] Textures?
- [ ] Everything else

**note:** The code is a bit of a mess rn. `HelloApp` is still the main class name because I'm too scared to break the build by renaming it yet `¯\_(ツ)_/¯` If it works don't touch it.
