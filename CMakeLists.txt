cmake_minimum_required(VERSION 3.22)
project(Paraller_Computing C)

set(CMAKE_C_STANDARD 99)

add_executable(Paraller_Computing
        H4/nBody.c)

add_compile_options(-O3 -Wall -lm -lpthread)
