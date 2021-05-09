# C++并发编程实战

## 2. 线程管理

### 2.1 线程管理基础

```C++
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
 
void f1(int n)
{
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread 1 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
 
void f2(int& n)
{
    for (int i = 0; i < 5; ++i) {
        std::cout << "Thread 2 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
 
class foo
{
public:
    void bar()
    {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Thread 3 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    int n = 0;
};
 
class baz
{
public:
    void operator()()
    {
        for (int i = 0; i < 5; ++i) {
            std::cout << "Thread 4 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    int n = 0;
};
 
int main()
{
    int n = 0;
    foo f;
    baz b;
    std::thread t1; // t1 不是线程
    std::thread t2(f1, n + 1); // 按值传递
    std::thread t3(f2, std::ref(n)); // 按引用传递
    std::thread t4(std::move(t3)); // t4 现在运行 f2() 。 t3 不再是线程
    std::thread t5(&foo::bar, &f); // t5 在对象 f 上运行 foo::bar()
    std::thread t6(b); // t6 在对象 b 的副本上运行 baz::operator()
    t2.join();
    t4.join();
    t5.join();
    t6.join();
    std::cout << "Final value of n is " << n << '\n';
    std::cout << "Final value of f.n (foo::n) is " << f.n << '\n';
    std::cout << "Final value of b.n (baz::n) is " << b.n << '\n';
}
```

thread对象不支持拷贝，只支持移动。线程在`std::thread`对象创建(为线程指定任务)时开始运行。启动线程后，你需要明确是要等待线程结束（join），还是让其自主运行（detach）。如果在thread对象销毁前还未作出选择，程序就会终止线程（thread的析构函数中，会调用terminate函数来终止线程）。因此，即便是有异常存在，也需要确保线程能够正确的*加入*(joined)或*分离*(detached)。



如果不等待线程，就必须保证线程结束之前，可访问的数据得有效性。这不是一个新问题——单线程代码中，对象销毁之后再去访问，也会产生未定义行为——不过，线程的生命周期增加了这个问题发生的几率。这种情况很可能发生在线程还没结束，函数已经退出的时候，这时线程函数还持有函数局部变量的指针或引用。

```C++
struct func
{
  int& i;
  func(int& i_) : i(i_) {}
  void operator() ()
  {
    for (unsigned j=0 ; j<1000000 ; ++j)
    {
      do_something(i);           // 1 潜在访问隐患：悬空引用
    }
  }
};
void oops()
{
  int some_local_state=0;
  func my_func(some_local_state);
  std::thread my_thread(my_func);
  my_thread.detach();          // 2 不等待线程结束
}                             // 3 新线程可能还在运行
```

处理这种情况的常规方法：使线程函数的功能齐全，将数据复制到线程中，而非复制到共享数据中。如果使用一个可调用的对象作为线程函数，这个对象就会复制到线程中，而后原始对象就会立即销毁。

此外，可以通过join()函数来确保线程在函数完成前结束。

join()是简单粗暴的等待线程完成或不等待。当你需要对等待中的线程有更灵活的控制时，比如，看一下某个线程是否结束，或者只等待一段时间(超过时间就判定为超时)。想要做到这些，你需要使用其他机制来完成，比如条件变量和*期待*(futures)，相关的讨论将会在第4章继续。调用join()的行为，还清理了线程相关的存储部分，这样`std::thread`对象将不再与已经完成的线程有任何关联。这意味着，只能对一个线程使用一次join();一旦已经使用过join()，`std::thread`对象就不能再次加入了，当对其使用joinable()时，将返回false。

如果想要分离一个线程，可以在线程启动后，直接使用detach进行分离。如果打算等待对应线程，则需要细心挑选调用join的位置。需要注意线程运行时是否可能在调用join前程序就抛出异常退而终止了。对于这种情况，需要在异常处理过程中调用join，从而避免生命周期的问题。

还有一种方式是，使用RAII思想封装thread，在对象析构时调用join函数。

```C++
class thread_guard
{
  std::thread& t;
public:
  explicit thread_guard(std::thread& t_):
    t(t_)
  {}
  ~thread_guard()
  {
    if(t.joinable()) // 1
    {
      t.join();      // 2
    }
  }
  thread_guard(thread_guard const&)=delete;   // 3
  thread_guard& operator=(thread_guard const&)=delete;
};
struct func; // 定义在清单2.1中
void f()
{
  int some_local_state=0;
  func my_func(some_local_state);
  std::thread t(my_func);
  thread_guard g(t);
  do_something_in_current_thread();
} 
```

使用detach会让线程在后台运行，这意味着主线程不会再等在这个线程结束。如果线程分离了，那么就不可能有thread对象能引用它，分离后的线程，其状态joinable将变为false。C++运行库保证，线程退出时，相关资源能够被正确回收。

### 2.2 向线程函数传递参数

在thread的构造函数中，基本就是简单的将参数传递给给可调用对象或函数。需要注意的是，参数会默认被拷贝到线程的独立内存空间后再传递给可调用对象或者函数，不管函数需求的是参数的引用；这样做的目的是方便在新线程中进行访问。

```C++
void f(int i,std::string const& s);
void oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i",some_param);
    // buffer是指针，一个指向局部变量，该指针通过thread构造函数传递到新线程中；
    // 在新线程传递参数给函数时，会调用string的构造函数，完成字面量到string类型的转换；
    // 而函数oops可能会在上面转换完成前就退出了，这会导致buffer成为一个野指针，程序出现未定义的行为
    std::thread t(f,3,buffer);
    t.detach();
}
```

