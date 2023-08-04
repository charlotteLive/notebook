### GNU C 编译器扩展语法精讲

### 1. 指定初始化

#### 1.1 指定初始化数组元素

在C语言标准中，当我们定义并初始化一个数组时，常用方法如下：

```c
int a[10] = {0,1,2,3,4,5,6,7,8};
```

按照这种固定的顺序，我们可以依次给a[0]和a[8]赋值。因为没有对a[9]赋值，所以编译器会将a[9]默认设置为0。当数组长度比较小时，使用这种方式初始化比较方便；当数组比较大，而且数组里的非零元素并不连续时，再按照固定顺序初始化就比较麻烦了。

C99标准改进了数组的初始化方式，支持指定元素初始化，不再按照固定的顺序初始化。

```c
int b[100] ={ [10] = 1, [30] = 2};
```

通过数组元素索引，我们可以直接给指定的数组元素赋值。除了数组，一个结构体变量的初始化，也可以通过指定某个结构体成员直接赋值。

如果我们想给数组中某一个索引范围的数组元素初始化，可以采用下面的方式。

```c
int main(void)
{
    int b[100] = { [10 ... 30] = 1, [50 ... 60] = 2 }；
    for(int i = 0; i < 100; i++)
    {
        printf("%d  ", a[i]);
        if( i % 10 == 0)
            printf("\n");
    }
    return 0;   
}
```

GNU C支持使用...表示范围扩展，这个特性不仅可以使用在数组初始化中，也可以使用在switch-case语句中，如下面的程序。

```c
#include<stdio.h>
int main(void)
{
    int i = 4;
    switch(i)
    {
        case 1:
            printf("1\n");
            break;
        case 2 ... 8:
            printf("%d\n",i);
            break;
        case 9:
            printf("9\n");
            break;
        default:
            printf("default!\n");
            break;
    }
    return 0;
}
```

在这个程序中，如果当case值为2～8时，都执行相同的case分支，我们就可以通过case 2...8：的形式来简化代码。这里同样有一个细节需要注意，**就是...和其两端的数据范围2和8之间也要有空格**，不能写成2...8的形式，否则会报编译错误。

#### 1.2 指定初始化结构体成员

和数组类似，在C语言标准中，初始化结构体变量也要按照固定的顺序，但在GNU C中我们可以通过结构域来指定初始化某个成员。

```c
struct student{
    char name[20];
    int age;
};

int main(void)
{
    struct student stu1={ "wit",20 };
    printf("%s:%d\n",stu1.name,stu1.age);

    struct student stu2=
    {
        .name = "wanglitao",
        .age  = 28
    };
    printf("%s:%d\n",stu2.name,stu2.age);

    return 0;
}
```

初始化stu2时，我们采用GNU C的初始化方式，通过结构域名.name和.age，就可以给结构体变量的某一个指定成员直接赋值。**当结构体的成员很多时，使用第二种初始化方式会更加方便**。

#### 1.3 Linux内核驱动注册

在Linux内核驱动中，大量使用GNU C的这种指定初始化方式，通过结构体成员来初始化结构体变量。如在字符驱动程序中，我们经常见到下面这样的初始化。

```c
static const struct file_operations ab3100_otp_operations = {
    .open        = ab3100_otp_open,
    .read        = seq_read,
    .llseek      = seq_lseek,
    .release     = single_release,
};
```

在驱动程序中，我们经常使用`file_operations`这个结构体来注册我们开发的驱动，然后系统会以回调的方式来执行驱动实现的具体功能。结构体`file_operations`在Linux内核中的定义如下。

