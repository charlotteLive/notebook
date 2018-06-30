## Makefile学习

### 1. g++编译选项

 gcc/g++ 在执行编译时，需要4步：

1. 预处理,生成.i的文件[使用-E参数]
2. 将预处理后的文件不转换成汇编语言,生成文件.s[使用-S参数]
3. 有汇编变为目标代码(机器代码)生成.o的文件[使用-c参数]
4. 连接目标代码,生成可执行程序[使用-o参数]

**常用编译选项：**

- -c ：只激活预处理、编译和汇编，最后生成的文件为obj文件。
- -S ：只激活预处理和编译，只将文件编译成汇编代码，生成s汇编代码。
- -E ：只激活预处理，并不生成文件，需要将输出重定向到一个输出文件里面查看。
- -o ：指定目标名称，缺省时编译出来的文件时`a.out`。
- -include file ：包含某个代码,简单来说,就是便于某个文件需要另一个文件的时候,就能用他设
  定,功能就相当于在代码中使用`＃include`
- -O0、O1、O2、O3 ：指定编译器优化选项的级别，`-O0`表示没有优化，`-O1`为缺省值。
- -g ：指示编译器在编译时产生调试信息。
- -static ：此选项将禁止使用动态库，所以，编译出来的东西，一般都非常大，也不必什么
  动态连接库，就能运行.
- -shared ：此选项用于编译生成动态链接库，所以生成文件比较小，不过需要系统有动态库。
- -Wall ：显示所有警告信息(warning all)。


### 2. Makefile规则

在软件开发中，make通常被视为一种软件构建工具。该工具主要经由读取一种名为“makefile”或“Makefile”的文件来实现软件的自动化建构。它会通过一种被称之为“target”概念来检查相关文件之间的依赖关系，这种依赖关系的检查系统非常简单，主要通过对比文件的修改时间来实现。在大多数情况下，我们主要用它来编译源代码，生成结果代码，然后把结果代码连接起来生成可执行文件或者库文件。

#### 2.1 Makefile基本规则

首先让我们来了解下Makefile的基本规则：

```makefile
target ... : prerequisites ...
    command
    ...
    ...
```

target是一个目标文件，可以是Object File，也可以是执行文件，还可以是一个标签（Label），即一个伪目标。prerequisites是要生成那个target所需要的文件或是目标。command是make需要执行的shell命令。

这是一个文件的依赖关系，也就是说，target这一个或多个的目标文件依赖于prerequisites中的文件，其生成规则定义在command中。说白一点就是说，prerequisites中如果有一个以上的文件比target文件要新的话，command所定义的命令就会被执行。这就是Makefile的规则。也就是Makefile中最核心的内容。

接下来我们从下面的示例来了解Makefile的其他规则：

```makefile
# Makefile for shared library.
prog = edit
obj = main.o command.o display.o search.o files.o

$(prog) : $(obj)
	gcc -o $(prog) $(obj)

main.o : main.c defs.h
	cc -c main.c
command.o : command.c defs.h command.h
	cc -c command.c
display.o : display.c defs.h buffer.h
	cc -c display.c
search.o : search.c defs.h buffer.h
	cc -c search.c
files.o : files.c defs.h buffer.h command.h
	cc -c files.c
	
.PHONY : clean
clean :
	-rm edit $(obj)
```

>  请注意，在第二行的“gcc”命令之前必须要有一个**tab缩进**。语法规定Makefile中的任何命令之前都必须要有一个tab缩进，否则make就会报错。

**1） 首先，我们了解下Makefile的工作方式**：

- 当我们输入make命令时，make会在当前目录下寻找名字为"Makefile"或"makefile"的文件；
- 如果找到，它会找文件中的第一个目标文件（target），在上面的例子中，他会找到“edit”这个文件，并把这个文件作为最终的目标文件。
- 如果edit文件不存在，或是edit所依赖的后面的 .o 文件的文件修改时间要比edit这个文件新，那么，他就会执行后面所定义的命令来生成edit这个文件。
- 如果edit所依赖的.o文件也存在，那么make会在当前文件中找目标为.o文件的依赖性，如果找到则再根据那一个规则生成.o文件。
- 当然，你的C文件和H文件是存在的啦，于是make会生成 .o 文件，然后再用 .o 文件生命make的终极任务，也就是执行文件edit了。

这就是整个make的依赖性，make会一层又一层地去找文件的依赖关系，直到最终编译出第一个目标文件。在找寻的过程中，如果出现错误，比如最后被依赖的文件找不到，那么make就会直接退出，并报错，而对于所定义的命令的错误，或是编译不成功，make根本不理。make只管文件的依赖性，如果在我找了依赖关系之后，冒号后面的文件还是不在，则make停止工作。

**2）makefile中使用变量**

Makefile中的变量即字符串替换。在上面的Makefile中，我们使用了`obj = main.o ...`带定义了一个Makefile变量，后续使用`$(obj)`即可使用相应的字符串代替变量使用的地方，避免重复。

**3）make自动推导功能**

make可以自动推导文件及文件依赖关系后面的命令、只要make看到.o文件，就会自动地把.c文件加载依赖关系中，所以可以省略依赖关系中同名的.c文件。如：

```makefile
main.o : defs.h
	cc -c main.c
```

**4）清空目标文件的规则**

每个Makefile中都应该写一个清空目标文件（.o和执行文件）的规则，这不仅便于重新编译，也有利于文件的清洁。一般的风格是：

```makefile
.PHONY : clean
clean :
	-rm edit $(obj)
```

`.PHONY`表示clean是一个伪目标，而在rm命令前面加一个小减号的意思是某些文件出现问题时不要管它，继续做后面的事。

**5） 在规则中使用通配符**

如果我们想定义一系列比较类似的文件，我们很自然地就想起使用通配符。make 支持三个通配符 ：`*`、`?`、`[]`。

```makefile
clean: 
	rm -f *.o
```

**6） 文件搜寻**





### 3. 常用Makefile模板

#### 3.1 编译动态链接库

```makefile
############################################################# 
# Makefile for shared library.
# 编译动态链接库
#############################################################
#set your own environment option
CC = g++
CC_FLAG = -D_NOMNG -D_FILELINE

#set your inc and lib
INC = 
LIB = -lpthread -L./ -lsvrtool

#make target lib and relevant obj 
PRG = libsvrtool.so
OBJ = Log.o

#all target
all:$(PRG)

$(PRG):$(OBJ)
	$(CC) -shared -o $@ $(OBJ) $(LIB)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......;
	rm -f $(OBJ) $(PRG)
```

#### 3.2 编译静态链接库

```makefile
#############################################################
# Makefile for static library.
# 编译静态链接库
#############################################################
#set your own environment option
CC = g++
CC_FLAG = -D_NOMNG -D_FILELINE

#static library use 'ar' command 
AR = ar

#set your inc and lib
INC = 
LIB = -lpthread -L./ -lsvrtool

#make target lib and relevant obj 
PRG = libsvrtool.a
OBJ = Log.o

#all target
all:$(PRG)
$(PRG):$(OBJ)
	${AR} rv ${PRG} $?

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
```

#### 3.3 编译可执行程序

```makefile
###########################################
#Makefile for simple programs
###########################################
INC=
LIB= -lpthread

CC=CC
CC_FLAG=-Wall

PRG=threadpooltest
OBJ=CThreadManage.o CThreadPool.o CThread.o CWorkerThread.o threadpooltest.o

$(PRG):$(OBJ)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)
	
.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
```

