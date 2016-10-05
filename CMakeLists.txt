cmake_minimum_required (VERSION 2.8)


include(FindPkgConfig)

project(game)

find_package(OpenGL REQUIRED)

if (WIN32)
   find_file(SDL2_INCLUDE_DIRS NAME SDL.h HINTS SDL2)
   find_library(SDL2_LIBRARIES NAME SDL2)
   find_library(SDL2MAIN_LIBRARIES NAME SDL2)
endif (WIN32)

if (UNIX)
   pkg_search_module(SDL2 REQUIRED sdl2)
endif (UNIX)

include_directories(
   ${SDL2_INCLUDE_DIRS}
)

add_executable(game WIN32
   MEMMGR.C
   IDLIBC.C
   IDLIBC_SDL.C
   IDLIBC_SHADERS.C
   IDLIBC_GL.C
   HOVDRAW.C
   HOVTRACE.C
   HOVSCALE.C
   HOVLOOP.C
   HOVACTS.C
   JM_SB.C
   HOVMAIN.C
   matrix.cpp
)

target_link_libraries(game
   ${SDL2_LIBRARIES}
   ${SDL2MAIN_LIBRARIES}
   ${OPENGL_LIBRARIES}
)
