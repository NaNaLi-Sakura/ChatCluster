#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <iostream>
#include <vector>

using namespace std;

class COfflineMsgModel
{
public:
  //插入 存储用户的离线消息
  bool insert(const int userid, const string& message);

  //删除 删除用户的离线消息
  bool remove(const int userid);

  //查询 查询用户的离线消息(可能不止一条消息记录)
  vector<string> query(const int userid);
};

#endif //OFFLINEMESSAGEMODEL_H