/*
 功能 : 演示json的序列化

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
void testFunc1()
{
  json js;

  //添加key-value
  js["msg_type"] = 2;
  js["from"] = "zhang san";
  js["to"] = "li si";
  js["msg"] = "hello, what are you doing now ?";

  cout << js << endl; //已序列化为json字符串。

  //若要通过网络传输，可转成string字符串。
  string sendbuf = js.dump();
  cout << sendbuf << endl;
}

//json序列化示例2
void testFunc2()
{
  json js;

  //添加数组
  js["id"] = {1, 2, 3, 4};
  //添加key-value
  js["name"] = "zhang san";

  js["msg"]["zhang san"] = "hello world";
  js["msg"]["li si"] = "hello china";
  js["msg2"] = {{"zhang san2", "hello world2"}, {"li si2", "hello china2"}};
  //上面的"msg"和"msg2"都是一个意思，最终的形式是下面的样子，只是写成代码时，不能安装后面的"msg3"的写法，必须是上面的两种写法。
  // "msg2":{"li si2":"hello china2","zhang san2":"hello world2"}
  // js["msg3"] = {"zhang san3":"hello world3", "li si3":" hello china3"};

  cout << js << endl;
}

//json序列化示例3
void testFunc3()
{
  json js;

  //直接序列化一个vector容器
  vector<int> vecA1;
  vecA1.push_back(1);
  vecA1.push_back(2);
  vecA1.push_back(3);
  js["list"] = vecA1;

  //直接序列化一个map容器
  map<int, string> mapA1;
  mapA1.insert({1, "黄山"});
  mapA1.insert({2, "华山"});
  mapA1.insert({3, "泰山"});
  js["path"] = mapA1;

  cout << js << endl;

  string sendbuf = js.dump(); //json数据对象 经过序列化 得到json字符串
  cout << sendbuf << endl;
}

int main(void)
{
  // testFunc1();
  // testFunc2();
  testFunc3();


  return 0;
}