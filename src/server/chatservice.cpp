#include "chatservice.h"
#include "public.hpp"
#include <muduo/base/Logging.h>

//获取单例对象的接口函数
ChatService* ChatService::getInstance()
{
  static ChatService chatService;
  
  return &chatService;
}

//构造函数 注册消息以及对应的Handler回调操作(将msgid和对应的处理方法对应起来)
ChatService::ChatService()
{
  //用户基本业务管理相关事件处理注册回调
  _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  _msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  _msgHandlerMap.insert({REGIST_MSG, std::bind(&ChatService::regist, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  //群组业务管理相关事件处理注册回调
  _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, placeholders::_1, placeholders::_2, placeholders::_3)});
  _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, placeholders::_1, placeholders::_2, placeholders::_3)});

  //连接redis服务器
  if (_redis.connect()) {
    //设置上报消息的回调
    _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubscribeMessage, this, placeholders::_1, placeholders::_2));
  }
}

//获取消息对应的处理器
MsgHandler ChatService::getHandler(const int msgid) const
{
  try {
    //若map中有对应的 msgid，则返回对应的处理器
    return _msgHandlerMap.at(msgid);
  } catch (const std::out_of_range) {
    //若map中没有对应的 msgid(即msgid没有对应的事件处理回调)，则返回一个默认的处理器，空操作
    return [=](const TcpConnectionPtr&, json&, Timestamp) {
      //记录错误日志
      LOG_ERROR << "msgid : " << msgid << " can not find handler!";
    };
  }
}

//处理登录业务
void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  //调试
  // LOG_INFO << "do login service!!!";

  //解析数据，获取id和密码，根据id查询密码，与用户发来的密码对比（用户输入账号即id和密码进行登录）
  int id = js["id"].get<int>();
  string password = js["password"].get<string>();

  CUser user = _userModel.query(id);
  //判断登录情况，作出回应
  if (user.getId() == id && user.getPwd() == password) {
    //如果用户已登录，则不允许重复登录
    if (user.getState() == "online") {
      json responsejs;
      responsejs["msgid"] = LOGIN_MSG_ACK;
      responsejs["errno"] = 2; //错误类别简单区分一下
      // responsejs["errmsg"] = "该账号已登录，请重新输入新账号";
      responsejs["errmsg"] = "this account is using, please input another!";
      conn->send(responsejs.dump());
    }
    else {
      //登录成功，首先记录该用户的连接信息（同时要保证线程安全）
      {
        lock_guard<mutex> lg(_connMutex);
        _userConnMap.insert({id, conn});
      }
      //用户登录成功后，首先向redis消息队列订阅该用户频道号(userid作为频道号)
      _redis.subscribe(id);

      //登录成功 更新用户状态信息(将state的offline改为online) 作出基本回应(如果有离线消息和好友列表，一并处理)
      user.setState("online");
      _userModel.updateState(user);

      json responsejs;
      responsejs["msgid"] = LOGIN_MSG_ACK;
      responsejs["errno"] = 0;
      responsejs["id"] = user.getId(); //可以返回id，不重要，以后若不需要，可以注释掉
      responsejs["name"] = user.getName(); //同上
      //查询是否有该用户的离线消息，加入到响应消息中一并返还给改用户，并删除对应离线消息
      vector<string> vmessage = _offlineMsgModel.query(id);
      if (!vmessage.empty()) {
        responsejs["offlinemsg"] = vmessage;
        _offlineMsgModel.remove(id);
      }
      //查询该用户的好友信息列表，加入到响应消息中一并返还给用户 json标签为"friends"
      vector<CUser> vuser = _friendModel.query(id);
      if (!vuser.empty()) {
        //vector的元素类型为CUser，无法存入json，作出以下更改。
        //vector的元素类型为json字符串，每个json字符串的内容为一个好友信息，即CUser信息。
        vector<string> vjson;
        json js;
        for (CUser& user : vuser) {
          js["id"] = user.getId();
          js["name"] = user.getName();
          js["state"] = user.getState();
          vjson.push_back(js.dump());
        }
        responsejs["friends"] = vjson;
      }
      //查询用户的群组信息，返还给用户 json标签为"groups"
      vector<CGroup> vgroup = _groupModel.queryGroups(id);
      if (!vgroup.empty()) {
        //群组信息标签用"groups"，值为群组信息列表vjson，每个json为一个CGroup
        vector<string> vjson; //vjson的每个字符串成员代表一个群组信息，且使用json对象转化字符串而来
        for (CGroup& cgroup : vgroup) {
          json groupjs; //一个群组消息
          groupjs["id"] = cgroup.getId();
          groupjs["groupname"] = cgroup.getGroupName();
          groupjs["groupdesc"] = cgroup.getGroupDesc();
          // groupjs["users"] = cgroup.getvGroupUser(); //vector<CGroupUser> => vector<string>
          vector<string> vuser; //vuser的每个字符串成员代表一个成员信息，且使用json对象转化字符串而来
          for (CGroupUser& cgroupUser : cgroup.getvGroupUser()) {
            json userjs; //一个成员信息
            userjs["id"] = cgroupUser.getId();
            userjs["name"] = cgroupUser.getName();
            userjs["state"] = cgroupUser.getState();
            userjs["role"] = cgroupUser.getRole();
            vuser.push_back(userjs.dump());
          }
          groupjs["users"] = vuser;
          vjson.push_back(groupjs.dump());
        }
        responsejs["groups"] = vjson;
      }

      conn->send(responsejs.dump()); //发送响应消息
    }
  }
  else {
    //登录失败 id和密码错误 id错误-用户不存在 或者用户存在但是密码错误
    json responsejs;
    responsejs["msgid"] = LOGIN_MSG_ACK;
    responsejs["errno"] = 1;
    // responsejs["errmsg"] = "用户id或者密码错误";
    // responsejs["errmsg"] = "id or password is invalid!";
    if (user.getId() != -1) {
      //id正确，密码错误
      responsejs["errmsg"] = "password is invalid!";
    }
    else {
      //id不存在
      responsejs["errmsg"] = "id is invalid";
    }

    conn->send(responsejs.dump());
  }
}

