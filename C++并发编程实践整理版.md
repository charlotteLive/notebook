# C++并发编程实践整理版

## 1. C++11线程组件

### 1.1 线程对象thread

thread对象不支持拷贝，只支持移动。创建线程后，需要明确是要你需要明确是要等待线程结束（join），还是让其自主运行（detach）。如果在thread对象销毁前还未作出选择，程序就会终止线程（thread的析构函数中，会调用terminate函数来终止线程）。

```C++
class thread
{
public:
    thread() noexcept;  //构造不创建线程的新thread对象
    template<class Function, class ...Args>
    explicit thread(Function&& f, Args&&... args); //构造thread对象并将它与执行线程关联
    thread(thread&& other) noexcept; //移动构造函数
    thread(const thread&) = delete;  //thread对象不可复制

    void join(); //阻塞当前线程，直至所标识的线程执行完成
    void detach(); //从thread对象分离执行的线程，允许独立地继续执行；线程退出时释放所有分配的资源

    //检查thread对象是否标识活跃的执行线程；故默认构造的 thread 不可合并。
    bool joinable() const noexcept; 
    thread::id get_id() const noexcept; //返回关联线程的id类型值
    native_handle_type native_handle(); //返回实现定义的底层线程句柄
    static unsigned int hardware_concurrency() noexcept; //返回实现所支持的并发线程数
};
```

**注意事项1：正确传递执行函数所需的引用类型参数。**

调用thread构造函数时，基本就是先将参数默认拷贝到线程的私有内存空间，然后再传递给可调用对象或函数。如果可调用对象或者函数需要的是参数的引用，那么该参数引用可能指向的是一个线程私有空间中的一个临时变量，导致悬垂引用等错误。

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

可行的解决方案：在传递到`std::thread`构造函数之前就将字面值转化为`std::string`对象。

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

如果想将变量的引用正确传递给线程，应**使用`std::ref`将参数转换成引用的形式**。

**注意事项2：当传递变量引用或指针给执行函数时，应注意变量在线程执行期间的有效性。**

如果调用detach分离线程，当执行函数的入参有引用或指针类型时，应确保访问数据的有效性，避免出现数据销毁之后再去访问的场景。这种情况很可能发生在线程还没结束，函数已经退出的时候，这时线程函数还持有函数局部变量的指针或引用。

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

可行的解决方案：1）在可行的前提下，尽可能地选择将数据复制到线程中，而复制到共享数据中。2）使用join保证在数据销毁前，线程已经执行完成。

**注意事项3：选择合适的地方调用join，避免在调用join前程序就抛出异常终止了。**

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
```

使用detach会让线程在后台运行，这意味着主线程不会再等在这个线程结束。如果线程分离了，那么就不可能有thread对象能引用它，分离后的线程，其状态joinable将变为false。C++运行库保证，线程退出时，相关资源能够被正确回收。

### 1.2 互斥锁

#### 1.2.1 mutex类

**`mutex` 类**是能用于保护共享数据免受从多个线程同时访问的同步原语。`mutex` 提供排他性非递归所有权语义：调用`mutex`的线程从她成功调用lock或try_lock开始，到调用unlock的这段时间内占有mutex。线程占有mutex时，其他线程若试图获取mutex 的所有权，将会阻塞（对于 lock 的调用）或收到 false 返回值（对于 try_lock ）。若 `mutex` 在被线程所占有时被销毁，或在占有 `mutex` 时线程终止，会出现行为未定义的问题。

```C++
class mutex {
 public:
    constexpr mutex() noexcept;
    ~mutex();
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;
 
    void lock(); //锁定互斥，若互斥不可用则阻塞
    //尝试锁定互斥并立即返回; 成功获得锁时返回true，否则返回false;
    //若已占有 mutex 的线程调用 try_lock ，则行为未定义。
    bool try_lock(); 
    void unlock(); //解锁互斥
    typedef /* 由实现定义 */ native_handle_type;
    native_handle_type native_handle();
};
```

通常不直接使用mutex，而是使用更高层次的封装如`lock_guard`或`unique_lock`，以更加异常安全的方式管理锁定。

#### 1.2.2  lock_guard 类

 `lock_guard` 类是互斥封装器，为在作用域块期间占有互斥提供便利RAII 风格机制。`lock_guard`有两种构造函数，并且禁止拷贝：

```C++
// 等效地调用 m.lock() 。若 m 不是递归互斥，且当前线程已占有 m 则行为未定义。
explicit lock_guard( mutex_type& m );
// 获得互斥m的所有权而不试图锁定它, 若当前线程不占有m则行为未定义。
// 用于调用方已锁定互斥元的场景
lock_guard( mutex_type& m, std::adopt_lock_t t );
```

创建 `lock_guard` 对象时，它试图接收给定互斥的所有权。控制离开创建 `lock_guard` 对象的作用域时，销毁 `lock_guard` 并释放互斥。

```C++
std::list<int> some_list; 
std::mutex some_mutex;  
void add_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);
    some_list.push_back(new_value);
}
bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex); 
    return std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}
