#include "offlinemessagemodel.h"
#include "db.h"

//插入 存储用户的离线消息
bool COfflineMsgModel::insert(const int userid, const string& message)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "insert into offlinemessage values(%d, '%s')", userid, message.c_str());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      return true;
    }
  }

  return false;
}

//删除 删除用户的离线消息
bool COfflineMsgModel::remove(const int userid)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "delete from offlinemessage where userid = %d", userid);

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      return true;
    }
  }

  return false;
}

//查询 查询用户的离线消息(可能不止一条消息记录)
vector<string> COfflineMsgModel::query(const int userid)
{
  //拼接SQL语句，连接数据库，执行SQL语句，读取结果集，逐条记录存入vector容器，返回vector容器
  char strsql[1024]{};
  sprintf(strsql, "select message from offlinemessage where userid = %d", userid);

  vector<string> vresult;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(strsql);
    if (res != nullptr) {
      MYSQL_ROW row;
      //逐行读取结果集记录，将每行记录的第一个字段即message存入vector容器
      while ((row = mysql_fetch_row(res)) != nullptr) {
        vresult.push_back(row[0]);
      }

      mysql_free_result(res);
      return vresult;
    }
  }

  return vresult;
}