解决方案就是在传递到`std::thread`构造函数之前就将字面值转化为`std::string`对象：

```C++ 
void f(int i,std::string const& s);
void not_oops(int some_param)
{
  char buffer[1024];
  sprintf(buffer,"%i",some_param);
  std::thread t(f,3,std::string(buffer));  // 使用std::string，避免悬垂指针
  t.detach();
}
```

还可能遇到相反的情况：期望传递一个引用，但整个对象被复制了。当线程更新一个引用传递的数据结构时，这种情况就可能发生，比如：

```C++
void update_data_for_widget(widget_id w,widget_data& data); // 1
void oops_again(widget_id w)
{
  widget_data data;
  std::thread t(update_data_for_widget,w,data); // 2
  display_status();
  t.join();
  process_widget_data(data); // 3
}
```

虽然update_data_for_widget①的第二个参数期待传入一个引用，但是`std::thread`的构造函数②并不知晓；构造函数无视函数期待的参数类型，并盲目的拷贝已提供的变量。当线程调用update_data_for_widget函数时，传递给函数的参数是data变量内部拷贝的引用，而非数据本身的引用。因此，当线程结束时，内部拷贝数据将会在数据更新阶段被销毁，且process_widget_data将会接收到没有修改的data变量③。对于熟悉`std::bind`的开发者来说，问题的解决办法是显而易见的：可以使用`std::ref`将参数转换成引用的形式，从而可将线程的调用改为以下形式：

```C++
std::thread t(update_data_for_widget,w,std::ref(data));
```

在这之后，update_data_for_widget就会接收到一个data变量的引用，而非一个data变量拷贝的引用。

还有一种情况，即**传递的参数只能被移动，不能被拷贝**，如`std::unique_ptr`。当原对象是一个临时变量时，移动语义会自动生效；但当原对象是一个命名变量时，那么转移的时候就需要使用`std::move()`进行显式移动。

```C++
void process_big_object(std::unique_ptr<big_object>);

std::unique_ptr<big_object> p(new big_object);
p->prepare_data(42);
std::thread t(process_big_object,std::move(p));
```

在`std::thread`的构造函数中指定`std::move(p)`,big_object对象的所有权就被首先转移到新创建线程的的内部存储中，之后传递给process_big_object函数。

### 2.3 转移线程的所有权

假设你想要编写一个函数，该函数用于创建一个后台运行的线程，并且需要将新线程所有权交给函数调用方；又或者需要将线程的所有权交给需要等待其完成的函数。这些情况下，你都需要进行线程所有权转移的操作。这就是thread支持移动语义的原因。

```C++
std::thread f()
{
  void some_function();
  return std::thread(some_function);
}
std::thread g()
{
  void some_other_function(int);
  std::thread t(some_other_function,42);
  return t;
}
```

### 2.4 运行时决定线程数量

`std::thread::hardware_concurrency()`在新版C++标准库中是一个很有用的函数。这个函数会返回能并发在一个程序中的线程数量。例如，多核系统中，返回值可以是CPU核芯的数量。返回值也仅仅是一个提示，当系统信息无法获取时，函数也会返回0。

实例实现了一个并行版的`std::accumulate`。代码中将整体工作拆分成小任务交给每个线程去做，其中设置最小任务数，是为了避免产生太多的线程。程序可能会在操作数量为0的时候抛出异常。比如，`std::thread`构造函数无法启动一个执行线程，就会抛出一个异常。在这个算法中讨论异常处理，已经超出现阶段的讨论范围，这个问题我们将在第8章中再来讨论。

```C++
template<typename Iterator, typename T>
struct accumulate_block
{
    void operator()(Iterator first, Iterator last, T& result)
    {
        result = std::accumulate(first, last, result);
    }
};
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);
    if (!length) // 1
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =
        (length + min_per_thread - 1) / min_per_thread; // 2
    unsigned long const hardware_threads =
        std::thread::hardware_concurrency();
    unsigned long const num_threads =  // 3
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads; // 4
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);  // 5
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);  // 6
        threads[i] = std::thread(     // 7
            accumulate_block<Iterator, T>(),
            block_start, block_end, std::ref(results[i]));
        block_start = block_end;  // #8
    }
    accumulate_block<Iterator, T>()(
        block_start, last, results[num_threads - 1]); // 9
    std::for_each(threads.begin(), threads.end(),
        std::mem_fn(&std::thread::join));  // 10
    return std::accumulate(results.begin(), results.end(), init); // 11
}
```

函数看起来很长，但不复杂。如果输入的范围为空①，就会得到init的值。反之，如果范围内多于一个元素时，都需要用范围内元素的总数量除以线程(块)中最小任务数，从而确定启动线程的最大数量②，这样能避免无谓的计算资源的浪费。比如，一台32芯的机器上，只有5个数需要计算，却启动了32个线程。

计算量的最大值和硬件支持线程数中，较小的值为启动线程的数量③。因为上下文频繁的切换会降低线程的性能，所以你肯定不想启动的线程数多于硬件支持的线程数量。当`std::thread::hardware_concurrency()`返回0，你可以选择一个合适的数作为你的选择；在本例中，我选择了”2”。你也不想在一台单核机器上启动太多的线程，因为这样反而会降低性能，有可能最终让你放弃使用并发。