```c
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
    ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
    int (*iterate) (struct file *, struct dir_context *);
    unsigned int (*poll) (struct file *, struct poll_table_struct *);
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    int (*open) (struct inode *, struct file *);
    int (*flush) (struct file *, fl_owner_t id);
    int (*release) (struct inode *, struct file *);
    int (*fsync) (struct file *, loff_t, loff_t, int datasync);
    int (*aio_fsync) (struct kiocb *, int datasync);
    int (*fasync) (int, struct file *, int);
    int (*lock) (struct file *, int, struct file_lock *);
    ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
    unsigned long (*get_unmapped_area)(struct file *,
                                       unsigned long, unsigned long, unsigned long, unsigned long);
    int (*check_flags)(int);
    int (*flock) (struct file *, int, struct file_lock *);
    ssize_t (*splice_write)(struct pipe_inode_info *, 
                            struct file *, loff_t *, size_t, unsigned int);
    ssize_t (*splice_read)(struct file *, loff_t *, 
                           struct pipe_inode_info *, size_t, unsigned int);
    int (*setlease)(struct file *, long, struct file_lock **, void **);
    long (*fallocate)(struct file *file, int mode, loff_t offset,
                      loff_t len);
    void (*show_fdinfo)(struct seq_file *m, struct file *f);
    #ifndef CONFIG_MMU
    unsigned (*mmap_capabilities)(struct file *);
    #endif
};
```

结构体 `file_operations` 里面定义了很多结构体成员，而在这个驱动中，我们只初始化了部分成员变量。通过访问结构体的各个成员域来指定初始化，当结构体成员很多时优势就体现出来了，初始化会更加方便。

#### 1.4 指定初始化的好处

- 使用灵活。
- 易于维护。如果采用C标准按照固定顺序赋值，当`file_operations`结构体类型发生变化时，如添加了一个成员、删除了一个成员、调整了成员顺序，那么使用该结构体类型定义变量的大量C文件都需要重新调整初始化顺序。我们通过指定初始化方式，就可以避免这个问题。

### 2. 宏构造利器：语句表达式

GNU C对C语言标准作了扩展，允许在一个表达式里内嵌语句，允许在表达式内部使用局部变量、for循环和goto跳转语句。这种类型的表达式，我们称为语句表达式。语句表达式的格式如下。

```c
({ 表达式1; 表达式2; 表达式3; })
```

语句表达式最外面使用小括号()括起来，里面一对大括号{}包起来的是代码块，代码块里允许内嵌各种语句。语句的格式可以是一般表达式，也可以是循环、跳转语句。

和一般表达式一样，语句表达式也有自己的值。语句表达式的值为内嵌语句中最后一个表达式的值。

```c
int main(void)
{
    int sum = 0;
    sum = 
    ({
        int s = 0;
        for( int i = 0; i < 10; i++)
            s = s + i;
        s;
    });
    printf("sum = %d\n",sum);
    return 0;
}
```

在上面的程序中，我们在语句表达式内定义了局部变量，使用了for循环语句。在语句表达式内，我们同样可以使用goto进行跳转。

```c
int main(void)
{
    int sum = 0;
    sum = 
    ({
        int s = 0;
        for( int i = 0; i < 10; i++)
            s = s + i;
            goto here;
            s;  
    });
    printf("sum = %d\n",sum);
here:
    printf("here:\n");
    printf("sum = %d\n",sum);
    return 0;
}
```

#### 2.2 在宏定义中使用语句表达式

语句表达式的主要用途在于定义功能复杂的宏。使用语句表达式来定义宏，不仅可以实现复杂的功能，还能避免宏定义带来的歧义和漏洞。

```c
#define max(x, y) ({    \
    typeof(x) _x = (x); \
    typeof(y) _y = (y); \
    (void) (&_x == &_y);\
    _x > _y ? _x : _y; })
```

typeof是GNU C新增的一个关键字，用来获取数据类型。

`(void) (&x == &y);`这句话，一是用来给用户提示一个警告，对于不同类型的指针比较，编译器会给一个警告，提示两种数据类型不同；二是，当两个值比较，比较的结果没有用到，有些编译器可能会给出一个warning，加个(void)后，就可以消除这个警告！

#### 2.3 内核中的语句表达式

语句表达式，作为GNU C对C标准的一个扩展，在内核中，尤其在内核的宏定义中被大量使用。使用语句表达式定义宏，不仅可以实现复杂的功能，还可以避免宏定义带来的一些歧义和漏洞。如在Linux内核中，max_t和min_t的宏定义，就使用了语句表达式。

