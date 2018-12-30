cmake_minimum_required(VERSION 3.12)
project(DexShellTools)

set(CMAKE_CXX_STANDARD 98)

INCLUDE_DIRECTORIES(/usr/local/opt/openssl/include)

link_libraries(z)
link_libraries(crypto)
add_executable(DexShellTools main.cpp )
