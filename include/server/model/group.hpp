#ifndef GROUP_HPP
#define GROUP_HPP

#include "groupuser.hpp"
#include <vector>

/// @brief 群组表allgroup的ORM类
class CGroup
{
private:
  int _id;
  string _groupname;
  string _groupdesc;
  vector<CGroupUser> _vgroupuser; //组成员信息(某组的各个成员信息)
public:
  //构造
  CGroup(const int id = -1, const string &groupname = "", const string &groupdesc = "")
    : _id(id), _groupname(groupname), _groupdesc(groupdesc)
  {
    _vgroupuser.clear();
  }

  //设置成员的值
  void setId(const int id) { _id = id; }
  void setGroupName(const string& groupname) { _groupname = groupname; }
  void setGroupDesc(const string& groupdesc) { _groupdesc = groupdesc; }

  //获取成员的值
  int getId() const { return _id; }
  string getGroupName() const { return _groupname; }
  string getGroupDesc() const { return _groupdesc; }
  vector<CGroupUser>& getvGroupUser() { return _vgroupuser; }
};

#endif //GROUP_HPP