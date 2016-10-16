cmake_minimum_required (VERSION 2.6)

project (NetlistSim)
SET(CMAKE_CXX_FLAGS "-g -Wall -pedantic -std=c++14")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

set(${PROJECT_NAME}_SOURCES
    src/main.cpp
    src/Parser.cpp
    src/Simulator.cpp
)

include_directories(thirdparty/lexertl)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
