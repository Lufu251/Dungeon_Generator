cmake_minimum_required(VERSION 3.23)
project(dungeon_generator)

# toggle libs to be used with ON and OFF
option(USE_RAYLIB "Use raylib" ON)
option(USE_DELABELLA "Use delabella" ON)

# Fetch and add libraries
set(LINK_LIBRARIES "")  # libraries to link against
include(FetchContent) # use fetch content

# Fetch raylib
if(USE_RAYLIB)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  set(BUILD_GAMES OFF CACHE BOOL "" FORCE) # don't build the supplied example games
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(CUSTOMIZE_BUILD OFF CACHE BOOL "" FORCE) 

  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.2
  )

  FetchContent_MakeAvailable(raylib)

  list(APPEND LINK_LIBRARIES raylib)
endif()

# Fetch delabella
if(USE_DELABELLA)
  FetchContent_Declare(
      delabella
      GIT_REPOSITORY https://github.com/msokalski/delabella.git
      GIT_TAG        master
  )

  FetchContent_MakeAvailable(delabella)

  list(APPEND LINK_LIBRARIES delabella)
endif()

file(GLOB PROJECT_SOURCES "src/*.cpp")
file(GLOB PROJECT_INCLUDE "src")

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})