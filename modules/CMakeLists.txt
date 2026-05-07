add_library(mt_modapi SHARED)
target_sources(mt_modapi
    PUBLIC
    mt_modapi.h
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

add_library(mod_cpufreq MODULE)
set_target_properties(mod_cpufreq
    PROPERTIES PREFIX "")
target_sources(mod_cpufreq
    PRIVATE
    mod_cpufreq.c)
target_link_libraries(mod_cpufreq
    PRIVATE
    mt_modapi)