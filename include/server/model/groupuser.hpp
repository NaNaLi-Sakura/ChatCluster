#ifndef GROUPUSER_HPP
#define GROUPUSER_HPP

#include "user.hpp"

/// @brief groupuser的ORM类。为复用CUser类的信息，继承自CUser。
class CGroupUser : public CUser
{
private:
  string _grouprole; //角色
public:
  CGroupUser() = default;

  //设置grouprole
  void setRole(const string& grouprole) { _grouprole = grouprole; }

  //获取grouprole
  string getRole() const { return _grouprole; }
};

#endif //GROUPUSER_HPP