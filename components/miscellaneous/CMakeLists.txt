FILE(GLOB_RECURSE miscellaneous_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

idf_component_register(
    SRCS ${miscellaneous_sources}
    INCLUDE_DIRS "."
    REQUIRES  log xtensa
    PRIV_REQUIRES log soc
)