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



### 1.3 条件变量

C++标准库对条件变量有两套实现：`std::condition_variable`和`std::condition_variable_any`。这两个实现都包含在`<condition_variable>`头文件的声明中。两者都需要与一个互斥量一起才能工作(互斥量是为了同步)；前者仅限于与`std::mutex`一起工作，而后者可以和任何满足最低标准的互斥量一起工作，从而加上了*_any*的后缀。因为`std::condition_variable_any`更加通用，这就可能从体积、性能，以及系统资源的使用方面产生额外的开销，所以`std::condition_variable`一般作为首选的类型，当对灵活性有硬性要求时，我们才会去考虑`std::condition_variable_any`。

`std::condition_variable_any`的声明如下：

```C++
class condition_variable {
public:
    condition_variable();
    ~condition_variable();

    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;

    void notify_one() noexcept;
    void notify_all() noexcept;
    void wait(unique_lock<mutex>& lock);
    template<class Pred> 
    void wait(unique_lock<mutex>& lock, Pred pred);
    template<class Clock, class Duration>
    cv_status wait_until(unique_lock<mutex>& lock, const chrono::time_point<Clock, Duration>& abs_time);
    template<class Clock, class Duration, class Pred>
    bool wait_until(unique_lock<mutex>& lock, const chrono::time_point<Clock, Duration>& abs_time, Pred pred);
    template<class Rep, class Period>
    cv_status wait_for(unique_lock<mutex>& lock, const chrono::duration<Rep, Period>& rel_time);
    template<class Rep, class Period, class Pred>
    bool wait_for(unique_lock<mutex>& lock, const chrono::duration<Rep, Period>& rel_time, Pred pred);

    using native_handle_type = /* 由实现定义 */;
    native_handle_type native_handle();
};
```

wait/wait_for/wait_until会阻塞当前线程，直到条件变量被唤醒，或到指定时间后。这三个方法抖可以**传入条件判断，用于在等待特定条件为true时忽略虚假唤醒**。等效于`while(!pred())  wait(lock);`。执行wait时，会原子地解锁lock，阻塞当前线程；直到收到notify_all() 或 notify_one() 时解除阻塞，解除阻塞后，lock会再次锁定且wait退出。**调用wait前，需保证线程已获取到锁**，否则行为时未定义的。

示例程序如下：

```C++
std::mutex m;
std::condition_variable cv;
std::string data;
bool ready = false;
bool processed = false;

void worker_thread()
{
    // 等待直至 main() 发送数据
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [] {return ready; });

    // 等待后，我们占有锁。
    std::cout << "Worker thread is processing data\n";
    data += " after processing";

    // 发送数据回 main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";

    // 通知前完成手动解锁，以避免等待线程才被唤醒就阻塞（细节见 notify_one ）
    lk.unlock();
    cv.notify_one();
}

int main()
{
    std::thread worker(worker_thread);

    data = "Example data";
    // 发送数据到 worker 线程
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();

    // 等候 worker
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [] {return processed; });
    }
    std::cout << "Back in main(), data = " << data << '\n';

    worker.join();
}
```

上面的代码，我们注意到在调用wait之前锁定互斥元时，都是使用的`unique_lock`而不是`guard_lock`。在wait的检查条件不满足时，wait将解锁互斥元，并将该线程置于阻塞或等待状态。当收到notify（其他线程调用notify_one或notify_all）时，线程将从睡眠中唤醒，重新锁定互斥元，并再次检查条件。如果条件满足，就从wait返回；如果条件不满足，再次解锁互斥元，并恢复等待（既虚假唤醒）。因为在等待期间需要加锁解锁操作，所以需要选择更加灵活的`unique_lock`。

### 1.4. Future相关组件

#### 1.4.1 future

标准库提供了一些工具来获取异步任务（即在单独的线程中启动的函数）的返回值，并捕捉其所抛出的异常。这些值在共享状态中传递，其中异步任务可以写入其返回值或存储异常，而且可以由持有该引用该共享态的 `std::future `或 `std::shared_future `实例的线程检验、等待或是操作这个状态。

类模板 `std::future` 提供访问异步操作结果的机制，不支持拷贝，支持移动。主要有两个常用方法：

- get：等待直至future拥有合法结果并获取它，等效于调用wait等待结果。调用get后将释放共享状态，valid()返回false。
- wait/wait_for/wait_until：阻塞直至结果变得可用。若调用此函数前 `valid() == false` ，则行为未定义。

```C++
int main()
{
    // 来自 packaged_task 的 future
    std::packaged_task<int()> task([](){ return 7; }); // 包装函数
    std::future<int> f1 = task.get_future();  // 获取 future
    std::thread(std::move(task)).detach(); // 在线程上运行
 
    // 来自 async() 的 future
    std::future<int> f2 = std::async(std::launch::async, [](){ return 8; });
 
    // 来自 promise 的 future
    std::promise<int> p;
    std::future<int> f3 = p.get_future();
    std::thread( [&p]{ p.set_value_at_thread_exit(9); }).detach();
 
    std::cout << "Waiting..." << std::flush;
    f1.wait();
    f2.wait();
    f3.wait();
    std::cout << "Done!\nResults are: "
              << f1.get() << ' ' << f2.get() << ' ' << f3.get() << '\n';
}
```

