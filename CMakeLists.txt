cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(mqtt-mosquitto LANGUAGES C)

find_package(PkgConfig)
pkg_search_module(mosquitto libmosquitto REQUIRED IMPORTED_TARGET)

add_compile_options(-Wall -Wextra -Wpedantic -g -fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(client)
target_sources(client
    PRIVATE
    main.c)
target_link_libraries(client
    PRIVATE
    PkgConfig::mosquitto)
