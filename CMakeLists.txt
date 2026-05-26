cmake_minimum_required(VERSION 3.11)

project(Mario VERSION 4.7.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread" )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Mario_SOURCE_DIR}/cmake")

include(FetchContent)

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
    source/objects/bullet.hpp
    source/objects/bullet.cpp
    source/objects/pipes.hpp
)

find_package(SDL2 REQUIRED)
find_package(SDL2_Image REQUIRED)
find_package(SDL2_Mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

find_package(Box2D QUIET)
if(NOT Box2D_FOUND)
    FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG v2.4.1
    )
    set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
    set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(box2d)
endif()

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})

add_executable(${CMAKE_PROJECT_NAME} ${sources})

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_MIXER_LIBRARY}
    ${SDL2_TTF_LIBRARY}
)

if(TARGET Box2D::Box2D)
    target_link_libraries(${CMAKE_PROJECT_NAME} Box2D::Box2D)
elseif(TARGET box2d)
    target_link_libraries(${CMAKE_PROJECT_NAME} box2d)
elseif(Box2D_LIBRARIES)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${Box2D_INCLUDE_DIRS})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${Box2D_LIBRARIES})
else()
    message(FATAL_ERROR "Box2D was not found and could not be fetched")
endif()
