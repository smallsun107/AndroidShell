cmake_minimum_required(VERSION 3.12)
project(ElfEncrypt)

set(CMAKE_CXX_STANDARD 14)

add_executable(ElfEncrypt main.cpp elf.h)