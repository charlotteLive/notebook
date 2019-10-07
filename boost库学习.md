# boost库学习

## 1. 时间与日期

### 1.1timer类

timer类可以测量时间的流逝，是一个小型的计时器，提供毫秒级别的计时精度和操作函数。使用实例如下：

```C++
#include <iostream>
#include <boost/timer.hpp>

using namespace std;
using namespace boost;

int main()
{
	timer t;
	// do somrthing.
	cout << t.elapsed() << "s"<< endl;  //输出流逝的时间，单位为秒
	t.restart();  //重新开始计时
    // do somrthing.
	cout << t.elapsed() << "s" << endl;
	return 0;
}

```

timer的源码实现比较简单，是通过标准库`ctime>`里的`std::clock()`函数实现的。

```C++
class timer
{
 public:
         timer() { _start_time = std::clock(); }
  void   restart() { _start_time = std::clock(); }
  double elapsed() const
    { return  double(std::clock() - _start_time) / CLOCKS_PER_SEC; }

  double elapsed_max() const   // return estimated maximum value for elapsed()
  // Portability warning: elapsed_max() may return too high a value on systems
  // where std::clock_t overflows or resets at surprising values.
  {
    return (double((std::numeric_limits<std::clock_t>::max)())
       - double(_start_time)) / double(CLOCKS_PER_SEC); 
  }

  double elapsed_min() const  // return minimum value for elapsed()
   { return double(1)/double(CLOCKS_PER_SEC); }

 private:
  std::clock_t _start_time;
}; // timer
```





### 2. 内存池

### 2.1 pool类

pool是最简单最容易使用的内存池类，可以返回一个简单数据类型的内存指针。pool会根据需要自动自动地向系统申请或归还使用的内存，在析构时，pool将自动释放它所持有的所有内存块。

```C++
//模板类型参数UserAllocator是一个用户定义的内存分配器，它实现了特定的内存分配算法，
//通常可以直接用默认的default_user_allocator_new_delete
template <typename UserAllocator = ...>
class pool
{
public:
    //requsted_size表示每次pool分配的内存块大小，而不是内存池的大小
	explicit pool(size_type requsted_size);
	~pool();
	size_type get_requested_size() const;
    //malloc返回从内存池中分配的内存块，大小为构造函数中指定的requested_size.
    //如果内存分配失败，函数会返回0，不会抛出异常。
	void* malloc();
    //分配内存的同时合并空闲块链表，n表示可以连续分配n块的内存
	void* ordered_malloc(size_type n);
    //测试内存块是否是从这个内存池中分配出去的
	bool is_from(void* chunk) const;

    //手动释放内存，一般无需自己调用
	void free(void* chunk);
	void ordered_free(void* thunk);
	void free(void* chunk, size_type n);
	void ordered_free(void* thunk, size_type n);

    //释放所有未分配的内存，已分配的不受影响
	bool release_memory();
    //强制释放pool持有的内存，不管是否是被使用的，析构是会自动调用
	bool purge_memory();
};
```

使用实例：

```C++
#include <iostream>
#include <boost/pool/pool.hpp>
#include <assert.h>
using namespace std;
using namespace boost;

int main()
{
	pool<> pl(sizeof(int));

	int* p = (int*)pl.malloc();
	assert(pl.is_from(p));
	
	if (p != nullptr)
	{
		*p = 4;
		cout << *p << endl;
		pl.free(p);
	}
	return 0;
}
```

pool只能用于普通类型如int, double等的内存池，不能应用于复杂的类和对象，因为它只分配内存，不调用构造函数，这时候我们需要`object_pool`。

### 2.2 object_pool

`object_pool`是用于类对象的内存池，它的功能与pool类似，但会在析构时对所有已经分配的内存块调用析构函数，从而正确地释放资源。

类摘要如下：

```C++
// object_pool的模板类型参数指定了object_pool要分配的元素类型，
// 要求其析构函数不能抛出异常。
// 一旦指定了类型，object_pool实例就不能再用于分配其他类型的对象。
template <typename element_type>
class object_pool : protected pool
{
public:
	object_pool();
	~object_pool();
	element_type* malloc();
	void free(element_type* p);
	bool is_from(element_type* p)const;
	element_type* construct(...);
	void destroy(element_type* p);
};
```

`object_pool`是pool的子类，但它使用的是保护继承，因此不能使用pool的接口，但基本操作还是相似的。



## 3. 字符串于文本处理

### 3.1 `lexical_cast`

`lexical_cast`可以进行字符串、整数、浮点数之间的字面转换。使用`lexical_cast`时需要注意，要转换成数字的字符串中只能有数字和小数点，不能出现字母（表示指数的e/E除外）或其他非数字字符。

使用示例：

```c++
int x = lexical_cast<int>("1000");
long y = lexical_cast<long>("2000");
float f = lexical_cast<float>("3.14");
cout << x << y << f << endl;

cout << lexical_cast<string>(456) << endl;
cout << lexical_cast<string>(0x10) << endl;

//异常处理
try
{
    cout << lexical_cast<int>("0x100") << endl;
}
catch (bad_lexical_cast& e)
{
    //bad lexical cast: source type value could not be interpreted as target
    cout << e.what() << endl;
}
```

当`lexical_cast`无法执行转换操作时，会抛出异常`bad_lexical_cast`，它是`std::bad_cast`的派生类。为了使程序更加健壮，我们需要使用try/catch块来保护转换代码。

虽然`lexical_cast`的用法非常向转型操作符，但它仅仅是在用法上模仿了转型操作符而已，它实际上是一个模板函数。`lexical_cast`内部使用了标准库的流操作，因此对于他的转换对象有如下要求：

1. 转换起点对象是可流输出的，即定义了`operator<<`；
2. 转换终点对象是可流输入的，即定义了`operator>>`；
3. 转换终点对象必须是可缺省构造函数和可拷贝构造的。

C++中的内建类型和`std::string`都满足以上三个条件，他们也是`lexical_cast`常用的工作搭档。对于STL中的容器和其他用户自定义类型，如果没有重载流输入和流输出操作符，一般也不能使用`lexical_cast`。

### 3.2 format





### 3.3 string_algo





### 3.4 tokenizer





## 4. 文件与目录操作

