cmake_minimum_required(VERSION 3.22)
project(practice)

set(CMAKE_CXX_STANDARD 14)

add_executable(practice Server.cpp Common.h Client.cpp Library.h)
