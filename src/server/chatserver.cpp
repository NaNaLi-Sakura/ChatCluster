/*
 程序 : chatserver.cpp

 功能 : 网络模块的定义文件。
*/

#include "chatserver.h"
#include <functional>
#include "json.hpp"
#include "chatservice.h"

using namespace std;

using json = nlohmann::json;

//初始化聊天服务器对象
ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
  : _server(loop, listenAddr, nameArg), _loop(loop)
{
  //注册连接回调
  _server.setConnectionCallback(bind(&ChatServer::onConnection, this, placeholders::_1));

  //注册消息回调
  _server.setMessageCallback(bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

  //设置线程数量
  _server.setThreadNum(4);
}

//启动服务
void ChatServer::start()
{
  _server.start();
}

//上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
  //客户端断开连接
  if (!conn->connected()) {
    //用户断开连接的处理方法（1-用户状态改为offline，2-删掉用户连接map中对应信息）
    ChatService::getInstance()->clientCloseException(conn);

    conn->shutdown();
  }
}

//上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
  string strbuf = buffer->retrieveAllAsString(); //接收从客户端发送过来的数据，将其转换为字符串
  json js = json::parse(strbuf); //数据的反序列化
  
  //完全解耦网络模块的代码和业务模块的代码
  //通过js["msgid"]获取 -> 业务handler -> conn js time

  //回调消息已提前绑定好的事件处理器，来执行相应的业务处理
  // ChatService::getInstance()->getHandler(js["msgid"].get<int>())(conn, js, time);
  auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());
  msgHandler(conn, js, time);
}
