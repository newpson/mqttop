add_library(mt_modapi SHARED)
target_sources(mt_modapi
    PUBLIC
    mt_modapi.h
    mt_entry.h
    PRIVATE
    mt_modapi.c)
target_link_libraries(mt_modapi
    PRIVATE
    mt_tags
    mt_logf
    cjson
    PkgConfig::glib2)
target_include_directories(mt_modapi
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

add_library(mod_meminfo MODULE)
set_target_properties(mod_meminfo
    PROPERTIES PREFIX "")
target_sources(mod_meminfo
    PRIVATE
    mod_meminfo.c)
target_link_libraries(mod_meminfo
    PRIVATE
    mt_modapi)