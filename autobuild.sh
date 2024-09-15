#===================================================================
# File Name: autobuild.sh
# Author: 雪与冰心丶
# main: 2414811214@qq.com
# Created Time: 2024年 09月 15日 星期日 15:58:31 CST
#===================================================================
#!/bin/bash

set -x

rm -rf ./build/
cmake -B ./build
cmake --build ./build
