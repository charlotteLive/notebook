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

# #如果没有指定则设置默认编译方式为RelWithDebInfo
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message("Setting build type to 'RelWithDebInfo' as none was specified.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

# 生成静态库, 添加头文件查找目录
add_library(hello_library STATIC src/Hello.cpp)
target_include_directories(hello_library PUBLIC ${PROJECT_SOURCE_DIR}/include)

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

