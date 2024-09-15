#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

/// @brief 用户信息表user的数据操作接口类
class CUserModel
{
public:
  //插入数据
  bool insert(CUser& user);

  //根据用户id查询用户信息
  CUser query(const int id);

  //更新用户的状态信息state
  bool updateState(CUser& user);

  //重置用户的状态信息 将online状态的用户信息，设置成offline
  bool resetState();
};

#endif //USERMODEL_H