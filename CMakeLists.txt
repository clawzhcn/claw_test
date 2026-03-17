cmake_minimum_required(VERSION 3.10)
project(ClawWorkspace)

set(CMAKE_CXX_STANDARD 17)

# 自动发现目录下所有的 .cpp 文件
file(GLOB CPP_FILES "*.cpp")

foreach(CPP_FILE ${CPP_FILES})
    get_filename_component(EXE_NAME ${CPP_FILE} NAME_WE)
    add_executable(${EXE_NAME} ${CPP_FILE})
endforeach()
