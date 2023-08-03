# python ctypes笔记

ctypes是python的外部函数库，它提供了与C兼容的数据类型，并允许调用dll或共享库中的函数。

### 1. 基础数据类型

ctypes定义了一些和C兼容的基本数据类型：c_bool, c_char, c_byte, c_ubyte, c_short, c_ushort, c_int, c_uint, c_long, c_ulong, c_longlong, c_ulonglong, c_size_t, c_ssize_t, c_float, c_double, c_char_p, c_wchar_p, c_void_p。

其中，`c_char_p`对应C中的`char *`，python中的bytes对象；`c_wchar_p`对应C中的`wchar *`，python中的字符串；`c_void_p`对应C中的`void *`，python中的int。当给指针类型的对象赋值时，将改变他们所致的内存地址，而不是他们所指向的内存区域的内容。
```python
from ctypes import *

s = "Hello, World"
c_s = c_wchar_p(s)

print(c_s) # c_wchar_p(1888740221520)
print(c_s.value)  # Hello, World

c_s.value = "Hi, there"
print(c_s) # c_wchar_p(1888759435184)
print(c_s.value)  # Hi, there
```

创建可改变的内存块的方式：`create_string_buffer() `函数，当前的内存块内容可以通过raw属性存取，使用value属性可以返回已NUL结尾的字符串。
```python
from ctypes import *

from ctypes import *
p = create_string_buffer(3)    # create a 3 byte buffer, initialized to NUL bytes
print(sizeof(p), repr(p.raw))  # 3 b'\x00\x00\x00'
p = create_string_buffer(b"Hello")     # create a buffer containing a NUL terminated string
print(sizeof(p), repr(p.raw))  #6 b'Hello\x00'
print(repr(p.value))  # b'Hello'
p = create_string_buffer(b"Hello", 10) # create a 10 byte buffer
print(sizeof(p), repr(p.raw))  # 10 b'Hello\x00\x00\x00\x00\x00'
p.value = b"Hi"
print(sizeof(p), repr(p.raw))  # 10 b'Hi\x00lo\x00\x00\x00\x00\x00'
```

想创建`wchar_t`类型的unicode字符串，可以使用create_unicode_buffer函数。

### 2. 加载动态链接库


### 3. 函数调用

通过设置argtypes属性可以指定从DLL导出函数的必选参数类型，指定数据类型可以防止不合理的参数传递（就像 C 函数的原型），并且会自动尝试将参数转换为需要的类型。

在不指定函数返回值类型的情况下，会默认返回C int类型，可以通过restype属性指定返回值类型。

```python
# strchr用于查找字符串中的一个字符，并返回该字符在字符串中第一次出现的位置
strchr = libc.strchr
strchr.argtypes = [c_char_p, c_char]
strchr(b"abcdef", b"d")  # 8059983

strchr.restype = c_char_p    # c_char_p is a pointer to a string
strchr(b"abcdef", b"d")  # b'def'
```

**传递指针**
如果需要往C函数传递对象的指针时，可以使用`byref()`函数通过引用传递，也可以使用`pointer()`函数实现同样的效果。只不过`pointer()`函数会先构造一个真实的指针对象，所以在Python代码本身不需要使用这个指针对象的情况下，`byref()`效率更高。

```python
i = c_int()
f = c_float()
s = create_string_buffer(b'\000' * 32)

libc.sscanf(b"1 3.14 Hello", b"%d %f %s", byref(i), byref(f), s) # 3
print(i.value, f.value, repr(s.value))  # 1 3.1400001049 b'Hello'
```

### 4. 结构体和联合体

结构体和联合必须继承自ctypes模块中的Structure和Union 。子类必须定义 `_fields_` 属性。 `_fields_` 是一个二元组列表，二元组中包含field name和field type。

type字段必须是一个ctypes类型，比如c_int、结构体、联合、数组、指针。

