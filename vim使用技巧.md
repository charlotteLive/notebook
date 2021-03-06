## Vim使用技巧



编辑操作

```
o   -  向后插入一行
O   -  向前插入一行
i   -  光标前插入
a   -  光标后插入
dd  -  删除一行
dw  -  删除一个单词
x   -  删除一个字符
y   -  复制，与可视化操作进行配合
yy  -  复制一行
u   -  撤销
c^r -  恢复
p   -  粘贴


c^n -  智能提示
gb  -  移动到光标所处函数或变量的定义处
*   -  读取当前字符，并移动到本屏幕内下一次出现的地方
#   -  读取当前字符，并移动到本屏幕内上一次出现的地方
[{  -  跳转到本代码块的开头
]}  -  跳转到本代码块的结尾


/   -  向后查找
?   -  向前查找
n   -  跳转到下一个查找结果
N   -  跳转到上一处查找结果
```

快速跳转

```
gg - 跳到文件的开始 
G  - 跳到文件的结束 
10gg 或10G  - 跳到第10行 

^   - 行首 
$   - 行尾
0   - 第一个字符
```

多文件切换

``` 
:e + flinename  - 打开另一个文件
:bn             - 切换到下一个文件
:bp             - 切换到上一个文件
:ls             - 列出打开的文件，并带有编号
:b1~n           - 切换至第n个文件
```

可视化操作

```
ctrl + v  - 进入化模式
v         - 进入字符可视化模式
V         - 进入行可视化模式
d         - 删除选定的块
c         - 删除选定的块并进入插入模式
y         - 复制选定的块
```

### 多窗口使用技巧

#### 1. 打开关闭多个窗口

横向切割窗口：`:sp + 窗口名`

纵向切割窗口：`:vsp + 窗口名`

关闭其他窗口仅保留当前窗口`:only`

#### 2. 窗口切换

`ctrl+w+j/k`，通过j/k可以上下切换，或者`ctrl+w`加上下左右键，还可以通过快速双击`ctrl+w`依次切换窗口。

#### 3. 窗口大小调整

纵向调整：`:res(ize) +/- num`当前窗口加减num行

横向调整：`:vertical res +/- num`当前窗口加减num列

#### 4. 文件浏览

`:Ex` 开启目录浏览器，可以浏览当前目录下的所有文件，并可以选择；

`:Ve` 水平分割当前窗口，并在左分屏中开启目录浏览器。

`:He`上下分屏，并在下分屏中开启目录浏览器。

#### 5. vim与shell之间切换

在vim下输入`:shell`可以在不关闭vi的情况下切换到shell命令行，其实是在vim内开启终端；要返回vim，只有输入`exit`退出终端。

最简单的方式还是直接在vim下执行shell指令，输入`:!{program}`执行相关shell命令。





![vim快捷键](picture/vim快捷键.png)