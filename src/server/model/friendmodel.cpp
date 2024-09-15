#include "friendmodel.h"
#include "db.h"

//插入 添加好友关系
bool CFriendModel::insert(const int userid, const int friendid)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "insert into friend values(%d, %d)", userid, friendid);

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      return true;
    }
  }

  return false;
}

//查询 返回用户的好友信息列表
vector<CUser> CFriendModel::query(const int userid)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "select a.id, a.name, a.state from user as a "\
    "inner join friend as b on a.id = b.friendid "\
    "where b.userid = %d", userid);
  
  vector<CUser> vreuslt;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(strsql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        CUser user;
        user.setId(atoi(row[0]));
        user.setName(row[1]);
        user.setState(row[2]);
        vreuslt.push_back(user);
      }

      mysql_free_result(res);
      return vreuslt;
    }
  }
  
  return vreuslt;
}