```python
from ctypes import *
class POINT(Structure):
    _fields_ = [("x", c_int),
                ("y", c_int)]

point = POINT(10, 20)
print(point.x, point.y)  # 10 20
point = POINT(y=5)
print(point.x, point.y)  # 0 5

# 一个结构体可以通过设置 type 字段包含其他结构体或者自身
class RECT(Structure):
    _fields_ = [("upperleft", POINT),
                ("lowerright", POINT)]

rc = RECT(point)
print(rc.upperleft.x, rc.upperleft.y)  # 0 5
```

**结构体和联合体的字节对齐**：与C的字节对齐一致，也可以在定义时指定类的`_pack_`属性来设置最大对齐字节数，功能同`#pragma pack(n)`。

**字节序**：ctypes中的结构体和联合使用的是本地字节序。要使用非本地字节序，可以使用 BigEndianStructure, LittleEndianStructure, BigEndianUnion, and LittleEndianUnion 作为基类。这些类不能包含指针字段。

**位域**：结构体和联合中是可以包含位域字段的。位域只能用于整型字段，位长度通过 `_fields_` 中的第三个参数指定。
```python
class Int(Structure):
    _fields_ = [("first_16", c_int, 16),
                ("second_16", c_int, 16)]

print(Int.first_16)   #<Field type=c_long, ofs=0:0, bits=16>
print(Int.second_16)  # <Field type=c_long, ofs=0:16, bits=16>
```

注意：ctypes不支持将带位域的结构体或联合体以值的方式传递给函数，栀粽通过指针的方式传递给函数。

### 5. 数组

创建数组类型的推荐方式是使用一个类型乘以一个正数：
```python
TenPointsArrayType = POINT * 10

class POINT(Structure):
    _fields_ = ("x", c_int), ("y", c_int)

class MyStruct(Structure):
    _fields_ = [("a", c_int),
                ("b", c_float),
                ("point_array", POINT * 4)]

# 打印数组中元素的个数
print(len(MyStruct().point_array))  # 4

# 遍历数组
arr = TenPointsArrayType()
for pt in arr:
    print(pt.x, pt.y)

# 在定义时初始化
TenIntegers = c_int * 10
ii = TenIntegers(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
for i in ii:
    print(i, end=" ")  # 1 2 3 4 5 6 7 8 9 10
```

### 6. 指针

调用`pointer()`函数可以创建ctypes对象的指针，通过指针实例的contents属性可以返货指针指向的真实对象。需要注意的是，contents每次都会返回一个真实对象的拷贝，而不是原对象的引用。
```python
i = c_int(42)
pi = pointer(i)

print(pi.contents)          # c_long(42)
print(pi.contents is i)     # False
pi.contents is pi.contents  # False
```

通过contents给指针赋值，会使得指针指向赋值对象的内存地址。
```python
i = c_int(99)
pi.contents = i
pi.contents  # c_long(99)
```

通过整数下标赋值可以改变指针所指向的真实内容。
```python
print(i)   # c_long(99)
pi[0] = 22
print(i)   # c_long(22)
```

**内部细节**：`pointer()`函数不只是创建了一个指针实例，它首先会创建一个指针类型。这是通过调用`POINTER()`函数实现的，它接收 ctypes 类型为参数，返回一个新的类型。

### 7. 类型转换

ctypes具有严格的类型检查，如果在函数argtypes中或结构体定义成员中有`POINTER(c_int)`类型，只有相同类型的实例才会被接受。也有一些例外，比如，你可以传递兼容的数组实例给指针类型。所以，对于`POINTER(c_int)`，ctypes也可以接受c_int类型的数组。

另外，如果一个函数argtypes列表中的参数显式的定义为指针类型(如`POINTER(c_int)`)，指针所指向的类型(这个例子中是c_int)也可以传递给函数，ctypes会自动调用对应的`byref()`转换。

```python
class Bar(Structure):
    _fields_ = [("count", c_int), ("values", POINTER(c_int))]

bar = Bar()
bar.values = (c_int * 3)(1, 2, 3)
bar.count = 3
for i in range(bar.count):
    print(bar.values[i])

```

