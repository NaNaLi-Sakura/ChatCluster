/*
 程序 : chatservice.h

 功能 : 业务模块的声明文件。
*/

#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpServer.h>
#include "json.hpp"
#include <unordered_map>
#include "usermodel.h"
#include <mutex>
#include "offlinemessagemodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
#include "redis.h"

using namespace muduo;
using namespace muduo::net;
using namespace std;

using json = nlohmann::json;

//表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

/// @brief 聊天服务器业务类
class ChatService
{
private:
  //私有构造函数，禁止外部实例化
  ChatService();

  //存储消息id和其对应的业务处理方法
  unordered_map<int, MsgHandler> _msgHandlerMap;

  //数据操作类对象
  CUserModel _userModel;
  COfflineMsgModel _offlineMsgModel;
  CFriendModel _friendModel;
  CGroupModel _groupModel;

  //存储在线用户的通信连接
  unordered_map<int, TcpConnectionPtr> _userConnMap;

  //定义互斥锁，保证_userConnMap的线程安全
  mutex _connMutex;

  //redis操作对象
  Redis _redis;

public:
  //获取单例对象的接口函数
  static ChatService* getInstance();

  //获取消息对应的处理器
  MsgHandler getHandler(const int msgid) const;

  //处理登录业务
  void login(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //处理注册业务
  void regist(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //处理登出业务
  void logout(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //处理客户端异常退出
  void clientCloseException(const TcpConnectionPtr& conn);

  //业务重置方法 处理服务端异常退出
  void reset();

  //一对一聊天业务
  void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //添加好友业务
  void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //创建群组业务
  void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //加入群组业务
  void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //群组聊天业务
  void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

  //从redis消息队列中获取订阅收到的消息
  void handlerRedisSubscribeMessage(int userid, string message);
};

#endif //CHATSERVICE_H