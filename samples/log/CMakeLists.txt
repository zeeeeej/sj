# 定义一个变量来表示 log 目录，简化路径引用
set(LOG_SRC_DIR ${CMAKE_SOURCE_DIR}/log)

# 添加源文件到静态库，不要包含头文件
add_library(log_lib STATIC
    ${LOG_SRC_DIR}/circular_log.c
    ${LOG_SRC_DIR}/debug_logger.c
    ${LOG_SRC_DIR}/port/elog_file_port.c
    ${LOG_SRC_DIR}/port/elog_port.c
    ${LOG_SRC_DIR}/src/elog.c
    ${LOG_SRC_DIR}/src/elog_utils.c
    ${LOG_SRC_DIR}/src/elog_async.c
    ${LOG_SRC_DIR}/src/elog_buf.c
    ${LOG_SRC_DIR}/src/elog_file.c
    ${LOG_SRC_DIR}/log_test.c
    ${LOG_SRC_DIR}/Log_init.c
)

# 设置包含路径，仅保留必要的路径，并且合并了两个 target_include_directories 调用
target_include_directories(log_lib PUBLIC
    ${LOG_SRC_DIR}/include
    ${LOG_SRC_DIR}/port
    ${LOG_SRC_DIR}/src
)