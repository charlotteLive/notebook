## C++内存问题总结

### 1. 常见的内存错误及其对策：

1. **内存分配未完成或着失败，却使用了它。**
    ——使用内存前检查指针是否为NULL，断言或者防错处理。
2. **内存分配成功，但未初始化就使用了。**
    ——养成定义后就初始化的习惯，特别是数组，必要时候可以调用memset清零（主要针对C语言基础类型）。
3. **内存分配成功且初始化，但访问内存越界。**（例如在使用数组时经常发生下标“多 1”或者“少 1”的操作。特别是在 for 循环语句中，循环次数很容易搞错，导致数组操作越界。）
    ——可以在不是性能热点的逻辑里，可以加上边界检测判断。
4. **忘记释放内存，从而造成内存泄漏。**（随着程序的运行，最终会因为内存不足而被系统杀掉。）
    ——动态内存的申请与释放必须配对使用，注意new数组之后因使用detect[]释放；使用智能指针。
5. **使用了已经释放的内存。**
    ——这个情况比较复杂，列举三种场景供参考：
    1）程序中的对象调用关系过于复杂，实在难以搞清楚某个对象究竟是否已经释放了内存，此时应该重新设计数据结构，从根本上解决对象管理的混乱局面。
    2）函数返回了申请与栈内存上的指针或者引用。这时应该重新设计函数。
    3）使用 free 或 delete 释放了内存后，仍然在继续使用。这里释放内存后，最好将指针设为NULL，将错误现场第一时间暴露出来。



### 2. 访问NULL指针错误背后的原理

NULL的定义：Null 是一个特殊指针值（或是一种对象引用）表示这个指针并不指向任何的对象，其值取决于编译器的定义。

C 语言中，NULL 的值是 0，即 NULL == 0 是成立的。我们前面说访问 NULL 指针的行为会产生不可预料的后果。但是在 Linux 系统中后果是确定的：访问空指针会产生 Segmentation fault 的错误。因此这里的“不可预料”指的是在不同系统产生的后果不一样。

让我们假设现在使用的是 C 语言，运行在 Linux 系统上，以此来分析访问 NULL 指针的过程。

Linux 中，每个进程空间的 0x0 虚拟地址开始的线性区(memory region)都会被映射到一个用户态没有访问权限的页上。通过这样的映射，内核可以保证没有别的页会映射到这个区域。

1. 编译器把空指针当做 0 对待，开心地让你去访问空指针。
2. 缺页异常处理程序被调用，因为在 0x0 的页没有在物理内存里面。
3. 缺页异常处理程序发现你没有访问的权限。
4. 内核发送 SIGSEGV 信号给进程，该信号默认是让进程自杀。

可以看到：不需要特定的编译器实现或者内核的支持，只需要让一个页映射到 0x0 的虚拟地址上，就完美的实现了检测空指针的错误。



### 3.内存问题排查手段

1. 对于使用了未初始化内存的问题，由于未初始化内存的值是随机的，问题表现在复现过程中也不唯一。主要手段还是以预防为主，出了问题只能通过现象或者GDB调试找到未初始化的变量。预防的手段有cppcheck，自定义类中添加isinit变量。
2. 内存越界问题也是相当令人头痛的问题，因为是改写了其他地方的内存，问题场景多变不唯一，往往不在第一现场。这里讲讲怎么初步判断是否出现了内存越界了，一般如果是栈内存的数组越界，影响范围可能仅局限于该次函数调用，可以检查栈顶指针、栈尾指针、以及栈变量等是否正常，检查是否有栈内存数组写越界了。对于堆上的内存越界，更加不好排查了，GDB可用的手段是监测被改写内存的地址，找出是谁改写了它；实在分析不出来，只能上内存检查等大杀器了。
3. 使用已释放内存问题，同堆内存越界一样处理。。。。

内存检测工具，如公司在用的Valgrind工具包中的memcheck，GCC自带的AddressSanitizer。



### 4. 使用Valgrind检查内存问题

使用方法：

```
gcc XXX.c -g -o 程序
valgrind --tool=memcheck --leak-check=full ./程序
```

`--tool=memcheck`表示使用Memcheck工具发现内存错误使用情况。`--leak-check=full`表示详细显示内存泄露信息。

常见错误

| 错误                                                         | 中文解释                                   |
| :----------------------------------------------------------- | :----------------------------------------- |
| Use of uninitialised memory                                  | 使用未初始化的内存                         |
| Reading/writing memory after it has been free’d              | 使用已经释放了的内存                       |
| Reading/writing off the end of malloc’d blocks               | 使用超过malloc分配的内存空间               |
| Reading/writing inappropriate areas on the stack             | 对堆栈的非法访问                           |
| Memory leaks – where pointers to malloc’d blocks are lost forever | 申请的空间是否有释放                       |
| Mismatched use of malloc/new/new [] vs free/delete/delete [] | malloc/free/new/delete申请和释放内存的匹配 |
| Overlapping src and dst pointers in memcpy() and related functions | src和dst的重叠                             |

