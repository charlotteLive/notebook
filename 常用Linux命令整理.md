## 常用Linux命令整理

## 1. 常用命令拾遗

which，查看程序的binary文件所在路径：

```sh
$ which scp
/usr/bin/scp
```

whereis，查看程序的搜索路径，当系统中安装了同一软件的多个版本时，不确定使用的是哪个版本时，这个命令就能派上用场。

```sh
$ whereis ls
ls: /usr/bin/ls /usr/share/man/man1/ls.1.gz
```

find，查找目标文件夹中是否有指定名称的文件或目录:

```sh
$ find ./ -name '*.o'
```

tail，动态显示文本最新信息:

```sh
$ tail -f crawler.log
```

grep，在指定文件或目录下查找指定的字符串：

```sh
$ grep -in 'int' *  #-i忽略大小写  -n显示行号
```

ln，创建符号链接或硬链接：

```sh
 $ ln -s /home/ubuntu/repos/ repos_ln #创建指定目录的符号链接
```

查看磁盘空间利用大小

```shell
df -h   #-h: human缩写，以易读的方式显示结果
```

查看当前目录所占空间大小

```shell
du -sh   #-s递归整个目录的大小
```

查看当前目录下所有子文件夹排序后的大小

```sh
du -sh `ls` | sort
```

touch，更新文件的时间属性或者创建新的空文件：

```sh
touch test.txt   #文件存在则更新时间标签为当前系统时间，文件不存在则创建空文件
```

free，查看内存的使用情况：

```shell
free -h
              总计         已用        空闲      共享    缓冲/缓存    可用
内存：        4.7G        923M        3.1G         18M        664M        3.6G
交换：        2.0G          0B        2.0G
```

 一行命令杀死包含指定字符串的进程：

```sh
ps -ef | grep 'tmpdata' | grep -v 'grep' | awk '{print $2}' | xargs kill -9
```





### 2. 部分常用Linux命令详解

#### 2.1 awk

 awk是unix下的文本处理工具，功能复杂灵活，可以实现诸如cat, grep, cut, head, sed, wc 等所有文本处理命令的功能。

首先，awk脚本的结构是这样的：

```awk
# comment
Pattern1 { ACTIONS; }
# comment
Pattern2 { ACTIONS; }
```

awk从标准输入逐行读取内容，然后使用不同的**模式**(Pattern)进行匹配，如果匹配成功，则会执行对应大括号内的**动作**(Action)。这就是awk的执行模型。读取、匹配、执行，就这么简单，所用复杂的功能都是在这个模型之上衍生出来的。

在说模式和动作之前，先简单提一下**变量**。

awk的变量有三种：字符串、数字和字典。字符串和数字可以相互转换。

变量无需声明，可直接使用，如`{a=1}`。数字变量可以执行数学运算，如`{a+=1}` 。字典赋值使用方括号，如`{a[hi]=1}` 。

awk内置一组特殊变量，分别是`$0, $1, ..., $n`。它们都以美元符号开头，所面跟一个数字。它们的值取自 awk 读入的文体内容。比如 awk 读入的一行内容为`a b c`，那么对应的`$1`的内容为`a`，`$2`和`$3`的内容分别为`b`和`c`；`$0`比较特别，其内容是读入的整行即`a b c`。 awk默认使用**一个或多个空白字符切割字段**。

awk还有另一拨不常用的高级内置变量，在有些场景下有奇效，大家留点印像就好了。

```awk
BEGIN { # 用户可以修改
  FS = ",";   # 内容分割符
  RS = "\n";  # 行（记录）分割符
  OFS = " ";  # 输出内容分割符
  ORS = "\n"; # 输出行（记录）分割符
}
{ # 用户无法修改
  NF          # 当前行字段（列）数量
  NR          # 当前行的行数
  ARGV / ARGC # 脚本参数
}
```

------

有了变量基础，我们再说模式。模式有三类：正则模式、布尔模式和特殊模式。

典型的正则模式有

```awk
/admin/ { ... }     # 匹配包含 admin 的内容
/^admin/ { ... }    # 匹配 admin 开头的内容
/admin$/ { ... }    # 匹配 admin 结尾的内容
/^[0-9.]+ / { ... } # 匹配数字开头的内容
/(POST|PUT|DELETE)/ # 匹配包含部分 http 请求的内容
```

对于更简单的场景，你还可以直接使用变量比较运算来过滤内容，比如