我们也可以像C语言那样，做指针的强制转换。`cast()`接收两个参数，待转换对象的指针和转换后指针类型，返回转换后的指针对象。
```python
a = (c_byte * 4)()
cast(a, POINTER(c_int))  # <ctypes.LP_c_long object at ...>
```

所以`cast()`可以用来给结构体Bar的values字段赋值:
```python
bar = Bar()
bar.values = cast((c_byte * 4)(), POINTER(c_int))
print(bar.values[0])
```

### 8. 回调函数

ctypes可以创建一个指向Python可调用对象的C函数，即回调函数。

首先，你必须为回调函数创建一个类，这个类知道调用约定，包括返回值类型以及函数接收的参数类型及个数。

`CFUNCTYPE()`工厂函数使用cdecl调用约定创建回调函数类型。在Windows上，`WINFUNCTYPE()`工厂函数使用stdcall调用约定为回调函数创建类型。这些工厂函数的第一个参数是返回值类型，回调函数的参数类型作为剩余参数。

我们以C标准库中的qsort函数为例，它可以使用指定回调函数对数据进行排序，入参为待排序数据的指针、元素个数、元素大小，以及指向排序函数的指针，即回调函数。然后，回调函数接受两个元素的指针，如果第一个元素小于第二个，则返回一个负整数，如果相等则返回0，否则返回一个正整数。

```python
IntArray5 = c_int * 5
ia = IntArray5(5, 1, 7, 33, 99)
qsort = libc.qsort
qsort.restype = None

CMPFUNC = CFUNCTYPE(c_int, POINTER(c_int), POINTER(c_int))
def py_cmp_func(a, b):
    print("py_cmp_func", a[0], b[0])
    return a[0] - b[0]

cmp_func = CMPFUNC(py_cmp_func)
qsort(ia, len(ia), sizeof(c_int), CMPFUNC(py_cmp_func))

for i in ia: print(i, end=" ")
```

这些工厂函数可以当作装饰器工厂，所以可以这样写：
```python
@CFUNCTYPE(c_int, POINTER(c_int), POINTER(c_int))
def py_cmp_func(a, b):
    print("py_cmp_func", a[0], b[0])
    return a[0] - b[0]

qsort(ia, len(ia), sizeof(c_int), py_cmp_func)
```

注意：请确保你的`CFUNCTYPE()`对象的引用周期与它们在C代码中的使用期一样长，如果不这样做，它们可能会被垃圾回收，导致程序在执行回调函数时发生崩溃。

### 9. ctypes中的工具函数

`ctypes.addressof(obj)`
以整数形式返回内存缓冲区地址，obj必须为一个ctypes类型的实例。

`ctypes.alignment(obj_or_type)`
返回一个 ctypes 类型的对齐要求，obj_or_type 必须为一个 ctypes 类型或实例。

`ctypes.byref(obj[, offset])`
返回指向 obj 的轻量指针，该对象必须为一个 ctypes 类型的实例。 offset默认值为零，且必须为一个将被添加到内部指针值的整数。

`ctypes.cast(obj, type)`
指针类型强制转换。

`ctypes.create_string_buffer(init_or_size, size=None)`
创建一个可变的字符缓冲区，返回的对象是一个 c_char 的 ctypes 数组。
如果将一个字符串指定为第一个参数，则将使缓冲区大小比其长度多一项以便数组的最后一项为一个 NUL 终结符。 可以传入一个整数作为第二个参数以允许在不使用字符串长度的情况下指定数组大小。

`ctypes.create_unicode_buffer(init_or_size, size=None)`
创建一个可变的 unicode 字符缓冲区。 返回的对象是一个 c_wchar 的 ctypes 数组。

`ctypes.get_errno()`
返回调用线程中系统 errno 变量的 ctypes 私有副本的当前值。

