cmake_minimum_required(VERSION 3.31.6)
project(sandboxQt)

add_compile_options(-O2)
#add_compile_options(-march=native)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(src)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(sandboxQt main.cpp
        src/SandboxWorld.cpp
        src/SandboxWorld.h
        src/SandboxInput.cpp
        src/SandboxInput.h
        src/SandboxRenderer.cpp
        src/SandboxRenderer.h)

target_link_libraries(sandboxQt
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
