cmake_minimum_required(VERSION 3.20)
project(AStarLabWeb LANGUAGES CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Build this target with Emscripten: emcmake cmake -S web -B build-web")
endif()

add_executable(astar-core
    wasm_bridge.cpp
    ../src/AStar.cpp
    ../src/Graph.cpp
)
target_include_directories(astar-core PRIVATE ../include)
target_compile_features(astar-core PRIVATE cxx_std_17)
set_target_properties(astar-core PROPERTIES SUFFIX ".js")
target_link_options(astar-core PRIVATE
    "--no-entry"
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=createAStarCore"
    "-sENVIRONMENT=web"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sEXPORTED_RUNTIME_METHODS=['ccall']"
    "-sEXPORTED_FUNCTIONS=['_reset_graph','_add_node','_add_edge','_run_search','_result_found','_result_cost','_result_expanded_count','_result_path_count','_result_path_node','_result_step_count','_step_type','_step_line','_step_current','_step_neighbor','_step_edge_cost','_step_tentative_g','_step_previous_g','_step_improved','_step_score_node','_step_score_g','_step_score_h','_step_score_f']"
)
