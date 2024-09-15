#ifndef USER_HPP
#define USER_HPP

#include <string>

using namespace std;

/// @brief User表的ORM类 user表对象
class CUser
{
protected:
  int _id;
  string _name;
  string _password;
  string _state;
public:
  //构造
  CUser(const int id = -1, const string& name = "", const string& password = "", const string& state = "offline")
    : _id(id), _name(name), _password(password), _state(state) {}
  
  //设置成员的值
  void setId(const int id) { _id = id; }
  void setName(const string& name) { _name = name; }
  void setPwd(const string& password) { _password = password; }
  void setState(const string& state) { _state = state; }

  //获取成员的值
  int getId() const { return _id; }
  string getName() const { return _name; }
  string getPwd() const { return _password; }
  string getState() const { return _state; }
};

#endif //USER_HPP