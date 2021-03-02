# 并行计算——CUDA编程

正常的串行CPU被设计为优化延迟，而GPU被设计为优化吞吐量。CPU被设计为在最短时间内执行所有指令，而GPU被设计为在给定时间内执行更多指令。GPU的这种设计理念使它们在图像处理和计算机视觉应用中非常有用，这也是本书的目的，因为我们不介意单个像素处理的延迟。我们想要的是在给定的时间内处理更多的像素，这可以在GPU上完成。

CUDA C程序的开发步骤如下：1）为主机和设备显存中的数据分配内存。2）将数据从主机内存复制到设备显存。3）通过指定并行度来启动内核。4）所有线程完成后，将数据从设备显存复制回主机内存。5）释放主机和设备上使用的所有内存。

## CUDA编程模型基础

在给出CUDA的编程实例之前，这里先对CUDA编程模型中的一些概念及基础知识做个简单介绍。CUDA编程模型是一个异构模型，需要CPU和GPU协同工作。在CUDA中，**host**和**device**是两个重要的概念，我们用host指代CPU及其内存，而用device指代GPU及其内存。CUDA程序中既包含host程序，又包含device程序，它们分别在CPU和GPU上运行。同时，host与device之间可以进行通信，这样它们之间可以进行数据拷贝。典型的CUDA程序的执行流程如下：

1. 为主机和设备显存中的数据分配内存。
2. 将数据从主机内存复制到设备显存。
3. 调用CUDA的核函数在device上完成指定的运算。
4. 所有线程完成后，将数据从设备显存复制回主机内存。
5. 释放主机和设备上使用的所有内存。

上面流程中最重要的一个过程是调用CUDA的核函数来执行并行计算，kernel是CUDA中一个重要的概念，kernel是在device上线程中并行执行的函数，核函数用`__global__`符号声明，在调用时需要用`<<<grid, block>>>`来指定kernel要执行的线程数量，在CUDA中，每一个线程都要执行核函数，并且每个线程会分配一个唯一的线程号thread ID，这个ID值可以通过核函数的内置变量`threadIdx`来获得。

由于GPU实际上是异构模型，所以需要区分host和device上的代码，在CUDA中是通过函数类型限定词开区别host和device上的函数，主要的三个函数类型限定词如下：

- `__global__`：在device上执行，从host中调用（一些特定的GPU也可以从device上调用），返回类型必须是`void`，不支持可变参数参数，不能成为类成员函数。注意用`__global__`定义的kernel是异步的，这意味着host不会等待kernel执行完就执行下一步。
- `__device__`：在device上执行，单仅可以从device中调用，不可以和`__global__`同时用。
- `__host__`：在host上执行，仅可以从host上调用，一般省略不写，不可以和`__global__`同时用，但可和`__device__`，此时函数会在device和host都编译。

要深刻理解kernel，必须要对kernel的线程层次结构有一个清晰的认识。首先GPU上很多并行化的轻量级线程。kernel在device上执行时实际上是启动很多线程，一个kernel所启动的所有线程称为一个**网格**（grid），同一个网格上的线程共享相同的全局内存空间，grid是线程结构的第一层次，而网格又可以分为很多**线程块**（block），一个线程块里面包含很多线程，这是第二个层次。线程两层组织结构如下图所示，这是一个gird和block均为2-dim的线程组织。grid和block都是定义为`dim3`类型的变量，`dim3`可以看成是包含三个无符号整数（x，y，z）成员的结构体变量，在定义时，缺省值初始化为1。因此grid和block可以灵活地定义为1-dim，2-dim以及3-dim结构，对于图中结构（主要水平方向为x轴），定义的grid和block如下所示，kernel在调用时也必须通过执行配置`<<<grid, block>>>`来指定kernel所使用的线程数及结构。

```c
dim3 grid(3, 2);
dim3 block(5, 3);
kernel_fun<<< grid, block >>>(prams...);
```