```c
#define min(x, y) ({                \
    typeof(x) _min1 = (x);          \
    typeof(y) _min2 = (y);          \
    (void) (&_min1 == &_min2);      \
    _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                \
    typeof(x) _max1 = (x);          \
    typeof(y) _max2 = (y);          \
    (void) (&_max1 == &_max2);      \
    _max1 > _max2 ? _max1 : _max2; })
```

### 3. typeof与container_of宏

### 3.1 typeof关键字

GNU C扩展了一个关键字typeof，用来获取一个变量或表达式的类型。

```c
int main(void)
{
    int i = 2;
    typeof(i) k = 6;

    int *p = &k;
    typeof(p) q = &i;

    printf("k = %d\n",k); 
    printf("*p= %d\n",*p); 
    printf("i = %d\n",i); 
    printf("*q= %d\n",*q); 
    return 0;
}
```

#### 3.2 Linux内核中的container_of宏

有了上面语句表达式和typeof的基础知识，我们就可以分析Linux内核第一宏：`container_of`了。

```c
// 取结构体member成员的地址，减去这个成员在结构体type中的偏移，就得到了结构体type的首地址
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define  container_of(ptr, type, member) ({    \
     const typeof( ((type *)0)->member ) *__mptr = (ptr); \
     (type *)( (char *)__mptr - offsetof(type,member) );})
```

它的主要作用就是，根据结构体某一成员的地址，获取这个结构体的首地址。根据宏定义，我们可以看到，这个宏有三个参数：type为结构体类型，member为结构体内的成员，ptr为结构体内成员member的地址。

也就是说，如果我们知道了一个结构体的类型和结构体内某一成员的地址，就可以获得这个结构体的首地址。container_of宏返回的就是这个结构体的首地址。例如现在，我们定义一个结构体类型student。

```c
struct student
{
    int age;
    int num;
    int math;
};
int main(void)
{
    struct student stu;
    struct student *p;
    p = container_of( &stu.num, struct student, num);
    return 0;
}
```

在内核中，我们经常会遇到这种情况：我们传给某个函数的参数是某个结构体的成员变量，在这个函数中，可能还会用到此结构体的其他成员变量，那么该怎么操作呢？container_of就是干这个的，通过它，我们可以首先找到结构体的首地址，然后通过结构体的成员访问就可以访问其他成员变量了。

```c
struct student
{
    int age;
    int num;
    int math;
};
int main(void)
{
    struct student stu = { 20, 1001, 99};

    int *p = &stu.math;
    struct student *stup = NULL;
    stup = container_of( p, struct student, math);
    printf("%p\n",stup);
    printf("age: %d\n",stup->age);
    printf("num: %d\n",stup->num);

    return 0;     
}
```

### 4. 零长数组

顾名思义，零长度数组就是长度为0的数组。

ANSI C标准规定：定义一个数组时，数组的长度必须是一个常数，即数组的长度在编译的时候是确定的。

C99标准规定：可以定义一个变长数组。也就是说，数组的长度在编译时是未确定的，在程序运行的时候才确定，甚至可以由用户指定大小。

```c
int main(void)
{
    int len;

    printf("input array len:");
    scanf("%d",&len);
    int a[len];

    for(int i=0;i<len;i++)
    {
        printf("a[%d]= ",i);
        scanf("%d",&a[i]);
    }

      printf("a array print:\n");
    for(int i=0;i<len;i++)
        printf("a[%d] = %d\n",i,a[i]);

    return 0;
}
```

GNU C可能觉得变长数组还不过瘾，再来一记“猛锤”：支持零长度数组。零长度数组有一个奇特的地方，就是它不占用内存存储空间。我们使用sizeof关键字来查看一下零长度数组在内存中所占存储空间的大小。

```c
int buffer[0];
int main(void)
{
    printf("%d\n", sizeof(buffer)); // 0
    return 0;
}
```

零长度数组经常以变长结构体的形式，在某些特殊的应用场合使用。在一个变长结构体中，零长度数组不占用结构体的存储空间，但是我们可以通过使用结构体的成员a去访问内存，非常方便。变长结构体的使用示例如下。

