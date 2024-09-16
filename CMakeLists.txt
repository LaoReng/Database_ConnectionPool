CMAKE_MINIMUM_REQUIRED(VERSION 3.22.0)
PROJECT(ConnectionPool)

# 系统类型检测
IF(UNIX AND NOT APPLE)
    EXECUTE_PROCESS(
        COMMAND lsb_release -is
        OUTPUT_VARIABLE
        SYSTEM_TYPE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # MESSAGE(STATUS "SYSTEM_TYPE: ${SYSTEM_TYPE}")
    # MESSAGE(STATUS "OUTPUT_VARIABLE: ${OUTPUT_VARIABLE}")
    # MESSAGE(STATUS "OUTPUT_STRIP_TRAILING_WHITESPACE: ${OUTPUT_STRIP_TRAILING_WHITESPACE}")
ELSE()
    MESSAGE(STATUS "不是UNIX系统, 可能为Windows或macOS系统, 请手动安装并指定库文件及头文件所在位置")
ENDIF()

# 查找MySQL开发环境头文件所在位置
FIND_PATH(
    MYSQL_INCLUDE_DIR
    NAMES mysql.h
    PATHS /usr/include /usr/include/mysql /usr/local/include /usr/local/include/mysql /opt/local/include /opt/local/include/mysql
)
# 查找MySQL开发环境库文件所在位置
FIND_LIBRARY(
    MYSQL_LIBRARIES
    NAMES mysqlclient
    PATHS /usr/lib /usr/lib/mysql /usr/local/lib /usr/local/lib/mysql /opt/local/lib /opt/local/lib/mysql
)

# 判断MySQL开发环境是否安装
IF(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    MESSAGE(STATUS "已找到MySQL的开发环境")
    MESSAGE(STATUS "头文件所在位置: ${MYSQL_INCLUDE_DIR}")
    MESSAGE(STATUS "库文件所在位置: ${MYSQL_LIBRARIES}")
ELSE()
    # 如果没有判断它是什么系统，如果是Ubuntu系统提供命令给他安装
    MESSAGE(SEND_ERROR "未找到MySQL的开发环境")
    IF(SYSTEM_TYPE STREQUAL "Ubuntu")
        MESSAGE(STATUS "检测到您为Ubuntu发行版系统, 为您提供MySQL开发环境安装命令")
        MESSAGE(STATUS "安装命令: sudo apt-get update && sudo apt install mysql-server mysql-client libmysqlclient-dev")
    ELSE()
        MESSAGE(STATUS "非Ubuntu发行版 SYSTEM_TYPE: ${SYSTEM_TYPE}")
    ENDIF()
ENDIF()

# 指定c++版本
SET(CMAKE_CXX_STANDARD 11)
# 指定生成的可执行程序的名字
SET(EXE_NAME dbConnPool.out)

# 指定头文件位置
INCLUDE_DIRECTORIES(
    src/ConnectionPool
    src/DBDispatcher
    src/jsoncpp
    src/MySQL
)

# 递归检索指定目录下的文件，可以指定多个检索条件
FILE(GLOB_RECURSE SRC src/*.cpp src/*.c)

# 生成可执行程序
ADD_EXECUTABLE(${EXE_NAME} ${SRC})

# 指定引用的动态库文件
TARGET_LINK_LIBRARIES(
    ${EXE_NAME}
    mysqlclient
)
