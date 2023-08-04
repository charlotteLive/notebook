## RPC：实现Python远程调用C++进程方法

RPC即remote procedure call（远程过程调用）。比如两台服务器A、B，一个应用部署在A服务器上，想要调用B服务器上应用提供的函数/方法，由于不在一个内存空间，不能直接调用，这就需要通过网络通信来表达调用的语义和传达调用的数据。

**RPC 的主要功能目标是让构建分布式计算（应用）更容易，在提供强大的远程调用能力时不损失本地调用的语义简洁性。**

### 1. 实现过程

- 首先，要解决通讯的问题，主要是通过在客户端和服务器之间建立TCP连接，远程过程调用的所有交换的数据都在这个连接里传输。连接可以是按需连接，调用结束后就断掉，也可以是长连接，多个远程过程调用共享同一个连接。
- 第二，要解决寻址的问题，也就是说，A服务器上的应用怎么告诉底层的RPC框架，如何连接到B服务器（如主机或IP地址）以及特定的端口，方法的名称名称是什么，这样才能完成调用。比如基于Web服务协议栈的RPC，就要提供一个endpoint URI，或者是从UDDI服务上查找。如果是RMI调用的话，还需要一个RMI Registry来注册服务的地址。
- 第三，当A服务器上的应用发起远程过程调用时，方法的参数需要通过底层的网络协议如TCP传递到B服务器，由于网络协议是基于二进制的，内存中的参数的值要序列化成二进制的形式，也就是序列化（Serialize）或编组（marshal），通过寻址和传输将序列化的二进制发送给B服务器。
- 第四，B服务器收到请求后，需要对参数进行反序列化（序列化的逆操作），恢复为内存中的表达方式，然后找到对应的方法（寻址的一部分）进行本地调用，然后得到返回值。
- 第五，返回值还要发送回服务器A上的应用，也要经过序列化的方式发送，服务器A接到后，再反序列化，恢复为内存中的表达方式，交给A服务器上的应用。

![RPC框架](picture\RPC框架.jpg)

这里，我们的主要目的是实现一个C++应用程序的远程调试工具。为了实现简单远程调试工具采用Python实现，通过Python脚本建立RPC客户端，远程调用C++应用构建RPC服务器的提供的方法。

### 2. C++中的RPC服务端实现

XML-RPC（RPCXML Remote Procedure Call）是通过HTTP传输XML来实现远程过程调用的RPC，因为是基于HTTP、并且使用XML文本的方式传输命令和数据，所以兼容性更好，能够跨域不同的操作系统、不同的编程语言进行远程过程调用，凡有所得，必有所失，在兼容性好的同时速度也会慢下来。

其中，XMLRPC++为XML-RPC 的 C++ 实现。它提供了简单的服务器和客户机。通过使用面向对象的技术，我们可以集成这些服务器和客户机类，并实现我们自己的 XML-RPC 服务器，以将业务功能作为服务公开。这里，我们将通过XMLRPC++来实现RPC服务端。

#### 1） XML-RPC++

XMLRPC 执行远程调用时，其输入参数与执行结果均封装在  XmlRpcValue 对象中。执行顺序如下：

>  客户端 将需执行的方法，以及方法参数（XmlRpcValue）以XML格式（通过HTTP协议）传输到服务器端，服务器解析XML，获得以XmlRpcValue封装的参数，在服务器端调用方法，获得以 XmlRpcValue封装的执行结果，将其以XML格式传输至客户端，客户端解析，获得执行结果（XmlRpcValue）。

也就是说，所有的数据都是 通过 XmlRpcValue 格式 进行交互。

首先我们来看看XmlRpc++支持的数据类型，其支持整形、布尔类型、字符串、双精度浮点数类型、时间和base64编码的二进制数据，也支持数组数据类型和结构数据类型，可阅读文件`XmlRpcValue.h`。

由于我们主要是用C++ XmlRpc做服务端，所以只需要了解以下两个类就可以了：`XmlRpcServer`，RPC服务端类；`XmlRpcServerMethod`RPC方法类。

服务器支持的调用方法必须继承自` XmlRpc::XmlRpcServerMethod`，在该类的派生类对象创建时，会自动将自己注册进服务器支持的方法中。而实际的调用方法，则是重写的方法`execute(XmlRpcValue &params, XmlRpcValue &result)`，输入参数为 params,从客户端读取而获得；执行结果 放入 result，执行完毕后返回给客户端。

以下为`XmlRpcServerMethod`RPC方法类的摘要：

