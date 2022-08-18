# C++模板

## 第1章 函数模板（ Function Templates ）

函数模板是被参数化的函数，因此他们代表的是一组具有相似行为的函数。

###  1.1 两阶段编译检查（ Two-Phase Translation ）

在实例化模板的时候，如果模板参数类型不支持所有模板中用到的操作符，将会遇到编译期错误。比如：

```c++
template<typename T>
T max (T a, T b)
{
    return b < a ? a : b;
}

std::complex<float> c1, c2; // std::complex<> 没有提供小于运算符
…
::max(c1,c2); // 编译期 ERROR
```


但是在定义的地方并没有遇到错误提示。这是因为模板是被分两步编译的：

1. 在模板定义阶段，模板的检查并不包含类型参数的检查。只包含下面几个方面：
- 语法检查。比如少了分号。
- 使用了未定义的不依赖于模板参数的名称（类型名，函数名，......）。
- 未使用模板参数的 static assertions。
2. 在模板实例化阶段，为确保所有代码都是有效的，模板会再次被检查，尤其是那些依赖于类型参数的部分。

名称被检查两次这一现象被称为“两阶段查找”，在 14.3.1 节中会进行更细致的讨论。

两阶段的编译检查给模板的处理带来了一个问题：当实例化一个模板的时候，编译器需要（一定程度上）看到模板的完整定义。这不同于函数编译和链接分离的思想，函数在编译阶段只需要声明就够了。第 9 章将讨论如何应对这一问题。我们将暂时采取最简单的方法：将模板的实现写在头文件里。

### 1.2 模板参数推断

当我们调用形如 max()的函数模板来处理某些变量时，模板参数将由被传递的调用参数决定。如果我们传递两个 int 类型的参数给模板函数，C++编译器会将模板参数 T 推断为 int。

不过 T 可能只是实际传递的函数参数类型的一部分。比如我们定义了如下接受常量引用作为函数参数的模板：

```c++
template<typename T>
T max (T const& a, T const& b)
{
	return b < a ? a : b;
}
```

此时如果我们传递 int 类型的调用参数，由于调用参数和 `int const &`匹配，类型参数 T 将被推断为 int。

在类型推断的时候自动的类型转换是受限制的：

- 如果调用参数是按引用传递的，任何类型转换都不被允许。通过模板类型参数 T 定义的两个参数，它们实参的类型必须完全一样。
- 如果调用参数是按值传递的，那么只有退化（decay）这一类简单转换是被允许的：const和 volatile 限制符会被忽略，引用被转换成被引用的类型，raw array 和函数被转换为相应的指针类型。通过模板类型参数 T 定义的两个参数，它们实参的类型在退化（decay）后必须一样。

```c++
template<typename T>
T max (T a, T b);
…
int const c = 42;
int i = 1;
max(i, c); // OK: T 被推断为 int ， c 中的 const 被 decay 掉
max(c, c); // OK: T 被推断为 int
int& ir = i;
max(i, ir); // OK: T 被推断为 int ， ir 中的引用被 decay 掉
int arr[4];
foo(&i, arr); // OK: T 被推断为 int*

// 错误示例
max(4, 7.2); // ERROR: 不确定 T 该被推断为 int 还是 double
std::string s;
foo("hello", s); //ERROR: 不确定 T 该被推断为 const[6] 还是 std::string
```

有两种办法解决以上错误：
1. 对参数做类型转换

  ```c++
  max(static_cast<double>(4), 7.2); // OK
  ```

2. 显式地指出类型参数 T 的类型，这样编译器就不再会去做类型推导。

  ```c++
  max<double>(4, 7.2); // OK
  ```

需要注意的是，类型推断并不适用于默认调用参数。例如：

```c++
template<typename T>
void f(T = "");
...
f(1); // OK: T 被推断为 int, 调用 f<int> (1)
f(); // ERROR: 无法推断 T 的类型
```

为应对这一情况，你需要给模板类型参数也声明一个默认参数：

```c++
template<typename T = std::string>
void f(T = "");
…
f(); // OK
```

### 1.3 返回类型推断

如果返回类型是由模板参数决定的，那么推断返回类型最简单也是最好的办法就是让编译器来做这件事。从 C++14 开始，这成为可能，而且不需要把返回类型声明为任何模板参数类型（不过你需要声明返回类型为 auto）：

```c++
template<typename T1, typename T2>
auto max (T1 a, T2 b)
{
	return b < a ? a : b;
}
```