![](https://cdn.jsdelivr.net/gh/charlotteLive/PicGoPiture/markdown/20210227231141.png)

所以，一个线程需要两个内置的坐标变量（blockIdx，threadIdx）来唯一标识，它们都是`dim3`类型变量，其中blockIdx指明线程所在grid中的位置，而threaIdx指明线程所在block中的位置，如图中的Thread (1,1)满足：

```text
threadIdx.x = 1
threadIdx.y = 1
blockIdx.x = 1
blockIdx.y = 1
```

一个线程块上的线程是放在同一个流式多处理器（SM)上的，但是单个SM的资源有限，这导致线程块中的线程数是有限制的，现代GPUs的线程块可支持的线程数可达1024个。有时候，我们要知道一个线程在blcok中的全局ID，此时就必须还要知道block的组织结构，这是通过线程的内置变量blockDim来获得。它获取线程块各个维度的大小。对于一个2-dim的block ![[公式]](https://www.zhihu.com/equation?tex=(D_x%2C+D_y)) ，线程 ![[公式]](https://www.zhihu.com/equation?tex=(x%2C+y)) 的ID值为 ![[公式]](https://www.zhihu.com/equation?tex=%28x+%2B+y+%2A+D_x%29) ，如果是3-dim的block ![[公式]](https://www.zhihu.com/equation?tex=%28D_x%2C+D_y%2C+D_z%29) ，线程 ![[公式]](https://www.zhihu.com/equation?tex=%28x%2C+y%2C+z%29) 的ID值为 ![[公式]](https://www.zhihu.com/equation?tex=%28x+%2B+y+%2A+D_x+%2B+z+%2A+D_x+%2A+D_y%29) 。另外线程还有内置变量gridDim，用于获得网格块各个维度的大小。

kernel的这种线程组织结构天然适合vector,matrix等运算，如我们将利用上图2-dim结构实现两个矩阵的加法，每个线程负责处理每个位置的两个元素相加，代码如下所示。线程块大小为(16, 16)，然后将N*N大小的矩阵均分为不同的线程块来执行加法运算。

```c
// Kernel定义
__global__ void MatAdd(float A[N][N], float B[N][N], float C[N][N]) 
{ 
    int i = blockIdx.x * blockDim.x + threadIdx.x; 
    int j = blockIdx.y * blockDim.y + threadIdx.y; 
    if (i < N && j < N) 
        C[i][j] = A[i][j] + B[i][j]; 
}
int main() 
{ 
    ...
    // Kernel 线程配置
    dim3 threadsPerBlock(16, 16); 
    dim3 numBlocks(N / threadsPerBlock.x, N / threadsPerBlock.y);
    // kernel调用
    MatAdd<<<numBlocks, threadsPerBlock>>>(A, B, C); 
    ...
}
```



此外这里简单介绍一下**CUDA的内存模型**，如下图所示。可以看到，每个线程有自己的私有本地内存（Local Memory），而每个线程块有包含共享内存（Shared Memory）,可以被线程块中所有线程共享，其生命周期与线程块一致。此外，所有的线程都可以访问全局内存（Global Memory）。还可以访问一些只读内存块：常量内存（Constant Memory）和纹理内存（Texture Memory）。内存结构涉及到程序优化，这里不深入探讨它们。

![CUDA内存模型](https://cdn.jsdelivr.net/gh/charlotteLive/PicGoPiture/markdown/20210227233029.png)

还有重要一点，你需要对GPU的硬件实现有一个基本的认识。上面说到了kernel的线程组织层次，那么一个kernel实际上会启动很多线程，这些线程是逻辑上并行的，但是在物理层却并不一定。这其实和CPU的多线程有类似之处，多线程如果没有多核支持，在物理层也是无法实现并行的。但是好在GPU存在很多CUDA核心，充分利用CUDA核心可以充分发挥GPU的并行计算能力。GPU硬件的一个核心组件是SM，前面已经说过，SM是英文名是 Streaming Multiprocessor，翻译过来就是流式多处理器。SM的核心组件包括CUDA核心，共享内存，寄存器等，SM可以并发地执行数百个线程，并发能力就取决于SM所拥有的资源数。当一个kernel被执行时，它的gird中的线程块被分配到SM上，一个线程块只能在一个SM上被调度。SM一般可以调度多个线程块，这要看SM本身的能力。那么有可能一个kernel的各个线程块被分配多个SM，所以grid只是逻辑层，而SM才是执行的物理层。SM采用的是[SIMT](https://link.zhihu.com/?target=http%3A//docs.nvidia.com/cuda/cuda-c-programming-guide/index.html%23simt-architecture) (Single-Instruction, Multiple-Thread，单指令多线程)架构，基本的执行单元是线程束（wraps)，线程束包含32个线程，这些线程同时执行相同的指令，但是每个线程都包含自己的指令地址计数器和寄存器状态，也有自己独立的执行路径。所以尽管线程束中的线程同时从同一程序地址执行，但是可能具有不同的行为，比如遇到了分支结构，一些线程可能进入这个分支，但是另外一些有可能不执行，它们只能死等，因为GPU规定线程束中所有线程在同一周期执行相同的指令，线程束分化会导致性能下降。当线程块被划分到某个SM上时，它将进一步划分为多个线程束，因为这才是SM的基本执行单元，但是一个SM同时并发的线程束数是有限的。这是因为资源限制，SM要为每个线程块分配共享内存，而也要为每个线程束中的线程分配独立的寄存器。所以SM的配置会影响其所支持的线程块和线程束并发数量。总之，就是网格和线程块只是逻辑划分，一个kernel的所有线程其实在物理层是不一定同时并发的。所以kernel的grid和block的配置不同，性能会出现差异，这点是要特别注意的。还有，由于SM的基本执行单元是包含32个线程的线程束，所以block大小一般要设置为32的倍数。

![](https://cdn.jsdelivr.net/gh/charlotteLive/PicGoPiture/markdown/CUDA编程的逻辑层和物理层.png)

在进行CUDA编程前，可以先检查一下自己的GPU的硬件配置，这样才可以有的放矢，可以通过下面的程序获得GPU的配置属性：

```C++
int dev = 0;
    cudaDeviceProp devProp;
    cudaGetDeviceProperties(&devProp, dev);
    std::cout << "使用GPU device " << dev << ": " << devProp.name << std::endl;
    std::cout << "SM的数量：" << devProp.multiProcessorCount << std::endl;
    std::cout << "每个线程块的共享内存大小：" << devProp.sharedMemPerBlock / 1024.0 << " KB" << std::endl;
    std::cout << "每个线程块的最大线程数：" << devProp.maxThreadsPerBlock << std::endl;
    std::cout << "每个EM的最大线程数：" << devProp.maxThreadsPerMultiProcessor << std::endl;
    std::cout << "每个EM的最大线程束数：" << devProp.maxThreadsPerMultiProcessor / 32 << std::endl;

//输出如下：
使用GPU device 0: GeForce GTX 750
SM的数量：4
每个线程块的共享内存大小：48 KB
每个线程块的最大线程数：1024
每个EM的最大线程数：2048
每个EM的最大线程束数：64
```

## 向量加法实例

知道了CUDA编程基础，我们就来个简单的实战，利用CUDA编程实现两个向量的加法，在实现之前，先简单介绍一下CUDA编程中内存管理API。首先是在device上分配内存的cudaMalloc函数：

```c
cudaError_t cudaMalloc(void** devPtr, size_t size);
```

这个函数和C语言中的malloc类似，但是在device上申请一定字节大小的显存，其中devPtr是指向所分配内存的指针。同时要释放分配的内存使用cudaFree函数，这和C语言中的free函数对应。另外一个重要的函数是负责host和device之间数据通信的cudaMemcpy函数：

```c
cudaError_t cudaMemcpy(void* dst, const void* src, size_t count, cudaMemcpyKind kind)
```

其中src指向数据源，而dst是目标区域，count是复制的字节数，其中kind控制复制的方向：cudaMemcpyHostToHost, cudaMemcpyHostToDevice, cudaMemcpyDeviceToHost及cudaMemcpyDeviceToDevice，如cudaMemcpyHostToDevice将host上数据拷贝到device上。

现在我们来实现一个向量加法的实例，这里grid和block都设计为1-dim，首先定义kernel如下：

```c
// 两个向量加法kernel，grid和block均为一维
__global__ void add(float* x, float * y, float* z, int n)
{
    // 获取全局索引
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    // 步长
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < n; i += stride)
    {
        z[i] = x[i] + y[i];
    }
}
```

其中stride是整个grid的线程数，有时候向量的元素数很多，这时候可以将在每个线程实现多个元素（元素总数/线程总数）的加法，相当于使用了多个grid来处理，这是一种[grid-stride loop](https://link.zhihu.com/?target=https%3A//devblogs.nvidia.com/cuda-pro-tip-write-flexible-kernels-grid-stride-loops/)方式，不过下面的例子一个线程只处理一个元素，所以kernel里面的循环是不执行的。下面我们具体实现向量加法：

```c
int main()
{
    int N = 1 << 20;
    int nBytes = N * sizeof(float);
    // 申请host内存
    float *x, *y, *z;
    x = (float*)malloc(nBytes);
    y = (float*)malloc(nBytes);
    z = (float*)malloc(nBytes);

    // 初始化数据
    for (int i = 0; i < N; ++i)
    {
        x[i] = 10.0;
        y[i] = 20.0;
    }

    // 申请device内存
    float *d_x, *d_y, *d_z;
    cudaMalloc((void**)&d_x, nBytes);
    cudaMalloc((void**)&d_y, nBytes);
    cudaMalloc((void**)&d_z, nBytes);

    // 将host数据拷贝到device
    cudaMemcpy((void*)d_x, (void*)x, nBytes, cudaMemcpyHostToDevice);
    cudaMemcpy((void*)d_y, (void*)y, nBytes, cudaMemcpyHostToDevice);
    // 定义kernel的执行配置
    dim3 blockSize(256);
    dim3 gridSize((N + blockSize.x - 1) / blockSize.x);
    // 执行kernel
    add <<< gridSize, blockSize >>>(d_x, d_y, d_z, N);

    // 将device得到的结果拷贝到host
    cudaMemcpy((void*)z, (void*)d_z, nBytes, cudaMemcpyHostToDevice);

    // 检查执行结果
    float maxError = 0.0;
    for (int i = 0; i < N; i++)
        maxError = fmax(maxError, fabs(z[i] - 30.0));
    std::cout << "最大误差: " << maxError << std::endl;

    // 释放device内存
    cudaFree(d_x);
    cudaFree(d_y);
    cudaFree(d_z);
    // 释放host内存
    free(x);
    free(y);
    free(z);

    return 0;
}
```

这里我们的向量大小为1<<20，而block大小为256，那么grid大小是4096，kernel的线程层级结构如下图所示：

![](https://cdn.jsdelivr.net/gh/charlotteLive/PicGoPiture/markdown/20210227235134.png)

使用nvprof工具可以分析kernel运行情况，结果如下所示，可以看到kernel函数费时约1.5ms。

```text
nvprof cuda9.exe
==7244== NVPROF is profiling process 7244, command: cuda9.exe
最大误差: 4.31602e+008
==7244== Profiling application: cuda9.exe
==7244== Profiling result:
            Type  Time(%)      Time     Calls       Avg       Min       Max  Name
 GPU activities:   67.57%  3.2256ms         2  1.6128ms  1.6017ms  1.6239ms  [CUDA memcpy HtoD]
                   32.43%  1.5478ms         1  1.5478ms  1.5478ms  1.5478ms  add(float*, float*, float*, int)
```

你调整block的大小，对比不同配置下的kernel运行情况，我这里测试的是当block为128时，kernel费时约1.6ms，而block为512时kernel费时约1.7ms，当block为64时，kernel费时约2.3ms。看来不是block越大越好，而要适当选择。

在上面的实现中，我们需要单独在host和device上进行内存分配，并且要进行数据拷贝，这是很容易出错的。好在CUDA 6.0引入统一内存（[Unified Memory](https://link.zhihu.com/?target=http%3A//docs.nvidia.com/cuda/cuda-c-programming-guide/index.html%23um-unified-memory-programming-hd)）来避免这种麻烦，简单来说就是统一内存使用一个托管内存来共同管理host和device中的内存，并且自动在host和device中进行数据传输。CUDA中使用cudaMallocManaged函数分配托管内存：

```c
cudaError_t cudaMallocManaged(void **devPtr, size_t size, unsigned int flag=0);
```

利用统一内存，可以将上面的程序简化如下：

```c
int main()
{
    int N = 1 << 20;
    int nBytes = N * sizeof(float);

    // 申请托管内存
    float *x, *y, *z;
    cudaMallocManaged((void**)&x, nBytes);
    cudaMallocManaged((void**)&y, nBytes);
    cudaMallocManaged((void**)&z, nBytes);

    // 初始化数据
    for (int i = 0; i < N; ++i)
    {
        x[i] = 10.0;
        y[i] = 20.0;
    }

    // 定义kernel的执行配置
    dim3 blockSize(256);
    dim3 gridSize((N + blockSize.x - 1) / blockSize.x);
    // 执行kernel
    add << < gridSize, blockSize >> >(x, y, z, N);

    // 同步device 保证结果能正确访问
    cudaDeviceSynchronize();
    // 检查执行结果
    float maxError = 0.0;
    for (int i = 0; i < N; i++)
        maxError = fmax(maxError, fabs(z[i] - 30.0));
    std::cout << "最大误差: " << maxError << std::endl;

    // 释放内存
    cudaFree(x);
    cudaFree(y);
    cudaFree(z);

    return 0;
}
```

相比之前的代码，使用统一内存更简洁了，值得注意的是kernel执行是与host异步的，由于托管内存自动进行数据传输，这里要用cudaDeviceSynchronize()函数保证device和host同步，这样后面才可以正确访问kernel计算的结果。

## 存储器架构

GPU有几个不同的存储器空间，每个存储器空间都有特定的特征和用途以及不同的速度和范围。这个存储空间按层次结构划分为不同的组块，比如全局内存、共享内存、本地内存、常量内存和纹理内存，每个组块都可以从程序中的不同点访问。此存储器架构如图所示。

![](https://cdn.jsdelivr.net/gh/charlotteLive/PicGoPiture/markdown/20210228102128.png)

每个线程都有自己的本地存储器和寄存器堆。与处理器不同的是，GPU核心有很多寄存器来存储本地数据。当线程使用的数据不适合存储在寄存器堆中或者寄存器堆中装不下的时候，将会使用本地内存。寄存器堆和本地内存对每个线程都是唯一的。寄存器堆是最快的一种存储器。同一个块中的线程具有可由该块中的所有线程访问的共享内存。全局内存可被所有的块和其中的所有线程访问。它具有相当大的访问延迟，但存在缓存这种东西来给它提速。如下表，GPU有一级和二级缓存（即L1缓存和L2缓存）。常量内存则是用于存储常量和内核参数之类的只读数据。最后，存在纹理内存，这种内存可以利用各种2D和3D的访问模式。

所有存储器特征总结如下。

![image-20210228102315911](https://cdn.jsdelivr.net/gh/charlotteLive/PicGoPiture/markdown/image-20210228102315911.png)

上表表述了各种存储器的各种特性。作用范围栏定义了程序的哪个部分能使用该存储器。而生存期定义了该存储器中的数据对程序可见的时间。除此之外，L1和L2缓存也可以用于GPU程序以便更快地访问存储器。总之，所有线程都有一个寄存器堆，它是最快的。共享内存只能被块中的线程访问，但比全局内存块。全局内存是最慢的，但可以被所有的块访问。常量和纹理内存用于特殊用途。存储器访问是程序快速执行的最大瓶颈。

### 全局内存

所有的块都可以对全局内存进行读写。该存储器较慢，但是可以从你的代码的任何地方进行读写。缓存可加速对全局内存的访问。所有通过cudaMalloc分配的存储器都是全局内存。

```C++
#include <stdio.h>
#define N 5

__global__ void gpu_global_memory(int *d_a)
{
	d_a[threadIdx.x] = threadIdx.x;
}
int main(int argc, char **argv)
{
	int h_a[N];
	int *d_a; 
						
	cudaMalloc((void **)&d_a, sizeof(int) *N);
	cudaMemcpy((void *)d_a, (void *)h_a, sizeof(int) *N, cudaMemcpyHostToDevice);
	gpu_global_memory << <1, N >> >(d_a);  
	cudaMemcpy((void *)h_a, (void *)d_a, sizeof(int) *N, cudaMemcpyDeviceToHost);
	
    printf("Array in Global Memory is: \n");
	for (int i = 0; i < N; i++) {
		printf("At Index: %d --> %d \n", i, h_a[i]);
	}
    
	return 0;
}
```

这段代码演示了如何从设备代码中进行全局内存的写入，以及如何从主机代码中用cudaMalloc进行分配，如何将指向该段全局内存的指针作为参数传递给内核函数。内核函数用不同的线程ID的值来填充这段全局内存。然后（用cudaMemcpy）复制到内存以便显示内容。

### 本地内存和寄存器堆

本地内存和寄存器堆对每个线程都是唯一的。寄存器是每个线程可用的最快存储器。当内核中使用的变量在寄存器堆中装不下的时候，将会使用本地内存存储它们，这叫寄存器溢出。请注意使用本地内存有两种情况：一种是寄存器不够了，一种是某些情况根本就不能放在寄存器中，例如对一个局部数组的下标进行不定索引的时候。基本上可以将本地内存看成是每个线程的唯一的全局内存部分。相比寄存器堆，本地内存要慢很多。虽然本地内存通过L1缓存和L2缓存进行了缓冲，但寄存器溢出可能会影响你的程序的性能。

```C++
#include <stdio.h>
#define N 5

__global__ void gpu_local_memory(int d_in)
{
	int t_local;    
	t_local = d_in * threadIdx.x;     
	printf("Value of Local variable in current thread is: %d \n", t_local);
}

int main(int argc, char **argv)
{
	printf("Use of Local Memory on GPU:\n");
	gpu_local_memory << <1, N >> >(5);  
	cudaDeviceSynchronize();
	return 0;
}
```

代码中的t_local变量是每个线程局部唯一的，将被存储在寄存器堆中。用这种变量计算的时候，计算速度将是最快速的。

在较新的GPU上，每个流多处理器都含有自己独立的L1缓存，以及GPU有L2缓存。L2缓存是被所有的GPU中的流多处理器都共有的。所有的全局内存访问和本地内存访问都使用这些缓存，因为L1缓存在流多处理器内部独有，接近线程执行所需要的硬件单位，所以它的速度非常快。一般来说，L1缓存和共享内存共用同样的存储硬件，一共是64KB，你可以配置L1缓存和共享内存分别在这64KB中的比例。所有的全局内存访问通过L2缓存进行。纹理内存和常量内存也分别有它们独立的缓存。

### 共享内存与线程同步

共享内存位于芯片内部，因此它比全局内存快得多（延迟低）。相比没有经过缓存的全局内存访问，共享内存大约在延迟上低100倍。同一个块中的线程可以访问相同的一段共享内存，这在许多线程需要与其他线程共享它们的结果的应用程序中非常有用。使用共享内存时，需要调用`__syncthreads()`指令确保在继续执行程序之前完成对内存的所有写入操作。这也被称为barrier。barrier的含义是块中的所有线程都将到达该代码行，然后在此等待其他线程完成。当所有线程都到达了这里之后，它们可以一起继续往下执行。

下面的例子用于计算数组中当前元素之前所有元素的平均值，很多线程计算的时候将会使用数组中的同样的数据。这就是一种理想的使用共享内存的用例，这样将会得到比全局内存更快的数据访问。这将减少每个线程的全局内存访问次数，从而减少程序的延迟。共享内存上的数字或者变量是通过__shared__修饰符定义的。通常，共享内存的大小应该等于每个块的线程数。

```C++
__global__ void gpu_shared_memory(float *d_a)
{
	// Defining local variables which are private to each thread
	int i, index = threadIdx.x;
	float average, sum = 0.0f;
	//Define shared memory
	__shared__ float sh_arr[10];
	sh_arr[index] = d_a[index];
    // This ensures all the writes to shared memory have completed
	__syncthreads();

	for (i = 0; i<= index; i++) 
	{ 
		sum += sh_arr[i]; 
	}
	average = sum / (index + 1.0f);
	d_a[index] = average; 
}
```

### 常量内存

CUDA程序员会经常用到另外一种存储器——常量内存，NVIDIA GPU卡从逻辑上对用户提供了64KB的常量内存空间，可以用来存储内核执行期间所需要的恒定数据。常量内存对一些特定情况下的小数据量的访问具有相比全局内存的额外优势。使用常量内存也一定程度上减少了对全局内存的带宽占用。

```C++
#include "stdio.h"
#include<iostream>
#include <cuda.h>
#include <cuda_runtime.h>
//Defining two constants
__constant__ int constant_f;
__constant__ int constant_g;
#define N	5
//Kernel function for using constant memory
__global__ void gpu_constant_memory(float *d_in, float *d_out) {
	//Thread index for current kernel
	int tid = threadIdx.x;	
	d_out[tid] = constant_f*d_in[tid] + constant_g;
}

int main(void) {
	//Defining Arrays for host
	float h_in[N], h_out[N];
	//Defining Pointers for device
	float *d_in, *d_out;
	int h_f = 2;
	int h_g = 20;
	// allocate the memory on the cpu
	cudaMalloc((void**)&d_in, N * sizeof(float));
	cudaMalloc((void**)&d_out, N * sizeof(float));
	//Initializing Array
	for (int i = 0; i < N; i++) {
		h_in[i] = i;
	}
	//Copy Array from host to device
	cudaMemcpy(d_in, h_in, N * sizeof(float), cudaMemcpyHostToDevice);
	//Copy constants to constant memory
	cudaMemcpyToSymbol(constant_f, &h_f, sizeof(int),0,cudaMemcpyHostToDevice);
	cudaMemcpyToSymbol(constant_g, &h_g, sizeof(int));

	//Calling kernel with one block and N threads per block
	gpu_constant_memory << <1, N >> >(d_in, d_out);
	//Coping result back to host from device memory
	cudaMemcpy(h_out, d_out, N * sizeof(float), cudaMemcpyDeviceToHost);
	//Printing result on console
	printf("Use of Constant memory on GPU \n");
	for (int i = 0; i < N; i++) {
		printf("The expression for input %f is %f\n", h_in[i], h_out[i]);
	}
	//Free up memory
	cudaFree(d_in);
	cudaFree(d_out);
	return 0;
}
```

常量内存中的变量使用`__constant__`关键字修饰。在之前的代码中，两个浮点数constant_f，constant_g被定义成在内核执行期间不会改变的常量。需要注意的第二点是，使用`__constant__`（在内核外面）定义好了它们后，它们不应该再次在内核内部定义。内核函数将用这两个常量进行一个简单的数学运算，在main函数中，我们用cudaMemcpyToSymbol函数把这些常量复制到内核执行所需要的常量内存中。

### 纹理内存

纹理内存是另外一种当数据的访问具有特定的模式的时候能够加速程序执行，并减少显存带宽的只读存储器。像常量内存一样，它也在芯片内部被cache缓冲。该存储器最初是为了图形绘制而设计的，但也可以被用于通用计算。当程序进行具有很大程度上的空间邻近性的访存的时候，这种存储器变得非常高效。空间邻近性的意思是，每个线程的读取位置都和其他线程的读取位置邻近。这对那些需要处理4个邻近的相关点或者8个邻近的点的图像处理应用非常有用。

通用的全局内存的cache将不能有效处理这种空间邻近性，可能会导致进行大量的显存读取传输。纹理存储被设计成能够利用这种访存模型，这样它只会从显存读取1次，然后缓冲掉，所以执行速度将会快得多。纹理内存支持2D和3D的纹理读取操作，在你的CUDA程序里面使用纹理内存可没有那么轻易，特别是对那些并非编程专家的人来说。

