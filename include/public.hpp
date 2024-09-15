/*
 程序 : public.hpp

 功能 : server和client的公共文件
*/

#ifndef PUBLIC_HPP
#define PUBLIC_HPP

/// @brief 业务消息类型
enum EnMsgType
{
  LOGIN_MSG = 1, //登录消息
  LOGIN_MSG_ACK, //登录响应消息
  LOGOUT_MSG, //登出消息
  REGIST_MSG, //注册消息
  REGIST_MSG_ACK, //注册响应消息
  ONE_CHAT_MSG, //一对一聊天消息
  ADD_FRIEND_MSG, //添加好友消息

  CREATE_GROUP_MSG, //创建群组
  ADD_GROUP_MSG, //添加群组
  GROUP_CHAT_MSG, //群聊
};

#endif //PUBLIC_HPP