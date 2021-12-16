## CMake实践

### 1. 一个例子，快速上手cmake

项目目录结构：

```
├── CMakeLists.txt
├── include
│   └── static
│       └── Hello.h
└── src
    ├── Hello.cpp
    └── main.cpp
```

`CMakeLists.txt`的内容注释版：

```cmake
# 设置最低要求的cmake版本
cmake_minimum_required(VERSION 3.20)

# 设置工程名和版本
project(hello_cmake VERSION 1.0)

# 指定使用C++11的版本
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# 设置编译选项
add_compile_options(-Wno-unused-variable -Wno-unused-function -fno-builtin)

# #如果没有指定则设置默认编译方式为RelWithDebInfo
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message("Setting build type to 'RelWithDebInfo' as none was specified.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

# 生成静态库, 添加头文件查找目录
add_library(hello_library STATIC src/Hello.cpp)
target_include_directories(hello_library PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 设置可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}) 

# 生成可执行文件, 链接可执行文件和静态库
add_executable(hello_binary src/main.cpp)
target_link_libraries(hello_binary PRIVATE hello_library)
```

cmake函数讲解：

- `add_library`函数用于从某些源文件创建一个库，默认生成在构建文件夹。使用`STATIC`关键字生成静态库，使用`SHARED`关键字生成动态库。

  ```cmake
  add_library(hello_library SHARED src/Hello.cpp)
  ```

- `target_include_directories`用于添加目标所包含头文件的目录。可选熟悉PRIVATE、INTERFACE和PUBLIC，三者的作用如下：
  - PRIVATE - 目录仅被添加到目标（库）的包含路径中。
  - INTERFACE - 目录没有被添加到目标（库）的包含路径中，而是添加到了链接这个库的其他目标（库或者可执行程序）的包含路径中。
  - PUBLIC - 目录既被添加到目标（库）的包含路径中，同时添加到了链接了这个库的其他目标（库或者可执行程序）的包含路径中。
- `target_link_libraries`用于添加目标所依赖的动态库或静态库，scope关键字PRIVATE、INTERFACE和PUBLIC的意义与上面类似。PRIVATE表示目标所链接的库不会暴露出去，而PUBLIC表示链接目标时，目标所链接的库也会被链接进去。

- `CMAKE_BUILD_TYPE`为cmake的内置变量，用于控制编译类型（代码优化等级和是否包含调试信息等），取值有以下几个：
  - Release - 添加`-o3 -DNDEBUG`编译选项，即不可调试、代码优化高，文件体积小，常用于发行版本。
  - Debug - 添加`-g`编译选项，可调试但文件体积大。
  - MinSizeRel - 添加`-Os -DNDEBUG`编译选项，最小体积版本。
  - RelWithDebInfo - 添加`-O2 -g -DNDEBUG`，有代码优化同时可调试。

### 2. cmake变量

CMake中变量的类型有多种：通过set设置的普通变量和缓存变量、环境变量、数组变量等等，由于CMake在生成的过程中会加载缓存的关系，因此用法不一样。

常用的变量设置语法：

```
set(<variable> <value>
    [[CACHE <type> <docstring> [FORCE]] | PARENT_SCOPE])
```

#### 2.1 普通变量

普通变量（normal variable）相当于编程中脚本内部变量，类似于脚本文件的局部变量，这种变量不能跨越CMakeLists.txt文档。普通变量定义方式如下：

```
set(var "value")
```

设置一个普通变量var，值为value。

和编程语言中局部变量的用法类似，这个变量会屏蔽CMake缓存中的同名变量，（类似局部变量屏蔽全局变量）。但是这条语句不会改变缓存中的var变量。

#### 2.2 缓存变量

cache variable用于缓存变量，定义如下：

```
set(var "value" CACHE STRING "" FORCE)
```

这条语句设置了一个CACHE语句，类型是STRING，说明信息为空字符串，上述都不能省略。

**还有一种方法能够设置CACHE变量，就是通过cmake命令的-D选项，可以添加一个CACHE变量。**

CACHE作用如下：

- 如果缓存中存在同名的变量，根据FORCE来决定是否写入缓存：如果没有FORCE，这条语句不起作用，使用缓存中的变量；如果有FORCE，使用当前设置的值。
  - 注意，如果是FORCE，也能修改-D选项设置的CACHE变量，所以有可能传入的生成命令选项是无效的。
- 如果缓存中不存在同名的变量，则将这个变量写入缓存并使用。