//处理注册业务
void ChatService::regist(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  //调试
  // LOG_INFO << "do regist service!!!";

  //获取用户名和密码，插入用户信息表，返还用户ID（用户输入用户名和密码进行注册）

  //解析数据成员，填充CUser结构，传入Model的insert方法
  string name = js["name"].get<string>();
  string password = js["password"].get<string>();
  CUser user;
  user.setName(name); user.setPwd(password);
  if (_userModel.insert(user)) {
    //注册成功 作出基本的回应 并返还生成的id
    json responsejs;
    responsejs["msgid"] = REGIST_MSG_ACK;
    responsejs["errno"] = 0;
    responsejs["id"] = user.getId();
    conn->send(responsejs.dump());
  }
  else {
    //注册失败 作出基本的回应
    json responsejs;
    responsejs["msgid"] = REGIST_MSG_ACK;
    responsejs["errno"] = 1;
    conn->send(responsejs.dump());
  }
}

//处理登出业务
void ChatService::logout(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  int userid = js["id"].get<int>();

  //1.清除用户连接map中对应信息（考虑线程安全）
  {
    lock_guard<mutex> lg(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end()) {
      _userConnMap.erase(it);
    }
  }

  //2.用户下线后，取消redis消息队列的订阅频道
  _redis.unsubscribe(userid);

  //3.将用户信息表中对应状态改为offline
  CUser user(userid, "", "", "offline");
  _userModel.updateState(user);
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
  CUser user;
  //1.清除用户连接map中对应信息（考虑线程安全）
  {
    lock_guard<mutex> lg(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
      if (it->second == conn) {
        user.setId(it->first);
        _userConnMap.erase(it);
        break;
      }
    }
  }

  //2.用户下线后，取消redis消息队列的订阅频道
  _redis.unsubscribe(user.getId());

  //3.将用户信息表中对应状态改为offline 若没找到连接，user的id是-1，就没必要连接数据库了
  if (user.getId() != -1) {
    user.setState("offline");
    _userModel.updateState(user);
  }
}

//业务重置方法 处理服务端异常退出
void ChatService::reset()
{
  //将online状态的用户信息，设置成offline
  _userModel.resetState();
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  //解析对端id，查询对端连接，若在线则直接转发，若不在线则暂存离线表

  int toid = js["toid"].get<int>();
  {
    lock_guard<mutex> lg(_connMutex);
    auto it = _userConnMap.find(toid);
    if (it != _userConnMap.end()) {
      //若对方在线，则服务端直接将消息转发给对方
      it->second->send(js.dump());      
      return;
    }
  }
  //若map中无对方的连接，可能对方在别的服务器，先查询数据表看对方是否在线
  CUser user = _userModel.query(toid);
  if (user.getState() == "online") {
    //若对方在线，则对方一定在别的服务器上登录了，这时将消息发布到redis的消息队列的对方频道号
    _redis.publish(toid, js.dump());
    return;
  }

  //若对方不在线，将离线消息储存在消息表中
  _offlineMsgModel.insert(toid, js.dump());
}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  int userid = js["id"].get<int>();
  int friendid = js["friendid"].get<int>();

  //存储好友信息
  _friendModel.insert(userid, friendid);
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  int userid = js["id"].get<int>();
  string groupname = js["groupname"].get<string>();
  string groupdesc = js["groupdesc"].get<string>();

  //创建新的群组
  CGroup group(-1, groupname, groupdesc);
  if (_groupModel.createGroup(group)) {
    //存储群组创建人的信息
    _groupModel.joinGroup(group.getId(), userid, GROUP_ROLE_CREATOR);
  }
}

//加入群组业务
void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();
  _groupModel.joinGroup(groupid, userid, GROUP_ROLE_NORMAL);
}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
  //解析userid和groupid，查询同组的userid，再查询各user的conn，发送消息或存离线表
  int userid = js["id"].get<int>();
  int groupid = js["groupid"].get<int>();
  vector<int> vuserid = _groupModel.queryGroupUsers(userid, groupid);

  //对每个组员转发消息或存储离线消息
  //涉及到操作用户连接的map，需要保证线程安全
  lock_guard<mutex> lg(_connMutex);
  for (int userid : vuserid) {
    //若在线，则直接转发
    auto iter = _userConnMap.find(userid);
    if (iter != _userConnMap.end()) {
      iter->second->send(js.dump());
      continue;
    }
    //若map中无对方的连接，可能对方在别的服务器，先查询数据表看对方是否在线
    CUser user = _userModel.query(userid);
    if (user.getState() == "online") {
      //若对方在线，则对方一定在别的服务器上登录了，这时将消息发布到redis的消息队列的对方频道号
      _redis.publish(userid, js.dump());
    }
    else {
      //若对方不在线，将离线消息储存在消息表中
      _offlineMsgModel.insert(userid, js.dump());
    }
  }
}

//从redis消息队列中获取订阅收到的消息
void ChatService::handlerRedisSubscribeMessage(int userid, string message)
{
  //查询map中用户的连接(保证线程安全)，若在线，则直接发送消息
  lock_guard<mutex> lg(_connMutex);
  auto it = _userConnMap.find(userid);
  if (it != _userConnMap.end()) {
    it->second->send(message);
    return;
  }
  //若用户下线，则存储离线消息
  _offlineMsgModel.insert(userid, message);
}