#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>
#include "user.hpp"

using namespace std;

class CFriendModel
{
public:
  //插入 添加好友关系
  bool insert(const int userid, const int friendid);

  //查询 返回用户的好友信息列表
  vector<CUser> query(const int userid);
};

#endif //FRIENDMODEL_H