`ctypes.memmove(dst, src, count)`
与标准 C memmove 库函数相同：将 count 个字节从 src 拷贝到 dst。 dst 和 src 必须为整数或可被转换为指针的 ctypes 实例。

`ctypes.memset(dst, c, count)`
与标准 C memset 库函数相同：将位于地址 dst 的内存块用 count 个字节的 c 值填充。 dst 必须为指定地址的整数或 ctypes 实例。

`ctypes.POINTER(type)`
这个工厂函数创建并返回一个新的 ctypes 指针类型。 指针类型会被缓存并在内部重用，因此重复调用此函数耗费不大。 type 必须为 ctypes 类型。

`ctypes.pointer(obj)`
此函数会创建一个新的指向 obj 的指针实例。 返回的对象类型为 POINTER(type(obj))。

注意：如果你只是想向外部函数调用传递一个对象指针，你应当使用更为快速的 byref(obj)。

`ctypes.resize(obj, size)`
此函数可改变 obj 的内部内存缓冲区大小，其参数必须为 ctypes 类型的实例。 没有可能将缓冲区设为小于对象类型的本机大小值，该值由 sizeof(type(obj)) 给出，但将缓冲区加大则是可能的。

`ctypes.set_errno(value)`
设置调用线程中系统 errno 变量的 ctypes 私有副本的当前值为 value 并返回原来的值。

`ctypes.sizeof(obj_or_type)`
返回 ctypes 类型或实例的内存缓冲区以字节表示的大小。 其功能与 C sizeof 运算符相同。

`ctypes.string_at(address, size=-1)`
此函数返回从内存地址 address 开始的以字节串表示的 C 字符串。 如果指定了 size，则将其用作长度，否则将假定字符串以零值结尾。

### 10. 公共基类`_CData`

`ctypes._CData`这个非公有类是所有 ctypes 数据类型的共同基类。所有 ctypes 类型的实例都包含一个存放 C 兼容数据的内存块；该内存块的地址可由 `addressof()` 辅助函数返回。还有一个实例变量被公开为 `_objects`，此变量包含其他在内存块包含指针的情况下需要保持存活的 Python 对象。

ctypes 数据类型的通用方法，它们都是类方法（严谨地说，它们是 metaclass 的方法）:

- `from_buffer(source[, offset])`：此方法返回一个共享 source 对象缓冲区的 ctypes 实例。 source 对象必须支持可写缓冲区接口。 可选的 offset 形参指定以字节表示的源缓冲区内偏移量；默认值为零。 如果源缓冲区不够大则会引发 ValueError。
- `from_buffer_copy(source[, offset])`：此方法创建一个 ctypes 实例，从 source 对象缓冲区拷贝缓冲区，该对象必须是可读的。 可选的 offset 形参指定以字节表示的源缓冲区内偏移量；默认值为零。 如果源缓冲区不够大则会引发 ValueError。
- `from_address(address)`：此方法会使用 address 所指定的内存返回一个 ctypes 类型的实例，该参数必须为一个整数。
- `from_param(obj)`：此方法会将 obj 适配为一个 ctypes 类型。 它调用时会在当该类型存在于外部函数的 argtypes 元组时传入外部函数调用所使用的实际对象；它必须返回一个可被用作函数调用参数的对象。所有 ctypes 数据类型都带有这个类方法的默认实现，它通常会返回 obj，如果该对象是此类型的实例的话。 某些类型也能接受其他对象。

ctypes 数据类型的通用实例变量:
- `_b_base_`：有时 ctypes 数据实例并不拥有它们所包含的内存块，它们只是共享了某个基对象的部分内存块。 `_b_base_` 只读成员是拥有内存块的根 ctypes 对象。
- `_b_needsfree_`：这个只读变量在 ctypes 数据实例自身已分配了内存块时为真值，否则为假值。
- `_objects`：这个成员或者为 None，或者为一个包含需要保持存活以使内存块的内存保持有效的 Python 对象的字典。 这个对象只是出于调试目的而对外公开；绝对不要修改此字典的内容。

