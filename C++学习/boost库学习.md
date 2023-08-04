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

## 4. 文件系统

filesystem库是一个可移植的文件系统操作库，它在底层做了大量的工作，使用POSIX标准表示文件系统的路径，使得C++具有类似脚本语言的功能，可以跨平台操作目录、文件，写出通用的脚本程序。

### 4.1 路径表示

filesystem库的核心类是path，它屏蔽了不同文件系统的差异，使用可移植的POSIX语法提供通用的目录及路径表示，并且支持POSIX的符号链接概念。path类摘要如下：

```C++
class path
{
public:
    //创建一个空路径
    path() BOOST_NOEXCEPT {}

    //根据给定字符串创建对象
    path(const char* s) : m_pathname(s) {}
    path(char* s) : m_pathname(s) {}
    path(const string& s) : m_pathname(s) {}
    path(string& s) : m_pathname(s) {}
    template <class InputIterator>
    path(InputIterator begin, InputIterator end);

    //添加元素到带目录分隔符的路径
    path& operator/=(const string& s);
    path& operator/=(const char* s);
    path& append(const char* ptr);
    
    //修改路径
    void clear() { m_pathname.clear(); }
    void swap(path& rhs);
    //将路径的通用格式视图中的所有目录分隔符转换成偏好目录分隔符
    path& make_preferred();
    //删除路径中最后的文件名，把path编程纯路径表示
    path& remove_filename();
    //变更文件的扩展名
    path& replace_extension(const path& new_extension = path());
    
    //返回路径的原生版本
    const string& native() const;
    const char* c_str() const;
    string string() const;
    
    //返回转换到通用的路径名格式
    path generic_path() const;
    string generic_string() const { return generic_path().string(); }
    
    //以字典序比较路径
    int compare(const path& p) const BOOST_NOEXCEPT; 
    int compare(const string& s) const { return compare(path(s)); }
    int compare(const char* s) const  { return compare(path(s)); }
    
    //分解
	path  root_path() const;     //若存在则返回路径的根名
	path  root_name() const;     //若存在则返回路径的根目录
	path  root_directory() const;//若存在则返回路径的根路径
	path  relative_path() const; //返回相对根路径的路径
	path  parent_path() const;   //返回亲路径的路径
	path  filename() const;      //返回文件名
	path  stem() const;          //返回路径所标识的文件名的主干
    
    //查询
    bool empty() const;  //判断是否为空路径
    bool filename_is_dot() const; 
    bool filename_is_dot_dot() const;
    bool has_root_path() const       
    bool has_root_name() const       
    bool has_root_directory() const  
    bool has_relative_path() const   
    bool has_parent_path() const     
    bool has_filename() const        
    bool has_extension() const       
    bool is_relative() const         
    bool is_absolute() const

private:
    string m_pathname;
};
```

使用示例如下：

```C++
int main()
{
    boost::filesystem::path fp("/home/tony/start");
    cout << fp << endl;  // "/home/tony/start"
    fp /= "test.cpp";
    cout << fp.string() << endl; // /home/tony/start/test.cpp
    cout << fp.generic_string() << endl; // /home/tony/start/test.cpp
    cout << fp.root_path() << endl;  // "/"
    cout << fp.root_name() << endl;  // ""
    cout << fp.root_directory() << endl;  // "/"
    cout << fp.relative_path() << endl;  // "home/tony/start/test.cpp"
    cout << fp.parent_path() << endl;  // "/home/tony/start"
    cout << fp.filename() << endl;  // "test.cpp"
    cout << fp.stem() << endl;  // "test"
}
```

path的构造函数没有声明为explicit，因此字符串可以被隐式转换为path对象，这在编写代码时非常方便，可以不用创建一个临时的path对象。path重载了`operator/=`操作符，可以向使用普通路径一样用`/`来追加路径，成员函数append也有同样的功能。

### 4.2 可移植的文件名

需要注意的是，path仅仅用于表示路径，而并不关心路径中的文件或目录是否存在，路径也可能在当前文件系统的是无效的名字。因此，`filesystem`库提供了一系列的检查函数，来根据系统命名规则判断文件名的有效性。

函数`portable_posix_name()`和`windows_name()`分别检测文件名字符串是否符合POSIX规范和Windows规范，保证名字可以移植到符合POSIX的类Unix系统和Windows系统上。POSIX规范只有一个很小的字符集用于文件名，包括大小写字母、点号、下划线和连字符；而Windows系统则范围要广一些，仅不允许`<>?:|/\`等少量字符。如：

```c++
string fname("w+abc.xxx");
assert(!boost::filesystem::portable_posix_name(fname));  //posix非法文件名
assert(boost::filesystem::windows_name(fname));   //Windows合法文件名
```

函数`portable_name()`则判断名字是否是一个可移植的文件名，相当于``portable_posix_name() && windows_name()`。但名字不能以点号或者连字符开头，并允许表示当前目录的`.`和父目录的`..`。