缓存变量也可以设置只在本文件内生效，将STRING类型改为INTERNAL即可。

#### 2.3 环境变量

- 读取环境变量：`$ENV{...}`
- 设置环境变量：`set(ENV{...} ...)`

```cmake
MESSAGE(STATUS “HOME dir: $ENV{HOME}”)
```

#### 2.4 cmake常用变量

-  EXECUTABLE_OUTPUT_PATH ：指定可执行文件的生成位置。

- LIBRARY_OUTPUT_PATH ：指定库文件的生成位置。

- CMAKE_BINARY_DIR和PROJECT_BINARY_DIR ：工程编译发生的目录。

- CMAKE_SOURCE_DIR和PROJECT_SOURCE_DIR ：工程顶层目录。

- CMAKE_CURRENT_SOURCE_DIR ：当前处理的 CMakeLists.txt 所在的路径。

- CMAKE_CURRRENT_BINARY_DIR ：如果是 in-source 编译，它跟 CMAKE_CURRENT_SOURCE_DIR 一致，如果是 out-of-source 编译，他指的是 target 编译目录。

- CMAKE_MODULE_PATH ：用来定义自己的 cmake 模块所在的路径。如果你的工程比较复杂，有可能会自己编写一些 cmake 模块，这些 cmake 模块是随你的工程发布的，为了让 cmake 在处理CMakeLists.txt 时找到这些模块，你需要通过 SET 指令，将自己的 cmake 模块路径设置一下。这时候你就可以通过 INCLUDE 指令来调用自己的模块了。

  ```cmake
  SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
  ```

- PROJECT_NAME ：返回通过 PROJECT 指令定义的项目名称。


#### 2.5 总结

正常使用的时候，如果有多层CMakeLists.txt，需要跨文本的变量，应该使用CACHE类型，如果只是当前文本的变量，则不需要使用CACHE，更重要的是，应该避免使用同名的普通和缓存变量。另外，由于CMake没有有效的清除缓存的方法，如果要彻底清除缓存，需要删除build或者release文件夹的所有文件。

### 3. 包含第三方库

#### 3.1 方法一：通过link_directories和include_directories设置库文件和头文件的路径

```cmake
include_directories(path_to_OpenBLAS/include/) # 头文件的路径
link_directories(path_to_OpenBLAS/lib/) # .a文件的路径
add_executable(Test_lib library.c)
target_link_libraries(Test_lib libopenblas.a)
```

#### 3.2 方法二：使用find_package自动查找库文件和头文件的路径

```cmake
add_executable(Test_lib library.c)
set(OpenBLAS_DIR "/path to .camke文件")
find_package(OpenBLAS REQUIRED)
include_directories(${OpenBLAS_INCLUDE_DIRS})
target_link_libraries(Test_lib ${OpenBLAS_LIBRARIES})
```

.cmake一般会由第三方库自动生成，去安装路径寻找。里面包含的是库文件和头文件的路径设置。

为了方便我们在项目中引入外部依赖包，cmake官方为我们预定义了许多寻找依赖包的Module，他们存储在`path_to_your_cmake/share/cmake-<version>/Modules`目录下。每个以`Find<LibaryName>.cmake`命名的文件都可以帮我们找到一个包。找到对应的包后，cmake会给我们提供以下几个变量：

- XX_FOUND - 系统是否有XX库；
- XX_INCULDE_DIR - XX库的头文件目录；
- XX_LIBRARIES - XX库的库文件目录；
- XX_DEFINITIONS - XX库需要的编译选项。

如果cmake里没有对应库的`Find<LibaryName>.cmake`文件，我们也可以自己写一个。

```cmake
#  JSONC_FOUND - System has json-c
#  JSONC_INCLUDE_DIRS - The json-c include directories
#  JSONC_LIBRARIES - The libraries needed to use json-c
#  JSONC_DEFINITIONS - Compiler switches required for using json-c

find_package(PkgConfig)
pkg_check_modules(PC_JSONC QUIET json-c)
set(JSONC_DEFINITIONS ${PC_JSONC_CFLAGS_OTHER})

find_path(JSONC_INCLUDE_DIR json.h
          HINTS ${PC_JSONC_INCLUDEDIR} ${PC_JSONC_INCLUDE_DIRS}
          PATH_SUFFIXES json-c)

find_library(JSONC_LIBRARY NAMES json-c libjson-c
             HINTS ${PC_JSONC_LIBDIR} ${PC_JSONC_LIBRARY_DIRS})

find_library(JSONC_LIBRARY NAMES json-c libjson-c
             HINTS ${PC_JSON-C_LIBDIR} ${PC_JSON-C_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set JSONC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(JSONC DEFAULT_MSG
                                  JSONC_LIBRARY JSONC_INCLUDE_DIR)

if (NOT JSONC_FOUND)

  message(FATAL_ERROR "Json-C is Required!\n")

endif (NOT JSONC_FOUND)

mark_as_advanced(JSONC_INCLUDE_DIR JSONC_LIBRARY)

set(JSONC_LIBRARIES    ${JSONC_LIBRARY})
set(JSONC_INCLUDE_DIRS ${JSONC_INCLUDE_DIR})
```

