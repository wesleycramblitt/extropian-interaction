cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(extropian-interaction VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "Extropian Interaction: event-driven edit logic, data binding, chart edits, zoom/navigation"
)

# ── Pre‑define desktop‑only targets (render fetches exd‑app unconditionally) ──
if(NOT TARGET exd::app)
    add_library(exd-app INTERFACE)
    add_library(exd::app ALIAS exd-app)
endif()

# ── Dependencies ──
include(FetchContent)

# extropian-core: ECS, math
if(NOT TARGET exd::core)
    if(DEFINED EXD_CORE_DIR)
        FetchContent_Declare(exd-core SOURCE_DIR "${EXD_CORE_DIR}")
    else()
        FetchContent_Declare(exd-core
            GIT_REPOSITORY https://github.com/wesleycramblitt/extropian-core.git
            GIT_TAG main
        )
    endif()
    FetchContent_MakeAvailable(exd-core)
endif()

# extropian-geometry: Bounds, primitives
if(NOT TARGET exd::geometry)
    if(DEFINED EXD_GEOMETRY_DIR)
        FetchContent_Declare(exd-geometry SOURCE_DIR "${EXD_GEOMETRY_DIR}")
    else()
        FetchContent_Declare(exd-geometry
            GIT_REPOSITORY https://github.com/wesleycramblitt/extropian-geometry.git
            GIT_TAG main
        )
    endif()
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_DEMO  OFF CACHE BOOL "" FORCE)
    set(ENABLE_TEXT  OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(exd-geometry)
endif()

# extropian-render: PickerSystem, CameraSystem, selection/hover components
if(NOT TARGET exd::render)
    if(DEFINED EXD_RENDER_DIR)
        FetchContent_Declare(exd-render SOURCE_DIR "${EXD_RENDER_DIR}")
    else()
        FetchContent_Declare(exd-render
            GIT_REPOSITORY https://github.com/wesleycramblitt/extropian-render.git
            GIT_TAG main
        )
    endif()
    set(EXD_RENDER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(EXD_RENDER_BUILD_DEMO  OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(exd-render)
endif()

# ── Library target (STATIC) ──
add_library(exd-interaction STATIC
    src/data_graph.cpp
    src/binding.cpp
    src/binding_resolver.cpp
    src/command.cpp
    src/edit_ops.cpp
    src/event.cpp
    src/hit.cpp
    src/gestures.cpp
    src/interaction_state.cpp
    src/focus.cpp
    src/camera_framing.cpp
    src/view_sync.cpp
)

add_library(exd::interaction ALIAS exd-interaction)

# ── Include paths ──
target_include_directories(exd-interaction PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# ── Link dependencies ──
target_link_libraries(exd-interaction PUBLIC
    exd::core
    exd::geometry
    exd::render
)

# ── Tests ──
option(EXD_INTERACTION_BUILD_TESTS "Build interaction tests" OFF)
if(EXD_INTERACTION_BUILD_TESTS)
    enable_testing()

    if(NOT TARGET doctest::doctest_with_main)
        FetchContent_Declare(doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v2.4.11
        )
        FetchContent_MakeAvailable(doctest)
    endif()

    add_subdirectory(tests)
endif()

# ── Demo ──
option(BUILD_DEMO "Build interaction demo" OFF)
if(BUILD_DEMO)
    add_subdirectory(demo)
endif()
