cmake_minimum_required(VERSION 3.1.9)

project(Agos VERSION 0.1 LANGUAGES CXX)
file(GLOB_RECURSE ALTIBIT_HEADERS   ./*.cpp
                                    ./*/*.cpp
                                    ./*/*/*.cpp)

add_library(Agos STATIC ${ALTIBIT_HEADERS})