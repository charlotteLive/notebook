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

format库实现了类似于printf的格式化对象，可以把参数格式化到一个字符串，而且是安全类型安全的。

```C++
#include <boost/format.hpp>
using namespace boost;
using namespace std;
int main()
{
    cout << format("%s:%d+%d=%d") % "sum" % 2 % 3 % 5 << endl;
    cout << format("(%1% + %2%) * %2% = %3%") % 2 % 3 % ((2 + 3) * 3) << endl;
}
```





### 3.3 string_algo

`string_algo`库是一个非常全面的字符串算法库，提供了大量的字符串操作函数，如大小写无关比较、修剪、特定模式的字串查找等，可以在不使用正则表达式的情况了处理大多数字符串相关问题。

`string_algo`被设计用于处理字符串，然后它的处理对象不一定是`string`或`basic_string`，可以是任何符合`boost.range`要求的容器，如标准库的`vector`、`deque`和`list`等。

`string_algo`库中的算法命名遵循标准库的惯例，算法名均为小写形式，并使用不同的前缀和后缀来区分不同的版本，命名规则如下：

- 前缀i：有这个前缀表明算法是大小写不敏感的，否则是大小写敏感的；
- 后缀`_copy`：有这个后缀表明算法不变动输入，返回处理结果的拷贝，否则算法原地处理，输入即输出；
- 后缀`_if`：这个后缀表明算法需要一个作为判断式的谓词函数对象，否则使用默认的判断准则。

**1）大小写转换**

```C++
template<typename T> void to_upper(T& Input); //转为大写
template<typename T> void to_lower(T& Input); //转为小写
```

这两个算法具有后缀为`_copy`的版本，返回变动后的结果。

**2）字符串判断**

- starts_with：检测一个字符串是否是另一个字符串的前缀；
- ends_with：检测一个字符串是否是另一个字符串的后缀；
- contains：检测一个字符串是否被另一个包含；
- equals：检测两个字符串是否相等；
- lexicographical_compare：根据字典顺序检测一个字符串是否小于另一个；
- all：检测一个字符串中的所有元素是否满足指定的判断式。

上述算法，除了all以外，都有一个i前缀的版本，用于进行大小写无关的字符串比较问题。

**3）分类函数**

`string_algo`库同时也提供了一组分类函数，可以用于检测一个字符是否符合某种特性，主要用于搭配其他算法使用，如all算法。

- is_space：字符是否为空格；
- is_alnum：字符是否为字母和数字；
- is_alpha：字符是否为字母；
- is_cntrl：字符是否为控制字符；
- is_digit：字符是否为十进制数字；
- is_xdigit：字符是否为十六进制数字；
- is_graph：字符是否为图形字符；
- is_lower：字符是否为小写字符；
- is_upper：字符是否为大写字符；
- is_print：字符是否为可打印字符；
- is_punct：字符是否为标点符号字符；
- is_any_of：字符是否是参数字符序列中的任意字符；
- is_from_range：字符是否位于指定区间内，即`from <= ch <= to`。

这些函数并不是真正的检测字符，而是返回类型为`detail::isClassifiedF`的函数对象，这个函数对象的operator()才是真正的分类函数。

函数对象`is_classifiedF`重载了逻辑运算符`||`、`&&`和`!`，可以使用逻辑运算符把它们组合成逻辑表达式，以实现更复杂的条件判断。

```C++
inline detail::is_classifiedF is_space(const std::locale& Loc=std::locale())
{
    return detail::is_classifiedF(std::ctype_base::space, Loc);
}

inline detail::is_classifiedF is_alpha(const std::locale& Loc=std::locale())
{
    return detail::is_classifiedF(std::ctype_base::alpha, Loc);
}
```

如果`string_algo`提供的分类判断式不能满足要求，我们也可以自己实现专用的判断式。方法很简单，定义一个返回值为bool的函数对象就可以，那么最简单的就是lambda表达式了。

```C++
// 等效于判断式boost::is_any_of("01")
auto pred_func = [] (char ch) { return ch == '0' || ch == '1';};
str = "0101digit1010";
cout<<trim_copy_if(str, pred_func);  //删除两边的数字
```

**4）修剪算法**

`string_algo`提供了3个修剪算法：`trim_left`、`trim_right`和`trim`，用于删除字符串开头或结尾部分的空格。它有`_if`和`_copy`两种后缀，因此每个算法都有四个版本。

```C++
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
using namespace boost;
using namespace std;
int main()
{
    string str = "   space   ";
    cout << trim_copy(str) << endl;  //删除两边的空格
    cout << trim_left_copy(str) << endl; //删除左边的空格
    trim(str);
    cout << str << endl;
    
    str = "..222digit222..";
    cout<<trim_copy_if(str, is_punct() || is_digit());  //删除两边的标点和数字
}
```

**5）替换与删除算法**

- replace/erase_first：替换/删除一个字符串在输入中第一次出现的位置；
- replace/erase_last：替换/删除一个字符串在输入中最后一次出现的位置；
- replace/erase_nth：替换/删除一个字符串在输入中第n次出现的位置；
- replace/erase_all  ：替换/删除一个字符串在输入中所有出现的位置；

这八个算法每个都有前缀i、后缀`_copy`的组合。



**6）分割与合并**

split可以使用某种策略，将字符串分割成若干部分，并将分割后的字符串拷贝存入指定的容器内。join则是把存储在容器中的字符串连成一个新的字符串，并且可以指定连接的分隔符。

分割与合并算法的容器的元素类型必须是`string`或`iterator_range<string::iterator>`，容器则可以是`vector`、`list`或`deque`等标准容器。算法的声明如下：

```C++
// split算法使用判断式Pred来确定分割的依据，如果字符ch满足判断式，那么他就是分隔符；
// 参数eCompress可以取值为token_compress_off或token_compress_on,
// token_compress_on表示多个连续的分隔符会被视为一个，token_compress_off则相反。
template< typename SequenceSequenceT, typename RangeT, typename PredicateT >
inline SequenceSequenceT& split(
    SequenceSequenceT& Result,
    RangeT& Input,
    PredicateT Pred,
    token_compress_mode_type eCompress=token_compress_off );

// join算法可以看成split的逆运算。
template< typename SequenceSequenceT, typename Range1T>
inline typename range_value<SequenceSequenceT>::type  join(
    const SequenceSequenceT& Input,
    const Range1T& Separator);

// join_if接受一个判断式，只有满足判断式的字符串才能参与合并
template< typename SequenceSequenceT, typename Range1T, typename PredicateT>
inline typename range_value<SequenceSequenceT>::type  join_if(
    const SequenceSequenceT& Input,
    const Range1T& Separator,
    PredicateT Pred);
```

使用示例如下：

```C++
#include <boost/algorithm/string.hpp>
#include <list>
int main()
{
    std::list<std::string> str_list;
    std::string target = "1,2,3,,,,,,4,5,6";
    boost::split(str_list, target, boost::is_any_of(","), boost::token_compress_on);
    for (auto& s : str_list)
    {
        std::cout << s << std::endl;
    }

    std::string result = boost::join(str_list, "+");
    std::cout << result << std::endl;
}
```



### 3.4 tokenizer





## 4. 文件与目录操作

