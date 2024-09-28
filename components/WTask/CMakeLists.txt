
if(CONFIG_WORKQUEUE_SUPPORT)
idf_component_register(
    SRCS Task.cpp NTask.cpp RTask.cpp WorkQueue.cpp
    INCLUDE_DIRS "."
    REQUIRES freertos esp_ringbuf
    PRIV_REQUIRES log
)
elseif(CONFIG_RTASK_SUPPORT)
idf_component_register(
    SRCS Task.cpp NTask.cpp RTask.cpp
    INCLUDE_DIRS "."
    REQUIRES freertos esp_ringbuf
    PRIV_REQUIRES log
)
elseif(CONFIG_NTASK_SUPPORT)
idf_component_register(
    SRCS Task.cpp NTask.cpp
    INCLUDE_DIRS "."
    REQUIRES freertos esp_ringbuf
    PRIV_REQUIRES log
)
else()
idf_component_register(
    SRCS Task.cpp
    INCLUDE_DIRS "."
    REQUIRES freertos esp_ringbuf
    PRIV_REQUIRES log
)
endif()

set_target_properties(${COMPONENT_LIB} PROPERTIES COMPILE_FLAGS "-save-temps")