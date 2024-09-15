/*
 功能 : 演示json的反序列化

 作者 : 雪与冰心丶
*/

#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

//json序列化示例1
string testFunc1()
{
  json js;

  //添加key-value
  js["msg_type"] = 2;
  js["from"] = "zhang san";
  js["to"] = "li si";
  js["msg"] = "hello, what are you doing now?";
  
  //转成json字符串格式 发送给网络
  string sendbuf = js.dump();

  return sendbuf;  
}

//json序列化示例2
string testFunc2()
{
  json js;

  //添加数组
  js["id"] = {1, 2, 3, 4};
  //添加key-value
  js["name"] = "zhang san";
  
  //添加对象
  js["msg"]["zhang san"] = "hello world";
  js["msg"]["li si"] = "hello china";

  return js.dump();
}

//json序列化示例3
string testFunc3()
{
  json js;

  //直接序列化一个vector容器
  vector<int> vecA1;
  vecA1.push_back(1);
  vecA1.push_back(2);
  vecA1.push_back(5);
  js["list"] = vecA1;

  //直接序列化一个map容器
  map<int, string> mapA1;
  mapA1.insert({1, "黄山"});
  mapA1.insert({2, "华山"});
  mapA1.insert({3, "泰山"});
  js["path"] = mapA1;

  string sendBuf = js.dump(); //json数据对象 经过序列化 得到json字符串

  return sendBuf;  
}

int main(void)
{
  //从网络中接收json字符串
  // string recvbuf = testFunc1();
  // string recvbuf = testFunc2();
  string recvbuf = testFunc3();

  //数据的反序列化 json字符串->反序列化->数据对象(可以看做容器，方便访问)
  json js = json::parse(recvbuf);

  // cout << js["msg_type"] << endl;
  // cout << js << endl;

  // cout << js["id"] << endl;

  // auto arr = js["id"];
  // for (int E : arr) {
  //   cout << E << " ";
  // }
  // cout << endl;

  // json msgjs = js["msg"];
  // cout << msgjs << endl;
  // cout << msgjs["zhang san"] << endl;

  vector<int> vecA1 = js["list"].get<vector<int>>();
  for (int& E : vecA1) cout << E << " ";
  cout << endl;

  map<int, string> mapA1 = js["path"].get<map<int, string>>();
  for (auto& kv : mapA1) {
    cout << kv.first << " <=> " << kv.second << endl;
  }


  return 0;
}
