cmake_minimum_required(VERSION 3.2)

project(Mario VERSION 4.7.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread" )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Mario_SOURCE_DIR}/cmake")

message (${CMAKE_MODULE_PATH})
set(sources
    main.cpp
    source/rsdl.cpp
    source/rsdl.hpp
    source/core.cpp
    source/core.hpp
    source/map.cpp
    source/map.hpp
    source/world.hpp
    source/world.cpp
    source/player.hpp
    source/player.cpp
    source/physics.hpp
    source/physics.cpp
    source/camera.hpp
    source/camera.cpp
    source/audio.hpp
    source/audio.cpp
    source/objects/object.cpp
    source/objects/object.hpp
    source/objects/block.hpp
    source/objects/ground.hpp
    source/objects/brick.hpp
    source/objects/coin_container.hpp
    source/objects/fire_block.hpp
    source/objects/health_block.hpp
    source/objects/goomba.hpp
    source/objects/goomba.cpp
    source/objects/koopa_troopa.hpp
    source/objects/koopa_troopa.cpp
    source/objects/mushroom.hpp
    source/objects/mushroom.cpp
    source/objects/flower.hpp
    source/objects/flower.cpp
    source/objects/pipes.hpp
)

find_package(SDL2 REQUIRED)
find_package(SDL2_Image REQUIRED)
find_package(SDL2_Mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})

add_executable(${CMAKE_PROJECT_NAME} ${sources})
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY})




