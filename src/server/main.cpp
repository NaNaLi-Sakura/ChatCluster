#include "chatserver.h"
#include <iostream>
#include <signal.h>
#include "chatservice.h"

using namespace std;

//捕获程序退出信号 服务器收到退出信号结束后，重置用户的在线状态信息
void resetHandler(int sig);

int main(int argc, char* argv[])
{
  //帮助文档
  if (argc != 3) {
    cerr << "command invalid! Please input \"ip + port\"" << endl;
    cerr << "Example: ./bin/ChatServer 127.0.0.1 6000" << endl;
    return 0;
  }

  //解析参数
  char* ip = argv[1];
  uint16_t port = port = atoi(argv[2]);

  //捕获2和15的退出信号
  signal(SIGINT, resetHandler); signal(SIGTERM, resetHandler);

  EventLoop loop;
  InetAddress addr(ip, port);
  ChatServer server(&loop, addr, "ChatServer");

  server.start();
  loop.loop();


  return 0;
}

//捕获程序退出信号 服务器收到退出信号结束后，重置用户的在线状态信息
void resetHandler(int sig)
{
  ChatService::getInstance()->reset();

  exit(0);
}
