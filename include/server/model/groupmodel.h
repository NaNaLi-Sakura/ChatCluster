#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"

//组员角色
const string GROUP_ROLE_CREATOR = "creator";
const string GROUP_ROLE_NORMAL = "normal";

/// @brief 群组业务的操作接口类，涉及allgroup表和groupuser表
class CGroupModel
{
public:
  //创建群组(插入allgroup表)
  bool createGroup(CGroup& group);

  //加入群组(插入groupuser表)
  bool joinGroup(const int groupid, const int userid, const string& grouprole);

  //查询用户所在群组信息及组成员信息
  vector<CGroup> queryGroups(const int userid);

  //查询用户在某组中除自己外的所有组成员id
  vector<int> queryGroupUsers(const int userid, const int groupid);
};

#endif //GROUPMODEL_H