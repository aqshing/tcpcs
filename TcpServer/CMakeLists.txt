## //***************** CMakeLists.txt *******************//
## //***************** written by shing *****************//
## //****************** 2020 - 06 - 27 ******************//


##cmake最低版本要求
cmake_minimum_required(VERSION 3.05)
##project name//工程名称
project(tcpserve)
##import package//导入包
find_package(Threads)
find_package(X11 REQUIRED)


##PROJECT_PATH:工程根路径
##注:${CMAKE_CURRENT_SOURCE_DIR}:获取当前CMakelists.txt文件所在目录
##注:使用${CMAKE_CURRENT_SOURCE_DIR}作用:自动获取工程路径
##但是如果自动识别出来的路径不对时,就需要手动指定目录了
##注:(使用此方法的CMakelists.txt必须放在工程根目录下面)
set(PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
##head file directory//头文件路径
set(INC_DIR "${PROJECT_PATH}/include")
##src file directory //源文件路径
set(SRC_DIR "${PROJECT_PATH}/src")
##link library directory//链接库路径
##EXECUTABLE_OUTPUT_PATH : binary file output path
##//指定二进制可执行文件的输出位置(就是把生成的目标文件放在哪)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_PATH}/bin)
##把指定目录下的所有源文件自动添加进后面的变量中
##aux_source_directory(指定目录     后面的变量)
##aux_source_directory(${SRC_DIR} SRC_FILES)
##file GLOB:把指定目录下的*.cpp *.h文件添加到变量中
file(GLOB SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h" "*${SRC_DIR}/.hpp")
##宏定义
##set(PICTURE_PATH "${PROJECT_PATH}/bin/girl.jpg")

configure_file(
	"${PROJECT_PATH}/path.h.in"
	"${INC_DIR}/path.h"
	)


##设置用debug还是release模式, debug方便调试, release跑的更快
##set(CMAKE_BUILD_TYPE Debug)
##set(CMAKE_BUILD_TYPE Release)
##set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
##set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
##set compiler flag//编译器编译选项设置
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O2")
##设置C/C++调试选项
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -O0 -D _DBG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -O0 -D _DBG")
##add head file path//添加头文件搜索路径
include_directories(${INC_DIR})
include_directories(${X11_INLUCDE_DIR})
##add lib file path//添加共享库搜索路径
link_directories(${LINK_DIR})
Message(${SRC_FILES})
add_executable(server
##src start
				${SRC_FILES}
##src end
)


##link target and lib file//把目标文件与库文件进行链接
target_link_libraries(server ${X11_LIBRARIES})
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})