`portable_directory_name()`的判断规则进一步严格，它包含`portable_name()`。并且要求名字中不能出现点号。`portable_file_name()`则要求文件名中最多有一个点号，且后缀不能超过3个字符。

```C++
assert(!boost::filesystem::portable_name("./abc.xxx"));
assert(boost::filesystem::portable_directory_name("abcd"));  //合法目录名
assert(boost::filesystem::portable_file_name("abc.xxx"));    //合法文件名
```

### 4.3 文件状态与属性

filesystem提供了一组状态判断函数，便于我们简化对文件状态的判断：

```C++
// 路径是否存在
bool exists(const path& p);
// 路径是否是目录
bool is_directory(const path& p);
// 路径是否是普通文件
bool is_regular_file(const path& p);
// 路径是否是符号链接文件
bool is_symlink(const path& p);
// 如果path是目录，当目录中没有文件时，返回true；
// 如果path是文件，当文件长度为0时，返回true.
bool is_empty(const path& p);
// 当文件存在，且不是普通文件、目录或者链接文件时，返回true
bool is_other(const path& p);
```

受可移植性的限制，很多文件属性不是各平台共通的，因此filesystem库仅提供少量的文件属性操作：

- 函数file_size()：以字节为单位返回文件的大小；
- 函数last_write_time()：返回文件的最后修改时间，类型为`std::time_t`。

这两个函数都要求操作的文件必须存在，否则会抛出异常，file_size还要求文件必须是普通文件。

使用示例：

```C++
std::string fname("./test_file.txt");
if (boost::filesystem::exists(fname))
{
    std::time_t t = boost::filesystem::last_write_time(fname);
    cout << std::ctime(&t) << endl;

    if (boost::filesystem::is_regular_file(fname))
    {
        cout << boost::filesystem::file_size(fname) << endl;
    }
}   
```

### 4.4 文件操作

filesystem库基于path的路径表示提供了基本的文件操作函数：

- create_directory和create_directories：创建目录；create_directory若目录存在，则返回false，创建成功则返回true；**create_directories可用于递归创建多级目录**。
- rename：文件改名；
- remove和remove_all：文件删除；remove智能删除空目录或者文件，remove_all则可以递归删除目录和文件。
- copy、copy_file、copy_directory：文件拷贝；copy用于创建目录、文件或者符号链接；copy_directory用于复制目录；copy_file用于复制文件，可选项fail_if_exists和overwrite_if_exists，默认为fail_if_exists，即如果目标对象存在则会抛出异常。

使用示例：

```C++
int main()
{
    boost::filesystem::path ptest = "d:/test";
    if (exists(ptest))
    {
        if (boost::filesystem::is_empty(ptest))
        {
            boost::filesystem::remove(ptest);
        } 
        else
        {
            boost::filesystem::remove_all(ptest);
        }
    } 
    boost::filesystem::create_directory(ptest);

    boost::filesystem::copy_file("d:/testfile.txt", ptest / "file.txt");
    assert(boost::filesystem::exists(ptest / "file.txt"));
    boost::filesystem::rename(ptest / "file.txt", ptest / "file.cpp");
    //使用create_directories一次创建多级目录
    create_directories(ptest / "sub_dir" / "sub_dir");
}
```

### 4.5 遍历目录

filesystem库提供了directory_iterator和recursive_directory_iterator用于遍历目录下面的所有文件，directory_iterator只遍历当前目录下的所有文件，recursive_directory_iterator则能够递归遍历目录及其子目录的所有文件。

directory_iterator的用法类似与string_algo库的find_iterator，通过定义一个空的directory_iterator作为end迭代器，目标目录的path对象作为迭代的起始，通过反复调用operator++即可遍历目录下的所有文件。

```C++
path target("d:/");
// 方法1
boost::filesystem::directory_iterator end;
for (boost::filesystem::directory_iterator pos(target); pos !=end; ++pos)
{
    cout << pos->path() << endl;
}

// 方法2
boost::filesystem::directory_iterator it(target), end;
BOOST_FOREACH(boost::filesystem::path const &p, std::make_pair(it, end))
{
    cout << p << endl;
}
```

需要注意的是，directory_iterator迭代器返回的对象并不是path，而是一个directory_entry对象，通过调用其方法path()可以获得path对象。

recursive_directory_iterator能够深度搜索目录，迭代当前目录及其子目录下的所有文件。其一些成员函数可以帮助我们实现指定深度的目录遍历：

- level()返回当前的目录深度m_level；当recursive_directory_iterator构造时，m_level为0，每深入一级子目录m_level加1，退出时减1。
- pop()用于退出当前目录层次的遍历，同时m_level减1。
- no_push()可以让目录不参与遍历，使其行为等价于directory_iterator。

```C++
boost::filesystem::recursive_directory_iterator end;
for (boost::filesystem::recursive_directory_iterator pos("d:/test"); pos !=end; ++pos)
{
    cout << pos->path() << " " << pos.level()<< endl;
}
```

