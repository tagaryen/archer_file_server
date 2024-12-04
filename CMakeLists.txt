
cmake_minimum_required(VERSION 3.5)


project(FILE-SERVER VERSION "1.1.0")

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE "Release")

file(GLOB_RECURSE ALL_SOURCE_FILES 
    ${PROJECT_SOURCE_DIR}/libapi/*.cpp
    ${PROJECT_SOURCE_DIR}/libcommon/*.cpp
    ${PROJECT_SOURCE_DIR}/libhandler/*.cpp
    ${PROJECT_SOURCE_DIR}/libdatabase/*.cpp
    ${PROJECT_SOURCE_DIR}/libfileserver/*.cpp
    ${PROJECT_SOURCE_DIR}/libserver/*.cpp
    ${PROJECT_SOURCE_DIR}/libservice/*.cpp)

message("sources: ${ALL_SOURCE_FILES}")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/libapi
    ${PROJECT_SOURCE_DIR}/libcommon
    ${PROJECT_SOURCE_DIR}/libhandler
    ${PROJECT_SOURCE_DIR}/libdatabase
    ${PROJECT_SOURCE_DIR}/libfileserver
    ${PROJECT_SOURCE_DIR}/libserver
    ${PROJECT_SOURCE_DIR}/libservice
)

link_directories("${PROJECT_SOURCE_DIR}/lib")

set(CMAKE_EXE_LINKER_FLAGS "-O3 -std=c++11 -std=gnu++11 -static-libgcc -static-libstdc++ -falign-functions  -falign-jumps -falign-labels  -falign-loops -finline-small-functions")

message("cmake link flags: ${CMAKE_EXE_LINKER_FLAGS}")

add_executable(archer-file ${ALL_SOURCE_FILES})

target_include_directories(archer-file PRIVATE ${CMAKE_SOURCE_DIR} )

target_link_libraries(archer-file
    archer_net-linux
    jsoncpp
    alg-linux
    lmdb
    dl
    pthread
)
