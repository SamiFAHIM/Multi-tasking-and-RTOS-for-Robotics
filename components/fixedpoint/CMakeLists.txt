FILE(GLOB_RECURSE fixedpoint_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)
idf_component_register(
    SRCS ${fixedpoint_sources}
    INCLUDE_DIRS "."
    REQUIRES miscellaneous
)