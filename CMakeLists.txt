cmake_minimum_required(VERSION 3.20)

project(minecraft_vk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
  set(GLFW_SEARCH_HINTS
    /opt/homebrew
    /opt/homebrew/opt/glfw
    /usr/local
    /usr/local/opt/glfw
  )
  set(VULKAN_SEARCH_HINTS
    /opt/homebrew
    /opt/homebrew/opt/molten-vk
    /usr/local
    /usr/local/opt/molten-vk
  )
endif()

find_path(GLM_INCLUDE_DIR
  NAMES glm/glm.hpp
  HINTS
    /opt/homebrew
    /usr/local
  PATH_SUFFIXES include
)

if(NOT GLM_INCLUDE_DIR)
  message(FATAL_ERROR "GLM was not found. Install it with Homebrew or point CMake at your GLM package.")
endif()

set(GLFW_TARGET "")
find_package(glfw3 CONFIG QUIET)
if(TARGET glfw)
  set(GLFW_TARGET glfw)
elseif(TARGET glfw::glfw)
  set(GLFW_TARGET glfw::glfw)
else()
  find_path(GLFW_INCLUDE_DIR
    NAMES GLFW/glfw3.h
    HINTS
      ${GLFW_SEARCH_HINTS}
    PATH_SUFFIXES include
  )
  find_library(GLFW_LIBRARY
    NAMES glfw glfw3
    HINTS
      ${GLFW_SEARCH_HINTS}
    PATH_SUFFIXES lib
  )

  if(NOT GLFW_INCLUDE_DIR OR NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW was not found. Install it with Homebrew or point CMake at your GLFW package.")
  endif()

  add_library(glfw UNKNOWN IMPORTED)
  set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
  )
  set(GLFW_TARGET glfw)
endif()

set(VULKAN_TARGET "")
find_package(Vulkan QUIET)
if(TARGET Vulkan::Vulkan)
  set(VULKAN_TARGET Vulkan::Vulkan)
else()
  find_path(VULKAN_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    HINTS
      ${VULKAN_SEARCH_HINTS}
    PATH_SUFFIXES include
  )
  find_library(VULKAN_LIBRARY
    NAMES vulkan Vulkan
    HINTS
      ${VULKAN_SEARCH_HINTS}
    PATH_SUFFIXES lib
  )

  if(NOT VULKAN_INCLUDE_DIR OR NOT VULKAN_LIBRARY)
    message(FATAL_ERROR "Vulkan was not found. Install the Vulkan SDK or Homebrew molten-vk, vulkan-headers, and vulkan-loader.")
  endif()

  add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
  set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_LOCATION "${VULKAN_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIR}"
  )
  set(VULKAN_TARGET Vulkan::Vulkan)
endif()

find_program(GLSLANG_VALIDATOR_EXECUTABLE
  NAMES glslangValidator
  HINTS
    /opt/homebrew/bin
    /usr/local/bin
)

if(NOT GLSLANG_VALIDATOR_EXECUTABLE)
  message(FATAL_ERROR "glslangValidator was not found. Install glslang or point CMake at your shader compiler.")
endif()

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(SHADER_SOURCES
  "${SHADER_SOURCE_DIR}/square.vert"
  "${SHADER_SOURCE_DIR}/square.frag"
)
set(SHADER_OUTPUTS "")

foreach(SHADER_SOURCE IN LISTS SHADER_SOURCES)
  get_filename_component(SHADER_NAME "${SHADER_SOURCE}" NAME)
  set(SHADER_OUTPUT "${SHADER_BINARY_DIR}/${SHADER_NAME}.spv")
  add_custom_command(
    OUTPUT "${SHADER_OUTPUT}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_BINARY_DIR}"
    COMMAND "${GLSLANG_VALIDATOR_EXECUTABLE}" -V "${SHADER_SOURCE}" -o "${SHADER_OUTPUT}"
    DEPENDS "${SHADER_SOURCE}"
    VERBATIM
  )
  list(APPEND SHADER_OUTPUTS "${SHADER_OUTPUT}")
endforeach()

add_custom_target(compile_shaders DEPENDS ${SHADER_OUTPUTS})

include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC ${GLFW_TARGET} ${VULKAN_TARGET})

add_executable(minecraft_vk
  src/main.cpp
)

add_dependencies(minecraft_vk compile_shaders)

target_compile_definitions(minecraft_vk PRIVATE
  MINECRAFT_VK_SHADER_DIR="${SHADER_BINARY_DIR}"
)

target_link_libraries(minecraft_vk PRIVATE
  imgui
  ${GLFW_TARGET}
  ${VULKAN_TARGET}
)

target_include_directories(minecraft_vk PRIVATE
  ${GLM_INCLUDE_DIR}
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(APPLE)
  target_link_libraries(minecraft_vk PRIVATE
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework QuartzCore"
    "-framework Metal"
    "-framework IOSurface"
  )
endif()
