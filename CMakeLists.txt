cmake_minimum_required(VERSION 3.28)

project(AStarLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_USE_SYSTEM_DEPS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/3.1.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)
FetchContent_MakeAvailable(SFML)

add_library(astarlab_core
    src/AStar.cpp
    src/Graph.cpp
)

target_include_directories(astarlab_core PUBLIC include)

add_executable(AStarLab
    src/main.cpp
    src/Application.cpp
    assets/Inter-Variable.ttf
    assets/Inter-OFL.txt
)

target_include_directories(AStarLab PRIVATE include)
target_link_libraries(AStarLab PRIVATE astarlab_core SFML::Graphics)
target_compile_definitions(AStarLab PRIVATE
    ASTARLAB_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/Inter-Variable.ttf"
)

if(APPLE)
    set_source_files_properties(
        assets/Inter-Variable.ttf
        assets/Inter-OFL.txt
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
    set_target_properties(AStarLab PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "AStarLab"
        MACOSX_BUNDLE_GUI_IDENTIFIER "dev.astarlab.visualizer"
    )
endif()

enable_testing()

add_executable(astarlab_tests tests/astar_tests.cpp)
target_link_libraries(astarlab_tests PRIVATE astarlab_core)
add_test(NAME astarlab_tests COMMAND astarlab_tests)

