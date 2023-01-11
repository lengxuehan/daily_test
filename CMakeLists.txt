cmake_minimum_required(VERSION 3.10)
project(daily_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g2  -Wall -g2 -ggdb")

add_executable(${PROJECT_NAME} main.cpp tests/configurationmanage.cpp)

include_directories(
        3d/include
)

link_directories(/usr/local/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE sqlcipher jsoncpp pthread)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DSQLITE_HAS_CODEC)