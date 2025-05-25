cmake_minimum_required(VERSION 3.27)
project(untitled1 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled1 main.c
        bmp8.c
        bmp8.h
        bmp24.c
        bmp24.h
        bmp24.c)
