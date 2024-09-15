#include "usermodel.h"
#include "db.h"
#include <iostream>

using namespace std;

//插入数据
bool CUserModel::insert(CUser& user)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "insert into user(name, password, state) values('%s', '%s', '%s')",
    user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
  
  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      //插入数据成功，获取用户数据生成的主键id，储存到结构中的成员中
      user.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }

  return false;
}

//根据用户id查询用户信息
CUser CUserModel::query(const int id)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "select id, name, password, state from user where id = %d", id);

  MySQL mysql;
  if (mysql.connect()) {
    //执行SQL查询语句
    MYSQL_RES* res = mysql.query(strsql);
    if (res != nullptr) {
      //查询成功 读取结果集记录（此时肯定只有一行记录，因为是按主键查的）
      MYSQL_ROW row = mysql_fetch_row(res);
      if (row != nullptr) {
        //有记录 将记录填充到表结构中，返回表
        CUser user;
        user.setId(atoi(row[0])); //记录中都是字符串char*类型，需要转换
        user.setName(row[1]);
        user.setPwd(row[2]);
        user.setState(row[3]);

        mysql_free_result(res); //释放资源，避免泄漏
        return user;
      }
    }
  }

  //失败 返回默认的表结构 匿名对象
  return CUser();
}

//更新用户的状态信息state
bool CUserModel::updateState(CUser& user)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "update user set state = '%s' where id = %d",
    user.getState().c_str(), user.getId());
  
  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      return true;
    }
  }

  return false;
}

//重置用户的状态信息 将online状态的用户信息，设置成offline
bool CUserModel::resetState()
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "update user set state = 'offline' where state = 'online'");

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      return true;
    }
  }

  return false;
}
