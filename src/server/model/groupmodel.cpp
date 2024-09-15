#include "groupmodel.h"
#include "db.h"

//创建群组(插入allgroup表)
bool CGroupModel::createGroup(CGroup& group)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
    group.getGroupName().c_str(), group.getGroupDesc().c_str());
  
  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      //插入数据成功，获取生成的主键id，储存到结构中的成员中
      group.setId(mysql_insert_id(mysql.getConnection()));
      return true;
    }
  }

  return false;
}

//加入群组(插入groupuser表)
bool CGroupModel::joinGroup(const int groupid, const int userid, const string& grouprole)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, grouprole.c_str());

  MySQL mysql;
  if (mysql.connect()) {
    if (mysql.update(strsql)) {
      return true;
    }
  }

  return false;
}

//查询用户所在群组信息及组成员信息
vector<CGroup> CGroupModel::queryGroups(const int userid)
{
  //拼接SQL语句，连接数据库，执行SQL语句

  //1.查询用户所在群组信息
  char strsql[1024]{};
  sprintf(strsql, "select a.id, a.groupname, a.groupdesc from allgroup as a "\
    "inner join groupuser as b on a.id = b.groupid "\
    "where b.userid = %d", userid);
  
  vector<CGroup> vgroup;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(strsql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        CGroup group;
        group.setId(atoi(row[0]));
        group.setGroupName(row[1]);
        group.setGroupDesc(row[2]);
        vgroup.push_back(group);
      }

      mysql_free_result(res);
    }
  }

  //2.查询用户所在的各个群组的组成员信息，填充到vgroup容器中每个元素结构中的_vgroupuser容器中
  for (CGroup& group : vgroup) {
    sprintf(strsql, "select a.id, a.name, a.state, b.grouprole from user as a "\
      "inner join groupuser as b on a.id = b.userid "\
      "where b.groupid = %d", group.getId());
    
    MYSQL_RES* res = mysql.query(strsql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        CGroupUser groupUser;
        groupUser.setId(atoi(row[0]));
        groupUser.setName(row[1]);
        groupUser.setState(row[2]);
        groupUser.setRole(row[3]);
        group.getvGroupUser().push_back(groupUser);
      }

      mysql_free_result(res);
    }
  }

  return vgroup;
}

//查询用户在某组中除自己外的所有组成员id
vector<int> CGroupModel::queryGroupUsers(const int userid, const int groupid)
{
  //拼接SQL语句，连接数据库，执行SQL语句
  char strsql[1024]{};
  sprintf(strsql, "select userid from groupuser "\
    "where groupid = %d and userid != %d", groupid, userid);
  
  vector<int> vid;
  MySQL mysql;
  if (mysql.connect()) {
    MYSQL_RES* res = mysql.query(strsql);
    if (res != nullptr) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(res)) != nullptr) {
        vid.push_back(atoi(row[0]));
      }

      mysql_free_result(res);
    }
  }

  return vid;
}
