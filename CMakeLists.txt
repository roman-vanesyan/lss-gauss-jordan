cmake_minimum_required(VERSION 3.13)
project(lss C)

set(CMAKE_C_STANDARD 99)

add_executable(lss lss.h lss.c main.c)