```awk
$1 == 200 { ... } # 匹配状态码为 200 的请求日志（假设第一列为 http 状态码）
$1 >= 500 { ... } # 匹配 5xx 请求
$1 != 200 { ... } # 匹配非 200 请求
```

你还可以使用布尔表达式将这些匹配模式组合起来，比如

```awk
/admin/ || $1 >= 500 # 匹配 admin 接口的 5xx 错误
```

除此之外，awk还支持两种特殊的模式：**BEGIN**和**END**，分别在脚本开始之前和结束之后触发。比如`awk 'BEGIN{c=0}/admin/{c+=1}END{print c}'`会在结束的时候输出含有admin日志的**行数**。

------

最后说一下动作。简单的动作有

```awk
{ a=$1; b=$0 } # 变量赋值
{ c[$1] = $2 } # 字典赋值
{ exit; }      # 结束程序，很少使用
{ next; }      # 跳过当前行
```

awk还支持条件分支和循环结构，如下

```awk
{ if ($3 >= 500) { ACTION }
  else if ($3 >= 400) { ACTION }
  else { ACTION }
}
{ for (i=1; i<x; i++) { ACTION } }
{ for (item in c) { ACTION } }
```

最后给几个示例。

统计 user 接口单ip的请求数量超过10次的ip

```sh
/user/ { ip=$1; ip_count[ip]++ }
END{ for (ip in ip_count) { if (ip_count[ip] > 10) { print ip, ip_count[ip] } } }
```

请理 ubuntu 的 rc 配置包

```
dpkg -l|awk '/^rc/ {print $2}'|xargs aptitude purge -y
```

#### 2.2 xargs

xargs是给命令传递参数的一个过滤器，也是组合多个命令的一个工具。它可以将管道或标准输入（stdin）数据转换成命令行参数，也能够从文件的输出中读取数据。xargs 也可以将单行或多行文本输入转换为其他格式，例如多行变单行，单行变多行。

xargs 默认的命令是 echo，这意味着通过管道传递给 xargs 的输入将会包含换行和空白，不过通过 xargs 的处理，换行和空白将被空格取代。

xargs 是一个强有力的命令，它能够捕获一个命令的输出，然后传递给另外一个命令。之所以能用到这个命令，关键是由于很多命令不支持|管道来传递参数，而日常工作中有有这个必要，所以就有了 xargs 命令，例如：

``` sh
find /sbin -perm +700 |ls -l       #这个命令是错误的
find /sbin -perm +700 |xargs ls -l   #这样才是正确的
```

xargs 一般是和管道一起使用。

**命令格式：**

```sh
somecommand |xargs -item  command
```

**参数：**

- -a file 从文件中读入作为 stdin
- -e flag ，注意有的时候可能会是-E，flag必须是一个以空格分隔的标志，当xargs分析到含有flag这个标志的时候就停止。
- -p 当每次执行一个argument的时候询问一次用户。
- -n num 后面加次数，表示命令在执行的时候一次用的argument的个数，默认是用所有的。
- -t 表示先打印命令，然后再执行。
- -i 或者是-I，这得看linux支持了，将xargs的每项名称，一般是一行一行赋值给 {}，可以用 {} 代替。
- -L num 从标准输入一次读取 num 行送给 command 命令。
- -l 同 -L。
- -d delim 分隔符，默认的xargs分隔符是回车，argument的分隔符是空格，这里修改的是xargs的分隔符。

**示例如下：**

1）使用-n多行输出：

```sh
cat test.txt | xargs -n3

a b c
d e f
g h i
```

2）-d 选项可以自定义一个定界符：

```sh
echo "nameXnameXnameXname" | xargs -dX -n2

name name
name name
```

3）复制所有图片文件到 /data/images 目录下：

```
ls *.jpg | xargs -n1 -I {} cp {} /data/images
```

xargs 的一个选项 -I，使用 -I 指定一个替换字符串 {}，这个字符串在 xargs 扩展时会被替换掉，当 -I 与 xargs 结合使用，每一个参数命令都会被执行一次：

4）查找所有的 jpg 文件，并且压缩它们：

```sh
find . -type f -name "*.jpg" -print | xargs tar -czvf images.tar.gz
```

#### 2.3 sed

sed是非交互式的编辑器。它不会修改文件，除非使用shell重定向来保存结果。默认情况下，所有的输出行都被打印到屏幕上。

sed编辑器逐行处理文件（或输入），并将结果发送到屏幕。具体过程如下：首先sed把当前正在处理的行保存在一个临时缓存区中（也称为模式空间），然后处理临时缓冲区中的行，完成后把该行发送到屏幕上。sed每处理完一行就将其从临时缓冲区删除，然后将下一行读入，进行处理和显示。处理完输入文件的最后一行后，sed便结束运行。sed把每一行都存在临时缓冲区中，对这个副本进行编辑，所以不会修改原文件。

