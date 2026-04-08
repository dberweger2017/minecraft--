#!/bin/bash

# Exit on any error
set -e

# ANSI Color Codes
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${CYAN}🚀 Starting Minecraft Vulkan Build Pipeline...${NC}"

# 1. Create build directory
if [ ! -d "build" ]; then
    echo -e "${YELLOW}📁 Creating build directory...${NC}"
    mkdir build
fi

# 2. Configure
if [ ! -f "build/CMakeCache.txt" ]; then
    echo -e "${YELLOW}⚙️  Configuring project (First time)...${NC}"
    cmake -B build > /dev/null
else
    echo -e "${GREEN}✅ Project already configured.${NC}"
fi

# 3. Build with parallel jobs (using all available cores)
echo -e "${YELLOW}🛠️  Building project (Parallel)...${NC}"
# Use $(nproc) on Linux or sysctl on Mac for core count
if [[ "$OSTYPE" == "darwin"* ]]; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=$(nproc)
fi

cmake --build build -j $CORES

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✨ Build Successful!${NC}"
else
    echo -e "${RED}❌ Build Failed!${NC}"
    exit 1
fi

# 4. Run
echo -e "${CYAN}🎮 Launching Minecraft Vulkan...${NC}"
./build/minecraft_vk
