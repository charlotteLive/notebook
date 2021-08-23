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

设置一个普通变量var，值为value，引号的作用可以详见我的另一篇文章。

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


#### 2.4 总结

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

### 5. 代码生成





### 6. 附录

#### 6.1 target_include_directories、include_directories的区别

include_directories的影响范围最大，可以为CMakelists.txt后的所有项目添加头文件目录，一般写在最外层CMakelists.txt中影响全局。

target_include_directories的影响范围可以自定义。如加关键子PRIVATE或这PUBLIC。**一般引用库路径使用这个命令**，作为外部依赖项引入进来，target是自己项目生成的lib。

```cmake
include_directories(include/ffmpeg)

project(myLib)
target_include_directories(myLib PRIVATE ${OpenCV_Include_dir})
```

`link_directories`和`target_link_directories`同理。

#### 6.2 设置编译选项的讲究

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



