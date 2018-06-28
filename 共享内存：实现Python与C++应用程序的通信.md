## 共享内存：实现Python与C++应用程序的通信

linux下，每个进程都有自己拥有的内存区域，进程的内存总是私有的。共享内存是从系统的空闲内存池中分配的，希望访问它的每个进程连接它。这个连接过程称为映射。映射后，每个进程都可通过访问自己的内存而访问共享内存区域，从而与其他进程进行通信。如下图：![共享内存](E:\Github\notebook\picture\共享内存.png)

函数原型

```C++
// 返回值: 成功返回fd>0; 失败返回fd<0
int shm_open(const char *name, int oflag, mode_t mode);//打开创建共享内存文件
int shm_unlink(const char *name);//删除共享内存
int ftruncate(int fd, off_t length);//重置共享内存文件大小
// addr  ： 建立映射区的首地址，由Linux内核指定。使用时，直接传递NULL
// length： 欲创建映射区的大小
// prot  ： 映射区权限PROT_READ、PROT_WRITE、PROT_READ|PROT_WRITE
// flags ： 标志位参数(常用于设定更新物理区域、设置共享、创建匿名映射区)
//          MAP_SHARED:  会将映射区所做的操作反映到物理设备（磁盘）上,无血缘关系的进程通信
//          MAP_PRIVATE: 映射区所做的修改不会反映到物理设备。
//          MAP_ANONYMOUS:匿名映射区
// fd    ： 用来建立映射区的文件描述符
// offset： 映射文件的偏移(4k的整数倍)
void *mmap(void *addr, size_t length, int prot, int flags,int fd, off_t offset);//地址映射
int munmap(void *addr, size_t length);//解除映射
```

创建共享内存示例代码：

```C++
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//进程1，写入数据:
int main()
{
    int fd = shm_open("backdoor", O_CREAT|O_RDWR|O_EXCL, 0777);
    if(fd < 0) // 说明这个共享内存对象已经存在了
    {
        fd = shm_open("/sz02-shm", O_RDWR, 0777);
        printf("open ok\n");
        if(fd < 0)
        {
            printf("error open shm object\n");
            return 0;
        }
    }
    else
    {
        printf("create ok\n");
        // 说明共享内存对象是刚刚创建的
        ftruncate(fd, 1024); // 设置共享内存的尺寸

    }
    char* ptr = (char*)mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    strcpy(ptr, "write by t08");
    //shm_unlink("/sz02-shm");
    return 0;
}
```

读取共享内存数据示例代码：

```C++
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//进程2 读取共享数据
int main()
{
    int fd = shm_open("backdoor", O_RDWR, 0777);
    if(fd < 0)//不存在共享内存对象
     {
         printf("error open shm object\n");
         return 0;
     }
    char* ptr = (char*)mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    getchar();
    printf("%s\n", ptr);
    return 0;
}
```

