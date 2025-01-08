add_library(log_lib STATIC
    circular_log.c
    circular_log.h
    debug_logger.c
    debug_logger.h
)

target_include_directories(log_lib PUBLIC ${CMAKE_SOURCE_DIR}/log)