```

某些情况下使用全局变量没问题，但在大多数情况下，**互斥量通常会与需要保护的数据放在同一类中**，而不是定义成全局变量。这是面向对象设计的准则：将其放在一个类中，就可让他们联系在一起，也可对类的功能进行封装，并进行数据保护。

#### 1.2.3 unique_lock类

与 `lock_guard`相比，`unique_lock`是一个通用互斥包装器。支持延迟锁定、锁定的有时限尝试、递归锁定。`unique_lock`不支持拷贝，但支持移动。

```C++
template<class Mutex>
class unique_lock {
public:
    using mutex_type = Mutex;
 
    // 构造/复制/销毁
    unique_lock() noexcept;
    explicit unique_lock(mutex_type& m);
    unique_lock(mutex_type& m, defer_lock_t) noexcept;
    unique_lock(mutex_type& m, try_to_lock_t);
    unique_lock(mutex_type& m, adopt_lock_t);
    template<class Clock, class Duration>
      unique_lock(mutex_type& m, const chrono::time_point<Clock, Duration>& abs_time);
    template<class Rep, class Period>
      unique_lock(mutex_type& m, const chrono::duration<Rep, Period>& rel_time);
    ~unique_lock();
 
    unique_lock(const unique_lock&) = delete;
    unique_lock& operator=(const unique_lock&) = delete;
 
    unique_lock(unique_lock&& u) noexcept;
    unique_lock& operator=(unique_lock&& u);
 
    // 锁定
    void lock();
    bool try_lock();
    template<class Rep, class Period>
      bool try_lock_for(const chrono::duration<Rep, Period>& rel_time);
    template<class Clock, class Duration>
      bool try_lock_until(const chrono::time_point<Clock, Duration>& abs_time);
    void unlock();
 
    // 修改器
    void swap(unique_lock& u) noexcept;
    mutex_type* release() noexcept; //将关联互斥解关联而不解锁它
 
    // 观察器
    bool owns_lock() const noexcept;
    explicit operator bool () const noexcept;
    mutex_type* mutex() const noexcept;
};
```

这里解释下`unique_lock`几个构造函数的参数含义：

- `adopt_lock`假定线程已经占有互斥锁，构造时就不再锁定互斥了。如果使用该构造函数时，互斥没有被锁定，行为是未定义的。
- `defer_lock`假定线程不占有互斥锁，构造时也不锁定互斥，由用户在构造后适当的时机锁定互斥；如果使用该构造函数时线程占有了互斥锁，行为是未定义的。
- `try_to_lock`尝试锁定关联互斥而不阻塞。若当前线程已占有互斥则行为未定义，除非互斥是递归的。

`unique_lock`提供了加锁解锁的用户接口，相比于`guard_lock`能够实现更细粒度的锁定，更加灵活。

```C++
void shared_print(string msg, int id) {
    std::unique_lock<std::mutex> guard(_mu, std::defer_lock);
    //do something 1

    guard.lock();
    // do something protected
    guard.unlock(); //临时解锁

    //do something 2

    guard.lock(); //继续上锁
    // do something 3
    f << msg << id << endl;
    cout << msg << id << endl;
    // 结束时析构guard会临时解锁
}
```



#### 1.2.4 其他类型的互斥锁

`timed_mutex`类，在`mutex`的基础上增加了`try_lock_for`和`try_lock_until`方法，以支持带时限要求地获取锁的所有权。即尝试锁定互斥元，阻塞线程直到经过指定的时间段（或时间点）或得到锁，取决于何者先到来。成功获得锁时返回true ，否则返回 false 。

`recursive_mutex` 类，递归锁，提供排他性递归所有权语义。可多次锁定互斥，但需要调用与锁定次数匹配的解锁操作，才能释放互斥。

```C++
class X {
    std::recursive_mutex m;
    std::string shared;
  public:
    void fun1() {
      std::lock_guard<std::recursive_mutex> lk(m);
      shared = "fun1";
      std::cout << "in fun1, shared variable is now " << shared << '\n';
    }
    void fun2() {
      std::lock_guard<std::recursive_mutex> lk(m);
      shared = "fun2";
      std::cout << "in fun2, shared variable is now " << shared << '\n';
      fun1(); // 递归锁在此处变得有用
      std::cout << "back in fun2, shared variable is " << shared << '\n';
    };
};
 
int main() 
{
    X x;
    std::thread t1(&X::fun1, &x);
    std::thread t2(&X::fun2, &x);
    t1.join();
    t2.join();
}
/*
可能的输出：
in fun1, shared variable is now fun1
in fun2, shared variable is now fun2
in fun1, shared variable is now fun1
back in fun2, shared variable is fun1
*/
```



#### 1.2.5 死锁与死锁的解决方案



### 3. 条件变量



### 4. 同步组件



### 5. 原子变量





## 2. 线程安全数据结构

### 2.1 线程安全队列



### 2.2 线程安全栈



### 2.3 线程池



## 3. 并发代码设计