```c
struct buffer{
    int len;
    int a[0];
};
int main(void)
{
    struct buffer *buf;
    buf = (struct buffer *)malloc \
        (sizeof(struct buffer)+ 20);

    buf->len = 20;
    strcpy(buf->a, "hello wanglitao!\n");
    puts(buf->a);

    free(buf);  
    return 0;
}
```

在这个程序中，我们使用`malloc`申请一片内存，大小为`sizeof(buffer)+20`，即24字节。其中4字节用来表示内存的长度20，剩下的20字节空间，才是我们真正可以使用的内存空间。我们可以通过结构体成员a直接访问这片内存。

### 5. 属性声明`__attribute__`

GNU C增加了一个`__attribute__`关键字用来声明一个函数、变量或类型的特殊属性，用于指导编译器在编译程序时进行特定方面的优化或代码检查。

`__attribute__`的使用非常简单，当我们定义一个函数、变量或类型时，直接在它们名字旁边添加下面的属性声明即可。

```c
__atttribute__((ATTRIBUTE))
```

需要注意的是，`__attribute__`后面是两对小括号，不能图方便只写一对，否则编译就会报错。括号里面的ATTRIBUTE表示要声明的属性。目前`__attribute__`支持十几种属性声明：section，aligned，packed，format，weak，alias，noinline，always_inline。

在这些属性中，aligned和packed用来显式指定一个变量的存储对齐方式。在正常情况下，当我们定义一个变量时，编译器会根据变量类型给这个变量分配合适大小的存储空间，按照默认的边界对齐方式分配一个地址。而使用__`atttribute`__这个属性声明，就相当于告诉编译器：按照我们指定的边界对齐方式去给这个变量分配存储空间。

```c
char c2 __attribute__((aligned(8)) = 4;
int global_val __attribute__((section(".data")));
```

有些属性可能还有自己的参数。比如 aligned(8) 表示这个变量按8字节地址对齐，参数也要使用小括号括起来。如果属性的参数是一个字符串，小括号里的参数还要用双引号引起来。

当然，我们也可以对一个变量同时添加多个属性说明。在定义时，各个属性之间用逗号隔开就可以了。

```c
char c2 __attribute__((packed,aligned(4)));
char c2 __attribute__((packed,aligned(4))) = 4;
__attribute__((packed,aligned(4))) char c2 = 4;
```

在上面的示例中，我们对一个变量添加2个属性声明，这两个属性都放在` __atttribute__ (())` 的2对小括号里面，属性之间用逗号隔开。这里还有一个细节，就是属性声明要紧挨着变量，上面的三种定义方式都是没有问题的，但下面的定义方式在编译的时候可能就通不过。

```c
char c2 = 4 __attribute__((packed,aligned(4)));
```

#### 5.1 section属性

section属性的主要作用是：在程序编译时，将一个函数或变量放到指定的段，即放到指定的section中。

一个可执行文件主要由代码段、数据段、BSS段构成。代码段主要存放编译生成的可执行指令代码，数据段和BSS段用来存放全局变量、未初始化的全局变量。代码段、数据段和BSS段构成了一个可执行文件的主要部分。除了这三个段，可执行文件中还包含其他一些段。我们可以使用readelf命令查看一个可执行文件中各个section的信息。

在GNU C中，我们可以通过`__attribute__`的section属性，显式指定一个函数或变量，在编译时放到指定的section里面。未初始化的全局变量默认是放在`.bss` section中的，现在我们可以通过section属性声明，把这个未初始化的全局变量放到数据段`.data`中。

```c
int global_val = 8;
int uninit_val __attribute__((section(".data")));
int main(void)
{
    return 0;
}
```

通过上面的 readelf 命令查看符号表，我们可以看到，uninit_val 这个未初始化的全局变量，通过`__attribute__((section(".data")))` 属性声明，就和初始化的全局变量一样，被编译器放在了数据段`.data`中。

#### 5.2 aligned和packed属性

GNU C通过__attribute__来声明aligned和packed属性，指定一个变量或类型的对齐方式。这两个属性用来告诉编译器：在给变量分配存储空间时，要按指定的地址对齐方式给变量分配地址。如果你想定义一个变量，在内存中以8字节地址对齐，就可以这样定义。

