cmake_minimum_required (VERSION 2.6)


include(FindPkgConfig)


project(game)

find_package(SDL REQUIRED)

include_directories(
   ${SDL_INCLUDE_DIR}
)

add_executable(game WIN32
   MEMMGR.C
   IDLIBC.C
   IDLIBC_SDL.C
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
   opengl32.lib
   ${SDL_LIBRARY}
   ${SDLMAIN_LIBRARY}
)

#project (hovertank)
#add_executable(hovertank
#   main.c
#)
#target_link_libraries(hovertank
#   ${SDL_LIBRARY}
#   ${SDLMAIN_LIBRARY}
#   game
#)


