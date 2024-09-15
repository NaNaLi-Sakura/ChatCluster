/*
 功能 : muduo网络库服务器编程练习

 作者 : 雪与冰心丶
*/

#include <iostream>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace std;
using namespace muduo;
using namespace muduo::net;

/* 基于muduo网络库开发服务器程序
 1.组合TcpServer对象
 2.创建EventLoop事件循环对象的指针
 3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
 4.在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
 5.设置合适的服务端线程数量，muduo库会自动分配I/O线程和worker线程
*/
class ChatServer
{
private:
  TcpServer _server; //组合TcpServer对象
  EventLoop* _loop; //创建EventLoop事件循环对象的指针

  /* TcpServer绑定的回调函数，当有新连接或连接中断时调用 */
  /// @brief 专门处理用户的连接创建和断开
  /// @param conn 与客户端建立的TCP连接
  void onConnection(const TcpConnectionPtr& conn);

  /* TcpServer绑定的回调函数，当有新数据时调用 */
  /// @brief 专门处理用户的读写事件
  /// @param conn 与客户端建立的TCP连接
  /// @param buffer 缓冲区
  /// @param time 接收到数据的时间信息
  void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
public:
  /// @brief 构造
  /// @param loop 事件循环
  /// @param listenAddr IP + Port
  /// @param nameArg 服务器的名字
  ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
  {
    //给服务器注册用户连接的创建和断开回调（通过绑定器设置回调函数）
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, placeholders::_1));

    //给服务器注册用户读写事件回调（通过绑定器设置回调函数）
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

    //设置服务端的线程数量 1个I/O线程，3个worker线程
    _server.setThreadNum(4);
  }

  /* 启动ChatServer服务 开启事件循环 */
  //使服务器开始接受客户端连接，并处理连接上的事件
  void start() {
    _server.start();
  }
};

//专门处理用户的连接创建和断开
inline void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
  /*------------------------------------------------------------------------
   在用户连接建立时输出连接信息，并且可以根据连接的状态进行相应的处理。
   通常，在这个函数中可以执行一些初始化操作，比如记录日志、验证用户身份等
  ------------------------------------------------------------------------*/
  if (conn->connected()) {
    //输出连接的远程地址和本地地址，并提示连接状态
    cout << "远程 : " << conn->peerAddress().toIpPort() << " -> "
      << "本地 : " << conn->localAddress().toIpPort() << " state : online" << endl;
  }
  else {
    cout << "远程 : " << conn->peerAddress().toIpPort() << " -> "
      << "本地 : " << conn->localAddress().toIpPort() << " state : offline" << endl;
    
    conn->shutdown(); //close(fd) 关闭该 TCP 连接
    // _loop->quit(); //请求事件循环退出，即停止事件处理并结束程序的执行。
  }
}

//专门处理用户的读写事件
inline void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
  string strrecvbuf = buffer->retrieveAllAsString(); //接收从客户端发送过来的数据，将其转换为字符串
  cout << "接收到了数据 recv data : " << strrecvbuf << ", time : " << time.toString() << endl;

  //将数据发给对端
  conn->send(strrecvbuf);
}

int main(int argc, char* argv[])
{
  EventLoop loop; //epoll
  InetAddress inetaddr("192.168.204.133", 6000);
  ChatServer server(&loop, inetaddr, "ChatServer");

  //使服务器开始接受客户端连接，并处理连接上的事件
  server.start(); //listenfd epoll_ctl=>epoll 创建服务，加入epoll
  loop.loop(); //epoll_wait 以阻塞方式等待新用户连接，已连接用户的读写事件等


  return 0;
}
