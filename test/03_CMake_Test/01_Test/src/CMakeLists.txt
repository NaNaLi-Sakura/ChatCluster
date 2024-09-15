# 设置需要编译的源文件列表
set(SRC_LIST ./muduo_server1.cpp)

# 将.指定路径下的所有源文件名字放入变量名SRC_LIST里面
# aux_source_directory(. SRC_LIST)

# 表示生成可执行文件muduo_server1，由SRC_LIST变量所定义的源文件编译而来
add_executable(muduo_server1 ${SRC_LIST})

# 设置可执行文件最终存储的路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# 表示muduo_server1这个目标程序，需要连接muduo_net, muduo_base, pthread这三个库文件
target_link_libraries(muduo_server1 muduo_net muduo_base pthread)