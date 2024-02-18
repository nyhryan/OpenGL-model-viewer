# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OpenGL")

set (CMAKE_DEBUG_POSTFIX "_d")

# set cl.exe flag to /MT in release config
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

# search lib/ for libraries
link_directories(lib)

# search these folders for headers
include_directories("include")

# openGL
find_package(OpenGL REQUIRED)
  
# add GLFW library
find_package(glfw3 CONFIG REQUIRED)

# add fmt library
find_package(fmt CONFIG REQUIRED)

# add assimp library
find_package(assimp CONFIG REQUIRED)

# add imgui library
find_package(imgui CONFIG REQUIRED)

find_package(Stb REQUIRED)

find_package(glm CONFIG REQUIRED)

# add gl3w library
add_library(gl3w "lib/gl3w.c" "include/GL/gl3w.h")

# add my custom helper library
# add_library(helper "lib/helper.cpp" "include/helper.h")
set(HELPER "${CMAKE_SOURCE_DIR}/lib/helper.cpp" "${CMAKE_SOURCE_DIR}/include/helper.h")

# Include sub-projects.
add_subdirectory ("src/MyProgram")