事实上，在不使用尾置返回类型（trailing return type）的情况下将 auto 用于返回类型，要求返回类型必须能够通过函数体中的返回语句推断出来。当然，这首先要求返回类型能够从函数体中推断出来。因此，必须要有这样可以用来推断返回类型的返回语句，而且多个返回语句之间的推断结果必须一致。

在 C++14 之前，要想让编译器推断出返回类型，就必须让或多或少的函数实现成为函数声明的一部分。在 C++11 中，尾置返回类型（trailing return type）允许我们使用函数的调用参数。也就是说，我们可以基于运算符`?:`的结果声明返回类型：

```c++
template<typename T1, typename T2>
auto max (T1 a, T2 b) -> decltype(b<a?a:b)
{
	return b < a ? a : b;
}
```

但是在某些情况下会有一个严重的问题：由于 T 可能是引用类型，返回类型就也可能被推断为引用类型。因此你应该返回的是 decay 后的 T，像下面这样：

```c++
#include <type_traits>
template<typename T1, typename T2>
auto max (T1 a, T2 b) -> typename std::decay<decltype(b<a? a:b)>::type
{
	return b < a ? a : b;
}
```

在这里我们用到了类型萃取（type trait）`std::decay<>`，它返回其 type 成员作为目标类型。由于其 type 成员是一个类型，为了获取其结果，需要用关键字 typename 修饰这个表达式。

### 1.4 按值传递还是按引用传递？

你可能会比较困惑，为什么我们声明的函数通常都是按值传递，而不是按引用传递。通常而言，建议将按引用传递用于除简单类型（比如基础类型和 std::string_view）以外的类型，这样可以免除不必要的拷贝成本。不过出于以下原因，按值传递通常更好一些：

- 语法简单。

- 编译器能够更好地进行优化。

- 移动语义通常使拷贝成本比较低。

- 某些情况下可能没有拷贝或者移动。

再有就是，对于模板，还有一些特有情况：

- 模板既可以用于简单类型，也可以用于复杂类型，因此如果默认选择适合于复杂类型可能方式，可能会对简单类型产生不利影响。
- 作为调用者，你通常可以使用 `std::ref()`和` std::cref()`（参见 7.3 节）来按引用传递参数。
- 虽然按值传递 string literal 和 raw array 经常会遇到问题，但是按照引用传递它们通常只会遇到更大的问题。第 7 章会对此做进一步讨论。在本书中，除了某些不得不用按引用传递的地方，我们会尽量使用按值传递。

## 2. 类模板

### 2.1 Stack类模板的实现

```c++
#include <vector>
#include <cassert>
template<typename T>
class Stack {
private:
    std::vector<T> elems; // elements
public:
    void push(T const& elem); // push element
    void pop(); // pop element
    T const& top() const; // return top element
    bool empty() const { // return whether the stack is empty
        return elems.empty();
    }
};

template<typename T>
void Stack<T>::push (T const& elem)
{
    elems.push_back(elem); // append copy of passed elem
}

template<typename T>
void Stack<T>::pop ()
{
    assert(!elems.empty());
    elems.pop_back(); // remove last element
}

template<typename T>
T const& Stack<T>::top () const
{
    assert(!elems.empty());
    return elems.back(); // return copy of last element
}
```

这个类的类型是 `Stack<T>`， 其中 T 是模板参数。在将这个` Stack<T>`类型用于声明的时候，除非可以推断出模板参数的类型，否则就必须使用 `Stack<T>`（Stack 后面必须跟着`<T>`）。不过，如果在类模板内部使用 Stack 而不是 `Stack<T>`，表明这个内部类的模板参数类型和模板类的参数类型相同。

比如，如果需要定义自己的复制构造函数和赋值构造函数，通常应该定义成这样：

```c++
template<typename T>
class Stack {
    …
    Stack (Stack const&); // copy constructor
    Stack& operator= (Stack const&); // assignment operator
    …
};
```

它和下面的定义是等效的：

```c++
template<typename T>
class Stack {
    …
    Stack (Stack<T> const&); // copy constructor
    Stack<T>& operator= (Stack<T> const&); // assignment operator
    …
};
```

一般`<T>`暗示要对某些模板参数做特殊处理，所以最好还是使用第一种方式。

但是如果在类模板的外面，就需要这样定义：

```c++
template<typename T>
bool operator== (Stack<T> const& lhs, Stack<T> const& rhs);
```

注意在只需要类的名字而不是类型的地方，可以只用 Stack。这和声明构造函数和析构函数
的情况相同。