```c
int a __attribute__((aligned(8));
```

通过aligned属性，我们可以显式地指定变量a在内存中的地址对齐方式。使用时要注意，地址对齐的字节数必须是2的幂次方，否则编译就会出错。

为了配合计算机的硬件设计，编译器在编译程序时，对于一些基本数据类型，如int、char、short、float等，会按照其数据类型的大小进行地址对齐，按照这种地址对齐方式分配的存储地址，CPU一次就可以读写完毕。虽然边界对齐会造成一些内存空洞，浪费一些内存单元，但是在硬件上的设计却大大简化了。这也是编译器给我们定义的变量分配地址时，不同类型的变量按照不同字节数地址对齐的主要原因。

结构体作为一种复合数据类型，编译器在给一个结构体变量分配存储空间时，不仅要考虑结构体内各个基本成员的地址对齐，还要考虑结构体整体的对齐。为了结构体内各个成员地址对齐，编译器可能会在结构体内填充一些空间；为了结构体整体对齐，编译器可能会在结构体的末尾填充一些空间。





aligned属性一般用来增大变量的地址对齐，元素之间因为地址对齐会造成一定的内存空洞。而packed属性则与之相反，一般用来减少地址对齐，指定变量或类型使用最可能小的地址对齐方式。

```c
struct data{
     char a;
     short b __attribute__((packed));
     int c __attribute__((packed));
 };
 int main(void)
 {
     struct data s;
     printf("size: %d\n",sizeof(s));
     printf("&s.a: %p\n",&s.a);
     printf("&s.b: %p\n",&s.b);
     printf("&s.c: %p\n",&s.c);
 }
```

在这个程序中，我们将结构体的成员 b 和 c 使用 packed 属性声明，就是告诉编译器，尽量使用最可能小的地址对齐给它们分配地址，尽可能地减少内存空洞。程序的运行结果如下。

```c
 size: 7
 &s.a: 0028FF30
 &s.b: 0028FF31
 &s.c: 0028FF33
```

通过结果我们看到，结构体内各个成员地址的分配，使用最小1字节的对齐方式，导致整个结构体的大小只有7个字节。

这个特性在底层驱动开发中还是非常有用的。比如，你想定义一个结构体，封装一个 IP 控制器的各种寄存器。在 ARM 芯片中，每一个控制器的寄存器地址空间一般是连续存在的。如果考虑数据对齐，结构体内有空洞，这样就跟实际连续的寄存器地址不一致了，使用 packed 就可以避免这个问题，结构体的每个成员都紧挨着依次分配存储地址，这样就避免了各个成员元素因地址对齐而造成的内存空洞。

```c
 struct data{
     char a;
     short b ;
     int c ;
 }__attribute__((packed));
```

我们对整个结构体添加 packed 属性，和分别对每个成员添加 packed 属性，效果是一样的。修改结构体后，程序的运行结果跟上面程序运行结果相同——结构体的大小为7，结构体内各成员地址相同。





在Linux内核源码中，我们经常看到aligned和packed一起使用，即对一个变量或类型同时使用aligned和packed属性声明。这样做的好处是：既避免了结构体内各成员因地址对齐产生内存空洞，又指定了整个结构体的对齐方式。

```c
 struct data{
     char a;
     short b ;
     int c ;
 }__attribute__((packed,aligned(8)));
 int main(void)
 {
     struct data s;
     printf("size: %d\n",sizeof(s));
     printf("&s.a: %p\n",&s.a);
     printf("&s.b: %p\n",&s.b);
     printf("&s.c: %p\n",&s.c);
 }
```

程序运行结果如下。

```c
 size: 8
 &s.a: 0028FF30
 &s.b: 0028FF31
 &s.c: 0028FF33
```

在这个程序中，结构体 data 虽然使用 packed 属性声明，整个长度变为7，但是我们同时又使用了 aligned(8) 指定其按8字节地址对齐，所以编译器要在结构体后面填充1个字节，这样整个结构体的大小就变为8字节，按8字节地址对齐。