#### 1.4.2 async

函数模板 `async` 异步地运行函数 `f` （潜在地在可能是线程池一部分的分离线程中），并返回最终将保有该函数调用结果的` std::future`。

asnyc的第一个参数，可以传入`std::launch::async`，表示异步求值，即调用函数后就运行线程开始执行任务；也可以传入`std::launch::deferred`表示惰性求值，即调用方线程上首次请求其结果时才开始执行任务。

#### 1.4.3 packaged_task

std::packaged_task 包装一个可调用的对象，并且允许异步获取该可调用对象产生的结果，从包装可调用对象意义上来讲，std::packaged_task 与 std::function 类似，只不过 std::packaged_task 将其包装的可调用对象的执行结果传递给一个 std::future 对象（该对象通常在另外一个线程中获取 std::packaged_task 任务的执行结果）。

std::packaged_task 对象内部包含了两个最基本元素：1）被包装的任务(stored task)，任务(task)是一个可调用的对象，如函数指针、成员函数指针或者函数对象；2）共享状态(shared state)，用于保存任务的返回值，可以通过 std::future 对象来达到异步访问共享状态的效果。可以通过 std::packged_task::get_future 来获取与共享状态相关联的 std::future 对象。在调用该函数之后，两个对象共享相同的共享状态，具体解释如下：

- std::packaged_task 对象是异步 Provider，它在某一时刻通过调用被包装的任务来设置共享状态的值。
- std::future 对象是一个异步返回对象，通过它可以获得共享状态的值，当然在必要的时候需要等待共享状态标志变为 ready.

std::packaged_task 的共享状态的生命周期一直持续到最后一个与之相关联的对象被释放或者销毁为止。下面一个小例子大致讲了 std::packaged_task 的用法：

```C++
// count down taking a second for each value:
int countdown (int from, int to) {
    for (int i=from; i!=to; --i) {
        std::cout << i << '\n';
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Finished!\n";
    return from - to;
}

int main ()
{
    std::packaged_task<int(int,int)> task(countdown); // 设置 packaged_task
    std::future<int> ret = task.get_future(); // 获得与 packaged_task 共享状态相关联的 future 对象.

    std::thread th(std::move(task), 10, 0);   //创建一个新线程完成计数任务.

    int value = ret.get();                    // 等待任务完成并获取结果.

    std::cout << "The countdown lasted for " << value << " seconds.\n";

    th.join();
    return 0;
}
```

`packaged_task`常用方法：

- valid()：检查对象是否拥有共享状态。拥有则返回true，否则为false。
- get_future()：返回与对象共享同一共享状态的future。
- operator()：执行函数。
- reset()：重置状态，抛弃先前执行的结果。

```C++
int main()
{
    std::packaged_task<int(int,int)> task([](int a, int b) {
        return std::pow(a, b);
    });
    std::future<int> result = task.get_future();
    task(2, 9);
    std::cout << "2^9 = " << result.get() << '\n';
 
    task.reset();
    result = task.get_future();
    std::thread task_td(std::move(task), 2, 10);
    task_td.join();
    std::cout << "2^10 = " << result.get() << '\n';
}
```

#### 1.4.4 promise

promise 对象可以保存某一类型 T 的值，该值可被 future 对象读取（可能在另外一个线程中），因此 promise 也提供了一种线程同步的手段。在 promise 对象构造时可以和一个共享状态（通常是std::future）相关联，并可以在相关联的共享状态(std::future)上保存一个类型为 T 的值。注意 `std::promise` 只应当使用一次。

可以通过 get_future 来获取与该 promise 对象相关联的 future 对象，调用该函数之后，两个对象共享相同的共享状态(shared state)

- promise 对象是异步 Provider，它可以在某一时刻设置共享状态的值。
- future 对象可以异步返回共享状态的值，或者在必要的情况下阻塞调用者并等待共享状态标志变为 ready，然后才能获取共享状态的值。

```C++
void accumulate(std::vector<int>::iterator first,
                std::vector<int>::iterator last,
                std::promise<int> accumulate_promise)
{
    int sum = std::accumulate(first, last, 0);
    accumulate_promise.set_value(sum);  // 提醒 future
}
 
void do_work(std::promise<void> barrier)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    barrier.set_value();
}
 
int main()
{
    // 演示用 promise<int> 在线程间传递结果。
    std::vector<int> numbers = { 1, 2, 3, 4, 5, 6 };
    std::promise<int> accumulate_promise;
    std::future<int> accumulate_future = accumulate_promise.get_future();
    std::thread work_thread(accumulate, numbers.begin(), numbers.end(),
                            std::move(accumulate_promise));
 
    // future::get() 将等待直至该 future 拥有合法结果并取得它
    // 无需在 get() 前调用 wait()
    //accumulate_future.wait();  // 等待结果
    std::cout << "result=" << accumulate_future.get() << '\n';
    work_thread.join();  // wait for thread completion
 
    // 演示用 promise<void> 在线程间对状态发信号
    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread new_work_thread(do_work, std::move(barrier));
    barrier_future.wait();
    new_work_thread.join();
}
```



### 1.5 原子操作与原子类型





## 2. 线程安全数据结构

### 2.1 线程安全队列



### 2.2 线程安全栈



### 2.3 线程池



## 3. 并发代码设计

