cmake_minimum_required(VERSION 3.10)
project(daily_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -rdynamic")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g2  -Wall -g2 -ggdb")

include_directories(
    3d/include
)

file(GLOB_RECURSE PROJECT_tests ${PROJECT_SOURCE_DIR}/tests/*.cpp)
file(GLOB_RECURSE PROJECT_grammar ${PROJECT_SOURCE_DIR}/grammar/*.cpp)
file(GLOB_RECURSE PROJECT_advance ${PROJECT_SOURCE_DIR}/advance/*.cpp)

list(APPEND PROJECT_SOURCES ${PROJECT_tests})
list(APPEND PROJECT_SOURCES ${PROJECT_grammar})
list(APPEND PROJECT_SOURCES ${PROJECT_advance})

link_directories(/usr/local/lib)

message(${PROJECT_SOURCES})

add_executable(daily_test main.cpp ${PROJECT_SOURCES})

target_link_libraries(daily_test PRIVATE
    sqlcipher
    jsoncpp
    pthread
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DSQLITE_HAS_CODEC)