```C++
//! Abstract class representing a single RPC method
class XmlRpcServerMethod 
{
public:
	//! Constructor
	XmlRpcServerMethod(std::string const& name, XmlRpcServer* server = 0)
    {
        _name = name;
        _server = server;
        if (_server) _server->addMethod(this);
    }
	//! Destructor
	virtual ~XmlRpcServerMethod();
	//! Execute the method. Subclasses must provide a definition for this method.
	virtual void execute(XmlRpcValue& params, XmlRpcValue& result) = 0;
	//! Returns a help string for the method.
	//! Subclasses should define this method if introspection is being used.
	virtual std::string help() { return std::string(); }
	//! Returns the name of the method
	std::string& name() { return _name; }

protected:
	std::string _name;
	XmlRpcServer* _server;
};
```

XMLRPC服务器示例说明：

```C++
// HelloServer.cpp : Simple XMLRPC server example. Usage: HelloServer serverPort
//
#include "XmlRpc.h"
#include <iostream>
#include <stdlib.h>

using namespace XmlRpc;

// No arguments, result is "Hello".
class Hello : public XmlRpcServerMethod
{
public:
	Hello(XmlRpcServer* s) : XmlRpcServerMethod("Hello", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		result = "Hello";
	}

	std::string help() { return std::string("Say hello"); }

};    // This constructor registers the method with the server

// One argument is passed, result is "Hello, " + arg.
class HelloName : public XmlRpcServerMethod
{
public:
	HelloName(XmlRpcServer* s) : XmlRpcServerMethod("HelloName", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		std::string resultString = "Hello, ";
		resultString += std::string(params[0]);
		result = resultString;
	}
};

// A variable number of arguments are passed, all doubles, result is their sum.
class Sum : public XmlRpcServerMethod
{
public:
	Sum(XmlRpcServer* s) : XmlRpcServerMethod("Sum", s) {}

	void execute(XmlRpcValue& params, XmlRpcValue& result)
	{
		int nArgs = params.size();
		double sum = 0.0;
		for (int i = 0; i < nArgs; ++i)
			sum += double(params[i]);
		result = sum;
	}
};


XmlRpcServer s;   // The server
Hello hello(&s);  //注入方法hello
HelloName helloName(&s);  //注入方法helloName
Sum sum(&s);  //注入方法Sum

int main()
{
	XmlRpc::setVerbosity(5);   //为每次调用记录日志
	s.bindAndListen(8080);   //绑定并侦听指定的端口
	s.enableIntrospection(true); // Enable introspection
	s.work(-1.0);  // 启动服务器，Wait for requests indefinitely

	return 0;
}
```

### 3. Python中的RPC客户端实现

类库`SimpleXMLRPCServer`一般使用在服务器端，这个模块用来构造一个最基本的XML-RPC服务器框架。

`xmlrpclib`一般使用在客户端，这个模块用来调用注册在XML-RPC服务器端的函数，`xmlrpclib`并不是一个类型安全的模块，无法抵御恶意构造的数据，这方面的一些处理工作需要交给开发者自己。

大致用法：使用`SimpleXMLRPCServer`模块运行XMLRPC服务器，在其中注册服务器提供的函数或者对象；然后在客户端内使用`xmlrpclib.ServerProxy`连接到服务器，想要调用服务器的函数，直接调用`ServerProxy`即可。

**RPC客户端示例说明：**

```python
import xmlrpclib
server = xmlrpclib.ServerProxy("http://localhost:8080")
words = server.Hello()
print "result:" + words
```

**RPC服务端示例说明：**

```python
import SimpleXMLRPCServer

class MyObject:
    def Hello(self):
        return "hello xmlprc"

obj = MyObject()
server = SimpleXMLRPCServer.SimpleXMLRPCServer(("localhost", 8080))
server.register_instance(obj)

print "Listening on port 8080"
server.serve_forever()
```

SimpleXMLRPCServer是一个单线程的服务器。这意味着，如果几个客户端同时发出多个请求，其它的请求就必须等待第一个请求完成以后才能继续。若修改服务器端如下，服务器就支持多线程并发了。

```python
from SimpleXMLRPCServer import SimpleXMLRPCServer
from SocketServer import ThreadingMixIn
class ThreadXMLRPCServer(ThreadingMixIn, SimpleXMLRPCServer):pass

class MyObject:
    def sHello(self):
        return "hello xmlprc"

obj = MyObject()
server = ThreadXMLRPCServer(("localhost", 8080), allow_none=True)
server.register_instance(obj)

print "Listening on port 8080"
server.serve_forever()
```