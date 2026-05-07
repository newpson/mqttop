cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(mqttop LANGUAGES C)

find_package(cJSON REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(mosquitto libmosquitto REQUIRED IMPORTED_TARGET)
pkg_search_module(glib2 glib-2.0 REQUIRED IMPORTED_TARGET)

# add_compile_options(-O0 -Wall -Wextra -Wpedantic -g -fsanitize=address)
# add_link_options(-O0 -fsanitize=address)

add_library(mt_logf INTERFACE)
target_sources(mt_logf
    PUBLIC
    mt_logf.h)

add_library(mt_tags SHARED)
target_sources(mt_tags
    PUBLIC
    mt_tags.h
    mt_equation.h
    mt_entrylist.h
    PRIVATE
    mt_tags.c
    mt_equation.c
    mt_entrylist.c)
target_link_libraries(mt_tags
    PRIVATE
    mt_logf
    PkgConfig::glib2)
target_include_directories(mt_tags
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

add_library(mt_client SHARED)
target_sources(mt_client
    PUBLIC
    mt_client.h
    mt_module.h
    PRIVATE
    mt_client.c
    mt_module.c)
target_link_libraries(mt_client
    PRIVATE
    mt_logf
    mt_tags
    mt_modapi
    cjson
    mosquitto
    PkgConfig::glib2)
target_include_directories(mt_client
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

add_executable(mqttop)
target_sources(mqttop
    PRIVATE
    main.c)
target_link_libraries(mqttop
    PRIVATE
    mt_logf
    mt_client
    PkgConfig::glib2
    cjson)

add_subdirectory(modules)