#### 3.3 方法三：通过find_library直接设置库文件和头文件的路径

```cmake
include_directories(/path to OpenBLAS/include/)
find_library(OPENBLAS NAMES OpenBLAS PATHS /path to OpenBLAS/lib/ NO_DEFAULT_PATH)
add_executable(Test_lib library.c)
target_link_libraries(Test_lib ${OPENBLAS})
```

`find_library`用于查找库的位置，其用法如下：

```cmake
find_library(var NAMES name1 PATHS path [REQUIRED] [NO_DEFUALT_PATH])
```

- NAMES用于指定库可能的名字；
- HINTS和PATHS作用相同，用于指定查找库的路径；
- REQUIRED说明这个库是必须的，没有找到则产生一个错误信息；
- NO_DEFUALT_PATH用于说明除了指定的路径外，不再查找额外的路径。

还有其他的关键字，可以查看官方的说明文档。

#### 3.4 编写自己的find模块文件



### 4. 多层次cmake

CMakeLists.txt文件可以通过`add_subdirectory`包含和调用包含CMakeLists.txt文件的子目录。

```cmake
add_subdirectory(sublibrary1)
add_subdirectory(sublibrary2)
add_subdirectory(subbinary)
```

使用project（）命令创建项目时，CMake将自动创建许多变量，这些变量可用于引用有关该项目的详细信息。 这些变量然后可以由其他子项目或主项目使用。 例如，要引用您可以使用的其他项目的源目录。

- PROJECT_NAME：当前project函数设置的项目名称；
- CMAKE_PROJECT_NAME：有project设置的第一个项目的名称，即顶层项目；
- PROJECT_SOURCE_DIR：当前项目的源文件目录；
- PROJECT_BINARY_DIR：当前项目的构建目录。



### 5. 项目安装（install）



### 6. 代码生成

代码生成是一个非常有用的功能，它可以使用一份公共的描述文件，生成不同语言下的源代码。这个功能使得需要人工编写的代码大幅减少，同时也增加了互操作性。

我们通过两个示例来展示如何使用CMake变量和其他常用的工具进行代码生成。

#### 6.1 configure-file

在CMake中，我们可以使用`configure-file()`函数进行文件中变量的替换，其输入参数为源文件和目标文件。

```cmake
configure_file(ver.h.in ${PROJECT_BINARY_DIR}/ver.h)
configure_file(path.h.in ${PROJECT_BINARY_DIR}/path.h @ONLY)
```

```c++
// ver.h.in
#ifndef __VER_H__
#define __VER_H__

// version variable that will be substituted by cmake
// This shows an example using the $ variable type
const char* ver = "${cf_example_VERSION}";

#endif
```

```c++
// path.h.in
#ifndef __PATH_H__
#define __PATH_H__

// version variable that will be substituted by cmake
// This shows an example using the @ variable type
const char* path = "@CMAKE_SOURCE_DIR@";

#endif
```

第一个例子，在`ver.h.in`文件中，CMake可以将使用 `${}` 或 `@@` 的语法来定义一个CMake变量。在执行代码生成之后，在 `PROJECT_BINARY_DIR` 目录下将会出现一个新的ver.h文件。

第二个例子，在`path.h.in`文件中，`@ONLY` 指定了它只能用 `@@` 的语法来定义一个CMake变量。同样地，在执行代码生成之后，在 `PROJECT_BINARY_DIR` 目录下将会出现一个新的path.h文件。

在程序中，我们只需要包含生成的头文件，就能使用文件中定义的变量或宏了。

```c++
#include <iostream>
#include "ver.h"
#include "path.h"

int main(int argc, char *argv[])
{
    std::cout << "Hello Version " << ver << "!" << std::endl;
    std::cout << "Path is " << path << std::endl;
    return 0;
}
```