定义类模板的成员函数时，必须指出它是一个模板，也必须使用该类模板的所有类型限制。因此，要像下面这样定义 `Stack<T>`的成员函数` push()`:

```c++
template<typename T>
void Stack<T>::push (T const& elem)
{
	elems.push_back(elem); // append copy of passed elem
}
```

### 2.2 Stack类模板的使用

直到 C++17，在使用类模板的时候都需要显式的指明模板参数。下面的例子展示了该如何使用 Stack<>类模板：

```c++
Stack< int> intStack; // stack of ints
Stack<std::string> stringStack; // stack of strings
// manipulate int stack
intStack.push(7);
std::cout << intStack.top() << ' \n ';
// manipulate string stack
stringStack.push("hello");
std::cout << stringStack.top() << ' \n ';
stringStack.pop();
```

注意，**模板函数和模板成员函数只有在被调用的时候才会实例化**。这样一方面会节省时间和空间，同样也允许只是部分的使用类模板。

在这个例子中，对` int` 和 `std::string`，默认构造函数，push()以及 top()函数都会被实例化。而pop()只会针对 `std::string` 实例化。如果一个类模板有 static 成员，对每一个用到这个类模板的类型，相应的静态成员也只会被实例化一次。

#### 2.2.1 部分地使用类模板

一个类模板通常会对用来实例化它的类型进行多种操作（包含构造函数和析构函数）。这可能会让你以为，要为模板参数提供所有被模板成员函数用到的操作。但是事实不是这样：**模板参数只需要提供那些会被用到的操作（而不是可能会被用到的操作）**。

比如 `Stack<>`类可能会提供一个成员函数 printOn()来打印整个 stack 的内容，它会调用`operator <<`来依次打印每一个元素：

```c++
template<typename T>
class Stack {
    …
    void printOn() (std::ostream& strm) const {
        for (T const& elem : elems) {
        	strm << elem << ' '; // call << for each element
        }
    }
};
```

这个类依然可以用于那些没有提供 operator <<运算符的元素：

```c++
Stack<std::pair< int, int>> ps; // note: std::pair<> has no operator<< defined
ps.push({4, 5}); // OK
ps.push({6, 7}); // OK
std::cout << ps.top().first <<  ' \n ' ; // OK
std::cout << ps.top().second <<  ' \n '; // OK
```

只有在调用 printOn()的时候，才会导致错误，因为它无法为这一类型实例化出对 `operator<<`的调用：

```c++
ps.printOn(std::cout); // ERROR: operator<< not supported for element type
```

### 2.3 友元

相比于通过 printOn()来打印 stack 的内容，更好的办法是去重载 stack 的 `operator <<`运算符。而且和非模板类的情况一样，`operator<<`应该被实现为非成员函数，在其实现中可以调用printOn()：

```c++
template<typename T>
class Stack {
    …
    void printOn() (std::ostream& strm) const {
    …
    }
    friend std::ostream& operator<< (std::ostream& strm, Stack<T> const& s) {
        s.printOn(strm);
        return strm;
    }
};
```

注意在这里 `Stack<>`的 `operator<<`并不是一个函数模板（对于在模板类内定义这一情况），而是在需要的时候，随类模板实例化出来的一个常规函数。

然而如果你试着先声明一个友元函数，然后再去定义它，情况会变的很复杂。事实上我们有两种选择：

1. 可以隐式的声明一个新的函数模板，但是必须使用一个不同于类模板的模板参数，比如用 U：

   ```c++
   template<typename T>
   class Stack {
       …
       template<typename U>
       friend std::ostream& operator<< (std::ostream&, Stack<U> const&);
   };
   ```

2. 也可以先将` Stack<T>`的 `operator<<`声明为一个模板，这要求先对` Stack<T>`进行声明：

   ```c++
   template<typename T>
   class Stack;
   template<typename T>
   std::ostream& operator<< (std::ostream&, Stack<T> const&);
   
   template<typename T>
       class Stack {
       …
       friend std::ostream& operator<< <T> (std::ostream&, Stack<T> const&);
   }
   ```

   注意这里在` operator<<`后面用了`<T>`，这相当于声明了一个特例化之后的非成员函数模板作为友元。如果没有`<T>`的话，则相当于定义了一个新的非模板函数。

无论如何，你依然可以将 `Stack<T>`用于没有定义 `operator <<`的元素，只是当你调用 `operator<<`的时候会遇到一个错误。