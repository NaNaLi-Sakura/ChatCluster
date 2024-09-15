#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>

using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <semaphore.h>
#include <atomic>

#include "public.hpp"
#include "user.hpp"
#include "group.hpp"

CUser currUser; //记录当前系统登录的用户信息
vector<CUser> vcurrUserFriendList; //记录当前登录用户的好友列表信息
vector<CGroup> vcurrUserGroupList; //记录当前登录用户的群组列表信息

//向目标ip和端口发起socket连接。
int conntodst(const char* ip, const int port);

//login业务 (用户按照提示输入账号即id和密码进行登录)
bool loginService(const int clientsock);

//register业务 (用户输入用户名和密码进行注册，服务端返还生成的id)
void registService(const int clientsock);

//quit业务 关闭sock，退出程序
void quitService(int& clientsock);

//显示当前登录成功用户的基本信息
void showCurrUserData();

//接收子线程，接收服务端的响应数据
void readTaskHandler(const int clientsock);

//主聊天页面程序(登录成功之后)
void mainMenu(const int clientsock);

//获取系统的时间(聊天信息需要添加时间信息)
string getCurrTime();

//---------------------------------------------------------------------
//控制主菜单页面程序
bool isMainMenuRunning = false;

//系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
  {"help", "显示所有支持的命令列表，格式\"help\""},
  {"chat", "一对一聊天，格式\"chat:friendid:message\""},
  {"addfriend", "添加好友，格式\"addfriend:friendid\""},
  {"creategroup", "创建群组，格式\"creategroup:groupname:groupdesc\""},
  {"addgroup", "加入群组，格式\"addgroup:groupid\""},
  {"groupchat", "群聊，格式\"groupchat:groupid:message\""},
  {"logout", "注销，格式\"logout\""}
};

//帮助文档，显示命令列表
void help(const int clientsock = 0, const string& strdata = "");
//一对一聊天
void chat(const int clientsock, const string& strdata);
//添加好友
void addfriend(const int clientsock, const string& strdata);
//创建群组
void creategroup(const int clientsock, const string& strdata);
//加入群组
void addgroup(const int clientsock, const string& strdata);
//群聊
void groupchat(const int clientsock, const string& strdata);
//注销
void logout(const int clientsock, const string& strdata);

//命令对应处理方法列表
unordered_map<string, function<void(const int, const string&)>> commandHandlerMap = {
  {"help", help},
  {"chat", chat},
  {"addfriend", addfriend},
  {"creategroup", creategroup},
  {"addgroup", addgroup},
  {"groupchat", groupchat},
  {"logout", logout}
};
//---------------------------------------------------------------------

//聊天客户端程序实现，main线程用作发送线程(向服务端发送信息)，子线程用作接收线程(接收服务端的回应)。
int main(int argc, char* argv[])
{
  //帮助文档。
  if (argc != 3) {
    printf("Using: ./ChatClient ip port\n");
    printf("Example: /home/violet/D/PApplication/ChatCluster/bin/ChatClient 127.0.0.1 6000\n");
    printf("Example: ./bin/ChatClient 127.0.0.1 8000\n\n");
    return -1;
  }

  //创建客户端socket，向目标地址和端口发起连接。
  int clientsock = conntodst(argv[1], atoi(argv[2]));
  if (clientsock == -1) {
    cerr << "conntodst(" << argv[1] << ":" << argv[2] << ") func error" << endl;
    exit(-1);
  }

  //main线程用于接收用户输入，并发送消息给服务端。
  while (true) {
    //显示首页面菜单 登录、注册、退出
    cout << "==================================" << endl;
    cout << "1. login" << endl;
    cout << "2. register" << endl;
    cout << "3. quit" << endl;
    cout << "==================================" << endl;
    cout << "please input your choice:";
    int choice = 0;
    cin >> choice; cin.get(); //读掉缓冲区残留的回车

    //根据用户不同的选择，作出不同的业务处理。
    switch (choice) {
    case 1: {
      //login业务 (用户按照提示输入账号即id和密码进行登录)
      if (loginService(clientsock) == false) break;

      //登录成功，创建子线程负责随时读取消息(随时接收服务端传来的数据)，该线程只启动一次
      static int readthreadnumber = 0;
      if (readthreadnumber == 0) {
        std::thread readTask(readTaskHandler, clientsock);
        readTask.detach(); //线程分离
        readthreadnumber++; //自加之后，不再为0，即不会再次创建此线程
      }

      //登录成功，进入聊天主菜单页面
      isMainMenuRunning = true;
      mainMenu(clientsock);

      break;
    }
    case 2: {
      //regist业务 (用户输入用户名和密码进行注册，服务端返还生成的id)
      registService(clientsock);
      break;
    }
    case 3: {
      //quit业务 关闭sock，退出程序
      quitService(clientsock);
      break;
    }
    default: {
      //报错
      cerr << "invalid input!" << endl;
      break;
    }
    }
  }

  return 0;
}