**语法：** `sed [option]... {script} [input-file] `

**选项说明：**

- -n ：使用安静(silent)模式。在一般 sed 的用法中，所有来自 STDIN 的数据一般都会被列出到终端上。但如果加上 -n 参数后，则只有经过sed 特殊处理的那一行(或者动作)才会被列出来。
- -e ：直接在命令列模式上进行 sed 的动作编辑。
- -f ：直接将 sed 的动作写在一个文件内， -f filename 则可以运行 filename 内的 sed 动作。
- -r ：sed 的动作支持的是延伸型正规表示法的语法。(默认是基础正规表示法语法)。
- -i ：直接修改读取的文件内容，而不是输出到终端。

**动作说明：**

- a ：新增， a 的后面可以接字串，而这些字串会在新的一行出现(目前的下一行)。

- c ：取代， c 的后面可以接字串，这些字串可以取代 n1,n2 之间的行。

- d ：删除行。

- i  ：插入， i 的后面可以接字串，而这些字串会在新的一行出现(目前的上一行)。

- p ：打印，亦即将某个选择的数据印出。通常 p 会与参数 `sed -n` 配合使用。

- s ：取代，可以直接进行取代的工作，通常搭配正规表示法使用。

- q ：结束或退出sed。


**实例说明：**

1. 以行为单位的新增/删除

   ```sh
   # 将 /etc/passwd文件的内容列出并且列印行号，同时，请将第 2~5 行删除！
   nl /etc/passwd | sed '2,5d'
   
   # 删除第 3 到最后一行
   nl /etc/passwd | sed '3,$d'
   
   # 在第二行后(亦即是加在第三行)加上"drink tea?"
   nl /etc/passwd | sed '2a drink tea'
   
   # 在第二行前插入"drink tea?"
   nl /etc/passwd | sed '2i drink tea' 
   ```

2. 以行为单位的替换与显示

   ```shell
   # 将第2-5行的内容替换为"No. 2-5 rows"
   nl /etc/passwd | sed '2,5c No. 2-5 rows'
   
   # 仅列出 /etc/passwd 文件内的第 5-7 行
   nl /etc/passwd | sed -n '5,7p'
   ```

3. 匹配行的查找、显示与删除

   ```shell
   # 搜索 /etc/passwd 有root关键字的行
   nl /etc/passwd | sed -n '/root/p'
   
   # 删除/etc/passwd所有包含root的行，其他行输出
   nl /etc/passwd | sed  '/root/d'
   ```

4. 文本替换

   ```sh
   # 将/etc/passwd中所有root替换为father，并显示
   # 格式： 's/要被取代的字串/新的字串/'
   nl /etc/passwd | sed 's/root/father/'
   ```

#### 



### 3. Linux 命令行快捷键

| 快捷键   | 快捷键说明                                      |
| :------- | :---------------------------------------------- |
| Ctrl + A | 将光标移到行首                                  |
| Ctrl + E | 将光标移动到行尾                                |
| Ctrl + R | 回溯搜索(Backwards search)history缓冲区内的文本 |
| Ctrl + L | 进行清屏操作                                    |
| Ctrl + U | 删除当前光标前面的所有文字（还有剪切功能）      |
| Ctrl + K | 删除当前光标后面的所有文字（还有剪切功能）      |
| Ctrl + Y | 粘贴Ctrl + U或Ctrl + K剪切的内容                |
| Alt + B  | 往回(左)移动一个单词                            |
| Alt + F  | 往后(右)移动一个单词                            |





### /dev/null 文件

如果希望执行某个命令，但又不希望在屏幕上显示输出结果，那么可以将输出重定向到 /dev/null：

```
$ command > /dev/null
```

/dev/null 是一个特殊的文件，写入到它的内容都会被丢弃；如果尝试从该文件读取内容，那么什么也读不到。但是 /dev/null 文件非常有用，将命令的输出重定向到它，会起到"禁止输出"的效果。

如果希望屏蔽 stdout 和 stderr，可以这样写：

```
$ command > /dev/null 2>&1
```

> **注意：**0 是标准输入（STDIN），1 是标准输出（STDOUT），2 是标准错误输出（STDERR）。
>
> 这里的 **2** 和 **>** 之间不可以有空格，**2>** 是一体的时候才表示错误输出。