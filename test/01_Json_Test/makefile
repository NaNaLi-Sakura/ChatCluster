#-------------------------------------------------------------------
JSON_INCL = -I/home/violet/D/PApplication/ChatCluster/test

JSON_CPP = /home/violet/D/PApplication/ChatCluster/test/json.hpp

# 编译选项
CFLAGS = -g
# CFLAGS = -O2
#-------------------------------------------------------------------

all : testA1 testA2

testA1 : testA1.cpp
	g++ ${CFLAGS} -o testA1 testA1.cpp ${JSON_INCL} ${JSON_CPP}

testA2 : testA2.cpp
	g++ ${CFLAGS} -o testA2 testA2.cpp ${JSON_INCL} ${JSON_CPP}

clean :
	rm -rf testA1 testA2