//向目标ip和端口发起socket连接。
int conntodst(const char* ip, const int port)
{
  //创建，发起连接请求
  int clientsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (clientsock == -1) {
    perror("socket func error"); return -1;
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  struct hostent* h = gethostbyname(ip);
  if (h == NULL) { close(clientsock); return -1; }
  memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
  if (connect(clientsock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
    perror("connect func error"); return -1;
  }

  return clientsock;
}

//login业务 (用户按照提示输入账号即id和密码进行登录)
bool loginService(const int clientsock)
{
  //提示用户输入账号id和密码
  int userid = 0;
  char password[50]{};
  cout << "userid:"; cin >> userid; cin.get();
  cout << "password:"; cin.getline(password, 50); //getline支持空格

  //将账号id和密码存储为json对象，序列化为字符串，作为登录业务消息发送给服务器
  json js;
  js["msgid"] = LOGIN_MSG; //登录业务
  js["id"] = userid; //账号id
  js["password"] = password; //密码
  string strrequest = js.dump(); //将json对象序列化为字符串，作为待发送的请求数据

  //将登录业务消息发送给服务端
  int retlen = send(clientsock, strrequest.data(), strrequest.size(), 0);
  if (retlen == -1) {
    cerr << "send login msg(" << strrequest << ") error." << endl;
    return false;
  }
  else if (retlen == 0) {
    cerr << "clientsock is disconnected." << endl;
    return false;
  }
  //发送成功，接收服务端传来的响应
  char recvbuffer[1024]{};
  retlen = recv(clientsock, recvbuffer, sizeof(recvbuffer), 0);
  if (retlen == -1) {
    cerr << "recv login response error." << endl;
    return false;
  }
  else if (retlen == 0) {
    cerr << "clientsock is disconnected." << endl;
    return false;
  }
  //将响应数据反序列化为json对象，解析响应数据
  json responsejs = json::parse(recvbuffer);
  if (responsejs["errno"].get<int>() != 0) {
    //登录失败
    cerr << responsejs["errmsg"].get<string>() << endl;
    return false;
  }
  //登录成功，记录存储一些信息，以后若需要可以直接使用

  //记录当前用户的信息id和name
  currUser.setId(responsejs["id"].get<int>());
  currUser.setName(responsejs["name"].get<string>());

  //记录当前用户的好友列表信息(在"friends"存在的前提下)
  if (responsejs.contains("friends")) {
    //初始化好友列表
    vcurrUserFriendList.clear();

    //响应消息中大标签为friends，值为一个vector，vector的元素类型为json字符串，一个json字符串存储一个用户表结构信息
    vector<string> vjson = responsejs["friends"];
    //将每个json字符串反序列化为json对象，再解析到单个好友信息对象CUser中，最后添加到好友信息列表
    for (string& strjs : vjson) {
      json userjs = json::parse(strjs);
      CUser user;
      user.setId(userjs["id"].get<int>());
      user.setName(userjs["name"].get<string>());
      user.setState(userjs["state"].get<string>());
      vcurrUserFriendList.push_back(user);
    }
  }
  
  //记录当前用户的群组列表信息(在"groups"存在的前提下)
  if (responsejs.contains("groups")) {
    //初始化清空群组列表
    vcurrUserGroupList.clear();

    //响应消息中大标签为"groups"，值为vector，vector的元素类型为json字符串，一个json字符串存储一个群组信息
    vector<string> vjson = responsejs["groups"];
    //将每个json字符串反序列化为json对象，再解析到单个群组信息对象CGroup中，最后添加到群组信息列表
    for (string& strjs : vjson) {
      json groupjs = json::parse(strjs);
      CGroup group;
      group.setId(groupjs["id"].get<int>());
      group.setGroupName(groupjs["groupname"].get<string>());
      group.setGroupDesc(groupjs["groupdesc"].get<string>());
      //组成员信息是vector，因为一个组有多个成员。
      //大标签为"users"，值为vector，vector的元素类型为json字符串，一个json字符串存储一个成员信息
      vector<string> vjson = groupjs["users"];
      //将每个json字符串反序列化为json对象，再解析成CGroupUser对象，最后添加到CGroup对象的_vgroupuser容器中
      for (string& strjs : vjson) {
        json grpuserjs = json::parse(strjs);
        CGroupUser groupUser;
        groupUser.setId(grpuserjs["id"].get<int>());
        groupUser.setName(grpuserjs["name"].get<string>());
        groupUser.setState(grpuserjs["state"].get<string>());
        groupUser.setRole(grpuserjs["role"].get<string>());
        group.getvGroupUser().push_back(groupUser);
      }
      vcurrUserGroupList.push_back(group);
    }
  }

  //显示当前登录成功的用户的基本信息
  showCurrUserData();

  //显示当前用户的离线消息，不管是群聊还是私聊(在"offlinemsg"存在的前提下)
  if (responsejs.contains("offlinemsg")) {
    //离线消息大标签是"offlinemsg"，值是vector，vector的元素类型为json字符串，一个json字符串存储一条离线信息
    vector<string> vjson = responsejs["offlinemsg"];
    //将每个json字符串反序列化为json对象，再解析成具体的消息显示出来
    for (string& strjs : vjson) {
      json msgjs = json::parse(strjs);
      int msgtype = msgjs["msgid"].get<int>();
      //如果是私聊
      if (msgtype == ONE_CHAT_MSG) {
        //{"groupid":1,"id":21,"msg":"hello","msgid":10,"name":"gao yang","time":"2020-02-22 00:43:59"}
        //time + id + name + said(msg)
        cout << msgjs["time"].get<string>() << " [" << msgjs["id"] << "]" << msgjs["name"].get<string>()
          << " said: " << msgjs["msg"] << endl;
      }
      //否则如果是群聊
      else if (msgtype == GROUP_CHAT_MSG) {
        //groupid + time + id + name + said(msg)
        cout << "群消息[" << msgjs["groupid"].get<int>() << "]:" << msgjs["time"].get<string>()
          << " [" << msgjs["id"].get<int>() << "]" << msgjs["name"].get<string>()
          << " said: " << msgjs["msg"].get<string>() << endl;
      }
    }
  }

  return true;
}

//register业务 (用户输入用户名和密码进行注册，服务端返还生成的id)
void registService(const int clientsock)
{
  //提示用户输入用户名和密码
  char name[50]{};
  char password[50]{};
  cout << "username:"; cin.getline(name, 50);
  cout << "password:"; cin.getline(password, 50);

  //将账号id和密码存储为json对象，序列化为字符串，作为注册业务消息发送给服务器
  json js;
  js["msgid"] = REGIST_MSG; //注册业务
  js["name"] = name; //用户名
  js["password"] = password; //密码
  string strrequest = js.dump(); //将json对象序列化为字符串，作为待发送的请求数据

  //将注册业务消息发送给服务端
  int retlen = send(clientsock, strrequest.data(), strrequest.size(), 0);
  if (retlen == -1) {
    cerr << "send register msg(" << strrequest << ") error." << endl;
    return;
  }
  else if (retlen == 0) {
    cerr << "clientsock is disconnected." << endl;
    return;
  }

  //发送成功，接收服务端传来的响应
  char recvbuffer[1024]{};
  retlen = recv(clientsock, recvbuffer, sizeof(recvbuffer), 0);
  if (retlen == -1) {
    cerr << "recv register response error." << endl;
    return;
  }
  else if (retlen == 0) {
    cerr << "clientsock is disconnected." << endl;
    return;
  }

  //将响应数据反序列化为json对象，解析响应，显示对应信息
  json responsejs = json::parse(recvbuffer);
  if (responsejs["errno"].get<int>() != 0) {
    //注册失败
    cerr << name << " is already exist, register error!" << endl;
    return;
  }
  //注册成功，显示生成的id
  cout << name << " register success, userid is " << responsejs["id"]
    << ", don't forget it!" << endl;
}

//quit业务 关闭sock，退出程序
void quitService(int& clientsock)
{
  close(clientsock); clientsock = -1;
  exit(0);
}

//显示当前登录成功用户的基本信息
void showCurrUserData()
{
  //显示用户基本信息
  cout << "===========================<< login user >>===========================" << endl;
  cout << "current login user => id: " << currUser.getId() << " name: " << currUser.getName() << endl;
  //显示当前用户好友列表信息
  cout << "---------------------------<< friend list >>---------------------------" << endl;
  if (!vcurrUserFriendList.empty()) {
    for (CUser& user : vcurrUserFriendList) {
      cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
    }
  }
  //显示群组列表信息
  cout << "---------------------------<< group list >>---------------------------" << endl;
  if (!vcurrUserGroupList.empty()) {
    for (CGroup& group : vcurrUserGroupList) {
      cout << "[" << group.getId() << "] : " << group.getGroupName() << "(" << group.getGroupDesc() << ")" << endl;
      for (CGroupUser& groupuser : group.getvGroupUser()) {
        cout << groupuser.getId() << " " << groupuser.getName() << " "
          << groupuser.getState() << " " << groupuser.getRole() << endl;
      }
    }
  }
  cout << "=========================================================================" << endl;
}

//获取系统的时间(聊天信息需要添加时间信息)
string getCurrTime()
{
  time_t nowtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  struct tm* ptm = localtime(&nowtime);
  char currTime[60]{};
  sprintf(currTime, "%d-%02d-%02d %02d:%02d:%02d",
    ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
    ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  
  return string(currTime);
}

//接收子线程，接收服务端的响应数据
void readTaskHandler(const int clientsock)
{
  //循环接收服务器响应数据，并解析显示
  while (true) {
    char buffer[1024]{};
    if (recv(clientsock, buffer, sizeof(buffer), 0) <= 0) {
      close(clientsock); exit(0);
    }

    //将接收到的json字符串反序列化为json对象，再根据业务类型，解析成具体的消息显示出来
    json js = json::parse(buffer);
    int msgtype = js["msgid"].get<int>();

    //一对一聊天业务
    if (msgtype == ONE_CHAT_MSG) {
      //time + id + name + said(msg)
      cout << js["time"].get<string>() << " [" << js["id"].get<int>() << "]"
        << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
      continue;
    }
    //群组聊天业务
    if (msgtype == GROUP_CHAT_MSG) {
      //groupid + time + id + name + said(msg)
      cout << "群消息[" << js["groupid"].get<int>() << "]:" << js["time"].get<string>()
        << " [" << js["id"].get<int>() << "]" << js["name"].get<string>()
        << " said: " << js["msg"].get<string>() << endl;
      continue;
    }
  }
}

//主聊天页面程序(登录成功之后)
void mainMenu(const int clientsock)
{
  //帮助文档，显示命令列表
  help();

  //循环接收用户输入，根据命令调用对应的业务方法(发送请求给服务端)
  char buffer[1024]{};
  while (isMainMenuRunning) {
    //读取用户的输入，使用string类型变量存放
    cin.getline(buffer, 1024);
    string commandbuf(buffer);

    //截取命令(第一个":"前面的内容即命令，若无":"，则直接为命令。后续会有验证，保证命令位于命令列表中)
    string command;
    int index = commandbuf.find(":");
    if (index == std::string::npos) {
      command = commandbuf;
    }
    else {
      command = commandbuf.substr(0, index);
    }

    //判断命令是否位于命令处理方法列表中
    auto it = commandHandlerMap.find(command);
    if (it == commandHandlerMap.end()) {
      cerr << "invalid input command!" << endl;
      continue;
    }

    //根据命令回调相应的业务处理，即调用相应的命令处理方法。mainMenu对修改封闭，添加新功能不需要修改该函数。
    //strdata，传入的命令参数是后面的有效数据。
    it->second(clientsock, commandbuf.substr(index + 1, commandbuf.length() - index - 1));
  }
}

//---------------------------------------------------------------------
//帮助文档，显示命令列表
void help(const int clientsock, const string& strdata)
{
  cout << "show command list >>>" << endl;
  for (auto& kv : commandMap) {
    cout << kv.first << " : " << kv.second << endl;
  }
  cout << endl;
}
//一对一聊天
void chat(const int clientsock, const string& strdata)
{
  //strdata => friendid:message

  //解析数据，序列化为json字符串，发送对应请求给服务端
  int index = strdata.find(":");
  if (index == std::string::npos) {
    cerr << "chat command invalid!" << endl;
    return;
  }
  int friendid = atoi(strdata.substr(0, index).c_str());
  string message = strdata.substr(index + 1);

  json js;
  js["msgid"] = ONE_CHAT_MSG;
  js["id"] = currUser.getId();
  js["name"] = currUser.getName();
  js["toid"] = friendid;
  js["msg"] = message;
  js["time"] = getCurrTime();
  string sendbuffer = js.dump();

  if (send(clientsock, sendbuffer.data(), sendbuffer.size(), 0) == -1) {
    cerr << "send chat msg error : " << sendbuffer << endl;
  }
}
//添加好友
void addfriend(const int clientsock, const string& strdata)
{
  //strdata => friendid

  //解析数据，序列化为json字符串，发送对应请求给服务端
  json js;
  js["msgid"] = ADD_FRIEND_MSG;
  js["id"] = currUser.getId();
  js["friendid"] = atoi(strdata.c_str());
  string sendbuffer = js.dump();

  if (send(clientsock, sendbuffer.data(), sendbuffer.size(), 0) == -1) {
    cerr << "send addfriend msg error : " << sendbuffer << endl;
  }
}
//创建群组
void creategroup(const int clientsock, const string& strdata)
{
  //strdata => groupname:groupdesc

  //解析数据，序列化为json字符串，发送对应请求给服务端
  int index = strdata.find(":");
  if (index == std::string::npos) {
    cerr << "creategroup command invalid!" << endl;
    return;
  }
  string groupname = strdata.substr(0, index);
  string groupdesc = strdata.substr(index + 1);

  json js;
  js["msgid"] = CREATE_GROUP_MSG;
  js["id"] = currUser.getId();
  js["groupname"] = groupname;
  js["groupdesc"] = groupdesc;
  string sendbuffer = js.dump();

  if (send(clientsock, sendbuffer.data(), sendbuffer.size(), 0) <= 0) {
    cerr << "send creatgroup msg error : " << sendbuffer << endl;
  }
}
//加入群组
void addgroup(const int clientsock, const string& strdata)
{
  //strdata => groupid

  //解析数据，序列化为json字符串，发送对应请求给服务端
  json js;
  js["msgid"] = ADD_GROUP_MSG;
  js["id"] = currUser.getId();
  js["groupid"] = atoi(strdata.c_str());
  string sendbuffer = js.dump();
  
  if (send(clientsock, sendbuffer.data(), sendbuffer.size(), 0) <= 0) {
    cerr << "send addgroup msg error : " << sendbuffer << endl;
  }
}
//群聊
void groupchat(const int clientsock, const string& strdata)
{
  //strdata => groupid:message

  //解析数据，序列化为json字符串，发送对应请求给服务端
  int index = strdata.find(":");
  if (index == std::string::npos) {
    cerr << "groupchat command invalid!" << endl;
  }
  int groupid = atoi(strdata.substr(0, index).c_str());
  string message = strdata.substr(index + 1);

  json js;
  js["msgid"] = GROUP_CHAT_MSG;
  js["id"] = currUser.getId();
  js["name"] = currUser.getName();
  js["groupid"] = groupid;
  js["msg"] = message;
  js["time"] = getCurrTime();
  string sendbuffer = js.dump();
  if (send(clientsock, sendbuffer.data(), sendbuffer.size(), 0) <= 0) {
    cerr << "send groupchat msg error : " << sendbuffer << endl;
  }
}
//注销
void logout(const int clientsock, const string& strdata)
{
  json js;
  js["msgid"] = LOGOUT_MSG;
  js["id"] = currUser.getId();
  string sendbuffer = js.dump();
  if (send(clientsock, sendbuffer.data(), sendbuffer.size(), 0) <= 0) {
    cerr << "send logout msg error : " << sendbuffer << endl;
    return;
  }
  //登出成功，将退出主菜单，重新返回登录注册页面
  isMainMenuRunning = false;
}
//---------------------------------------------------------------------
