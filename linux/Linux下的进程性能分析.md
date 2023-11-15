## Linux下的进程性能分析

机器的性能受到以下几个方面的制约：CPU、内存、磁盘IO、网络IO。这也是我们分析程序性能的几个方面。

### 1. 硬件参数查看

#### **查看CPU参数**

使用`lscpu`或`cat /proc/cpuinfo`可以获取当前系统的CPU信息。

```shell
# 灵汐芯片的板载ARM系统的CPU信息
[root@lynxi ~]$ cat /proc/cpuinfo
processor       : 0      #CPU的核心编号
BogoMIPS        : 62.50  #CPU处理能力的衡量指标
Features        : fp asimd evtstrm crc32 cpuid
CPU implementer : 0x41   #CPU的制造商。0x41表示ARM
CPU architecture: 8      #CPU的架构版本，ARMv8架构
CPU variant     : 0x0
CPU part        : 0xd03  #CPU的型号或系列，ARM Cortex-A53
CPU revision    : 4

processor       : 1
BogoMIPS        : 62.50
Features        : fp asimd evtstrm crc32 cpuid
CPU implementer : 0x41
CPU architecture: 8
CPU variant     : 0x0
CPU part        : 0xd03
CPU revision    : 4

...
```

> ARM Cortex-A53 是一种基于 ARMv8 架构的 CPU 核心。以下是常见的 ARM Cortex-A53 CPU 的参数：
>
> - 指令集架构（Instruction Set Architecture，ISA）：ARMv8-A
> - 流水线深度（Pipeline Depth）：15 级
> - 指令长度（Instruction Length）：固定长度 32 位指令
> - 寄存器文件（Register File）：32 个 64 位通用寄存器
> - 缓存层次结构（Cache Hierarchy）：通常具有 L1 指令缓存（32KB），L1 数据缓存（32KB），L2 缓存（通常为共享的128KB或256KB）
> - 支持的 SIMD 扩展：NEON 和 VFPv4 浮点扩展
> - 支持的虚拟化技术：Virtualization Extensions（VHE）
>
> 这些参数提供了 Cortex-A53 CPU 的基本特性和配置信息。请注意，具体的实现可能会因不同的芯片制造商和 SoC (System-on-Chip) 变体而有所差异。

#### 查看CPU各级缓存大小

```shell
lynxi@bcloud-node4:~$ lscpu
Architecture:                       x86_64
CPU op-mode(s):                     32-bit, 64-bit
Byte Order:                         Little Endian
Address sizes:                      46 bits physical, 48 bits virtual
CPU(s):                             64
On-line CPU(s) list:                0-63
Thread(s) per core:                 2
Core(s) per socket:                 16
Socket(s):                          2
NUMA node(s):                       2
Vendor ID:                          GenuineIntel
CPU family:                         6
Model:                              85
Model name:                         Intel(R) Xeon(R) Silver 4216 CPU @ 2.10GHz
Stepping:                           7
CPU MHz:                            800.240
CPU max MHz:                        3200.0000
CPU min MHz:                        800.0000
BogoMIPS:                           4200.00
Virtualization:                     VT-x
L1d cache:                          1 MiB
L1i cache:                          1 MiB
L2 cache:                           32 MiB
L3 cache:                           44 MiB
......
```

#### 查看系统内存大小

```shell
[root@lynxi ~]$ cat /proc/meminfo
MemTotal:        2956268 kB
MemFree:         2581260 kB
MemAvailable:    2510412 kB
Buffers:               0 kB
Cached:           299040 kB
SwapCached:            0 kB
......
```

### 2. 性能分析辅助工具

#### uptime：查看系统的平均负载

uptime用于查看系统的运行时间和负载情况，它记录了1分钟、5分钟和15分钟内系统的平均负载。

```sh
10:26:16 up 1 day, 16:12, 13 users,  load average: 0.87, 1.18, 1.31
```

平均负载是指系统上正在执行或等待执行的进程数。通常，低于 CPU 核心数的负载值被认为是正常的。高于 CPU 核心数的负载值可能表示系统负荷较大，需要更多资源来处理任务。



#### 查看系统上下文切换

**pidstat**：使用`pidstat -w`选项查看具体进程的上下文切换次数：

```
$ pidstat -w -p 321728 1
110:19:13      UID       PID   cswch/s nvcswch/s  Command
10:19:14        0   3217281      0.00     18.00  stress
10:19:15        0   3217281      0.00     18.00  stress
10:19:16        0   3217281      0.00     28.71  stress
```

其中`cswch/s`和`nvcswch/s`表示自愿上下文切换和非自愿上下文切换。

**自愿上下文切换**：是指进程无法获取所需资源，导致的上下文切换。比如说， I/O、内存等系统资源不足时，就会发生自愿上下文切换。

**非自愿上下文切换**：则是指进程由于时间片已到等原因，被系统强制调度，进而发生的上下文切换。比如说，大量进程都在争抢 CPU 时，就容易发生非自愿上下文切换

1. 自愿上下文切换变多了，说明进程都在等待资源，有可能发生了 I/O 等其他问题。
2. 非自愿上下文切换变多了，说明进程都在被强制调度，也就是都在争抢 CPU，说明 CPU 的确成了瓶颈。

### 压测工具

##### stress 命令使用

```sh
# --cpu 8：8个进程不停的执行sqrt()计算操作
# --io 4：4个进程不同的执行sync()io操作（刷盘）
# --vm 2：2个进程不停的执行malloc()内存申请操作
# --vm-bytes 128M：限制1个执行malloc的进程申请内存大小
stress --cpu 8 --io 4 --vm 2 --vm-bytes 128M --timeout 10s
```