完整的Cmake文件见链接：[cmake-examples/CMakeLists.txt](https://github.com/ttroy50/cmake-examples/blob/master/03-code-generation/configure-files/CMakeLists.txt)

#### 6.2 使用Protobuf的代码生成

Protocol Buffers 是一种由谷歌提出的数据序列化的格式。用户可以提供一个描述了数据的 `.proto` 格式的文件，通过protobuf编译器，文件可以被编译成包括C++在内的一系列编程语言的源码文件。

本示例要求预安装protocol buffers的二进制文件和库文件，在Ubuntu系统中，可以使用下述命令安装：

```sh
sudo apt-get install protobuf-compiler libprotobuf-dev
```

CMake中，使用`find_package()`命令查找并导入protobuf包，这样我们可以使用以下变量获取protobuf的信息：

- `PROTOBUF_FOUND` - Protocol Buffers是否安装
- `PROTOBUF_INCLUDE_DIRS` - protobuf头文件路径
- `PROTOBUF_LIBRARIES` - protobuf库文件路径

CMake protobuf包中的`PROTOBUF_GENERATE_CPP`可以帮我们简化源代码生成流程：

```cmake
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS AddressBook.proto)
```

PROTO_SRC和OTO_HARS变量分别指代生成的cpp和h文件并可用于连接到target和设置include。

当`.proto`文件被改变时，与其相关联的源代码文件也将被自动重新生成；如果`.proto`文件没有发生修改，重新执行 `make` 命令，并不会重新生成。

完整的Cmake文件见链接：[cmake-examples/CMakeLists.txt](https://github.com/ttroy50/cmake-examples/blob/master/03-code-generation/protobuf/CMakeLists.txt)

#### 6.3 通用的代码生成方法

我们可以使用`add_custom_target`和`add_custom_command`命令来定制自己的代码生成命令。这里还是以protobuf举例，但不会使用CMake protobuf包中相关命令。

```cmake
find_package(Protobuf)

#获取需要编译的proto文件
file(GLOB_RECURSE MSG_PROTOS ${CMAKE_SOURCE_DIR}/*.proto)
set(MESSAGE_SRC "")
set(MESSAGE_HDRS "")

foreach(msg ${MSG_PROTOS})
    get_filename_component(FIL_WE ${msg} NAME_WE)

    list(APPEND MESSAGE_SRC "${PROJECT_BINARY_DIR}/${FIL_WE}.pb.cc")
    list(APPEND MESSAGE_HDRS "${PROJECT_BINARY_DIR}/${FIL_WE}.pb.h")

    # 使用自定义命令
    add_custom_command(
        OUTPUT "${PROJECT_BINARY_DIR}/${FIL_WE}.pb.cc"
        "${PROJECT_BINARY_DIR}/${FIL_WE}.pb.h"
        COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --cpp_out  ${PROTO_META_BASE_DIR}
        -I ${CMAKE_BINARY_DIR}
        ${msg}
        DEPENDS ${msg}
        COMMENT "Running C++ protocol buffer compiler on ${msg}"
        VERBATIM
    )
endforeach()

# 设置文件属性为 GENERATED
set_source_files_properties(${MESSAGE_SRC} ${MESSAGE_HDRS} PROPERTIES GENERATED TRUE)

# 添加自定义target
add_custom_target(generate_message ALL
                DEPENDS ${MESSAGE_SRC} ${MESSAGE_HDRS}
                COMMENT "generate message target"
                VERBATIM
                )
                
# Add an executable
add_executable(protobuf_example main.cpp ${MESSAGE_SRC} ${MESSAGE_HDRS})

add_dependencies(protobuf_example generate_message)

target_include_directories(protobuf_example
    PUBLIC
    ${PROTOBUF_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# link the exe against the libraries
target_link_libraries(protobuf_example PUBLIC ${PROTOBUF_LIBRARIES})
```

我们通过`add_custom_command`命令说明了proto描述文件有变化时，运行命令输出生成proto头文件和源文件。在通过`add_custom_target`命令定义generate_message目标，它的运行依赖于生成文件的变化。最后再使用`add_dependencies`定义依赖生成文件的目标，这样就能够在运行protobuf_example目标前，先检查并运行generate_message。

需要注意的几点：

- 设置生成的源码文件属性GENERATED为TRUE,否则cmake时会因找不到源码而报错
- 使用**add_custom_target**添加目标时要设置ALL关键字,否则target将不在默认编译列表中

### 7. 获取Git库版本信息

#### Git相关命令

获取commit hash值：`git log -1 --pretty=format:%H`

获取当前的Tag：`git describe --abbrev=6 --dirty --always --tags`

#### cmake脚本

```cmake
find_package(Git)
set(GIT_REPO_VERSION "git not found")
set(GIT_REPO_DATE "git not found")
set(GIT_REPO_HASH "git not found")
if(GIT_FOUND)
    # 获取当前版本的Tag
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --abbrev=6 --dirty --always --tags
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  GIT_REPO_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # 获取最新 commit 日期，YYYY-MM-DD
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  GIT_REPO_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # 获取最新 commit Hash
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE  GIT_REPO_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    message(STATUS "Git version is ${GIT_REPO_VERSION}: ${GIT_REPO_DATE} : ${GIT_REPO_HASH}")
endif(GIT_FOUND)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ver.h.ini
  ${CMAKE_BINARY_DIR}/gen/ver.h
  @ONLY
)

include_directories(${CMAKE_BINARY_DIR}/gen)
```

#### 配置文件

建立一个配置文件ver.h.ini.此处放在CMakeLists.txt目录下。

```c++
#ifndef _GIT_VER_
#define _GIT_VER_

#define GIT_REPO_TAG "@GIT_REPO_VERSION@"
#define GIT_REPO_DATE "@GIT_REPO_DATE@"
#define GIT_REPO_HASH "@GIT_REPO_HASH@"

#endif
```

cmake的configure_file命令会根据输入的配置文件，替换对应的变量，生成输出文件。

#### 使用生成的头文件

因为我们已经使用include_directories包含了ver.h的目录，后面直接include头文件就好。

```c++
#include <stdio.h>
#include "ver.h"

int main(int argc, char** argv)
{
    printf("version: %s\n", GIT_REPO_TAG);
    printf("date: %s\n", GIT_REPO_DATE);
    printf("hash: %s\n", GIT_REPO_HASH);
    return 0;
}
```

### 8. 动态库添加版本号

按照规则，动态库是应该包含一个版本号的，我们可以看一下系统的动态库，一般情况是：

```
libhello.so.1.2
libhello.so -> libhello.so.1
libhello.so.1 -> libhello.so.1.2
```

为了实现动态库版本号，我们可以使用 SET_TARGET_PROPERTIES 指令。
具体使用方法如下：

```cmake
SET_TARGET_PROPERTIES(hello PROPERTIES VERSION 1.2 SOVERSION 1)
```

VERSION 指代动态库版本，SOVERSION 指代 API 版本。
将上述指令加入 lib/CMakeLists.txt 中，重新构建看看结果，在指定的库生成目录会生成：

```
libhello.so.1.2
libhello.so.1->libhello.so.1.2
libhello.so ->libhello.so.1
```



### 附录

#### 1) target_include_directories、include_directories的区别

include_directories的影响范围最大，可以为CMakelists.txt后的所有项目添加头文件目录，一般写在最外层CMakelists.txt中影响全局。

target_include_directories的影响范围可以自定义。如加关键子PRIVATE或这PUBLIC。**一般引用库路径使用这个命令**，作为外部依赖项引入进来，target是自己项目生成的lib。

```cmake
include_directories(include/ffmpeg)

project(myLib)
target_include_directories(myLib PRIVATE ${OpenCV_Include_dir})
```

`link_directories`和`target_link_directories`同理。

#### 2) 设置编译选项的讲究

在cmake脚本中，设置编译选项可以通过`add_compile_options`命令，也可以通过set命令修改`CMAKE_CXX_FLAGS`或`CMAKE_C_FLAGS`。

使用这两种方式在有的情况下效果是一样的，但请注意它们还是有区别的：`add_compile_options`命令添加的编译选项是针对所有编译器的(包括c和c++编译器)，而set命令设置`CMAKE_C_FLAGS`或`CMAKE_CXX_FLAGS`变量则是分别只针对c和c++编译器的。

使用`add_compile_options`添加`-std=c++11`选项，是想在编译c++代码时加上c++11支持选项。但是因为`add_compile_options`是针对所有类型编译器的，所以在编译c代码时，就会产生如下warning。

```cmake
#判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")   
endif(CMAKE_COMPILER_IS_GNUCXX)
```

此外，还有`add_definitions`也可以用于添加编译选项，但实际使用过程中，常用它来添加`-D`的宏定义选项。这样我们就不需要修改源码，就能通过编译选项来控制源码中宏的开闭。

```cmake
# cmake .. -DTEST_DEBUG=ON
option(TEST_DEBUG "option for debug" OFF)
if (TEST_DEBUG) 
	add_definitions(-DTEST_DEBUG)
endif(TEST_DEBUG)
```

#### 3) add_dependencies

定义 target 依赖的其他 target ,确保在编译本 target 之前,其他的 target 已经被构建。

```cmake
add_dependencies(target-name depend-target1 depend-target2 ...)
```

#### 4) add_test

**ENABLE_TESTING**
指令用来控制 Makefile 是否构建 test 目标,涉及工程所有目录。语法很简单,没有任何参数, ENABLE_TESTING() ,一般情况这个指令放在工程的主CMakeLists.txt 中 .

**ADD_TEST **

```
ADD_TEST(testname Exename arg1 arg2 ...)
```

- testname 是自定义的 test 名称,
- Exename 可以是构建的目标文件也可以是外部脚本等等。
- 后面连接传递给可执行文件的参数。如果没有在同一个 CMakeLists.txt 中打开ENABLE_TESTING() 指令,任何 ADD_TEST 都是无效的。
  比如我们前面的 Helloworld 栗子,可以在工程主 CMakeLists.txt 中添加

```
ADD_TEST(mytest ${PROJECT_BINARY_DIR}/bin/main)
ENABLE_TESTING()
```

生成 Makefile 后,就可以运行 make test 来执行测试了。

#### 5) file指令

文件操作指令,基本语法为 :

```cmake
FILE(WRITE filename "message to write"... )
FILE(APPEND filename "message to write"... )
FILE(READ filename variable)
FILE(GLOB variable [RELATIVE path] [globbingexpressions]...)
FILE(GLOB_RECURSE variable [RELATIVE path] [globbing expressions]...)
FILE(REMOVE [directory]...)
FILE(REMOVE_RECURSE [directory]...)
FILE(MAKE_DIRECTORY [directory]...)
FILE(RELATIVE_PATH variable directory file)
FILE(TO_CMAKE_PATH path result)
FILE(TO_NATIVE_PATH path result)
```

这里的语法都比较简单,不在展开介绍了。

#### 6) foreach

FOREACH 指令的使用方法有三种形式:

#### 1) 列表

```cmake
FOREACH(loop_var arg1 arg2 ...)
COMMAND1(ARGS ...)
COMMAND2(ARGS ...)
...
ENDFOREACH(loop_var)
```

像我们前面使用的 AUX_SOURCE_DIRECTORY 的栗子

```cmake
AUX_SOURCE_DIRECTORY(. SRC_LIST)
FOREACH(F ${SRC_LIST})
MESSAGE(${F})
ENDFOREACH(F)
```

#### 2 )范围

```cmake
FOREACH(loop_var RANGE total)
ENDFOREACH(loop_var)从 0 到 total 以1为步进
```

举例如下:

```cmake
FOREACH(VAR RANGE 10)
MESSAGE(${VAR})
ENDFOREACH(VAR)
```

最终得到的输出是:
0
1
2
3
4
5
6
7
8
9
10

#### 3)范围和步进

```cmake
FOREACH(loop_var RANGE start stop [step])
ENDFOREACH(loop_var)
```

从 start 开始到 stop 结束,以 step 为步进,
举例如下

```cmake
FOREACH(A RANGE 5 15 3)
MESSAGE(${A})
ENDFOREACH(A)
```

最终得到的结果是:
5
8
11
14
注：整个FOREACH遇到 ENDFOREACH 指令,整个语句块才会得到真正的执行。

#### 7) macro

宏定义如下：

```cmake
macro(<name> [arg1 [arg2 [arg3 ...]]])
  COMMAND1(ARGS ...)
  COMMAND2(ARGS ...)
  ...
endmacro(<name>)
```

- <name><name>为函数名字
- arg1、arg2...为函数参数

举个栗子：

```cmake
set(var "ABC")

macro(Moo arg)
  message("arg = ${arg}")
  set(arg "abc")
  message("# After change the value of arg.")
  message("arg = ${arg}")
endmacro()
message("=== Call macro ===")
Moo(${var})

#输出如下：
=== Call macro ===
arg = ABC
# After change the value of arg.
arg = ABC
```

这里的宏是做了字符串的替换

