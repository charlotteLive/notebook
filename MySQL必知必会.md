## MySQL必知必会

### 1. 数据库基础

表是数据库中的文件，用来存储某种特定类型的数据。表本身由行和列组成，列表示表中的一个字段，而行则是一个记录。

表中每一行都应该有可以唯一标志自己的一列，称之为**主键**。表中的任何一列都可以作为主键，只要其满足：

- 任意两行都不具有相同的主键值；
- 每行都必须有一个主键值（主键列不允许为NULL值）。

主键的使用习惯：不更新主键列中的值；不重用主键列中的值；不在主键列中使用可能会更改的值。

MySQL数据库是基于客户机-服务器的数据库，服务器负责所有数据访问和处理，而客户机则是与用户打交道的软件；客户机通过发送请求给服务器软件，服务器则负责处理这个请求，根据需要过滤、丢弃和排序数据，然后将结果送回给客户机。

**Linux下安装MySQL**

```shell
sudo apt-get install mysql-server
sudo apt-get install mysql-client
sudo apt-get install libmysqlclient-dev

mysql -u root -p  #登录MySQL数据库
mysql> show databases;  #查看当前的数据库
mysql> use mysql;  #选择mysql数据库进行操作
mysql> show tables;   #显示当前数据库的表单
mysql> show columns from db;  #显示表单db中的表列
```

使用C++作为客户机操作数据库时，包含头文件`mysql/mysql.h`，编译链接时使用动态链接库`lmysqlclient`。

### 2. 常用操作

#### 2.1 数据库操作

﻿使用 create 命令创建数据库，使用drop删除数据库，使用use链接数据库

```shell
mysql> create database study;    #创建数据库
mysql> drop database study;      #删除数据库
mysql> use study;

#使用mysql admin来操作数据库也是可以的
mysqladmin -u root -p create study
mysqladmin -u root -p drop study
```

#### 2.2 增删数据表

MySQL支持数值、日期/时间和字符串(字符)类型。具体类型参见下表：

| 类型 | tinyint | smallint | meadiumint |  int  | bigint | float | double |
| ---- | :-----: | :------: | :--------: | :---: | :----: | :---: | :----: |
| 大小 |  1字节  |  2字节   |   3字节    | 4字节 | 8字节  | 4字节 | 8字节  |

| 类型      | 大小(字节) | 范围                                                         | 格式                | 用途                     |
| --------- | ---------- | ------------------------------------------------------------ | ------------------- | ------------------------ |
| DATE      | 3          | 1000-01-01/9999-12-31                                        | YYYY-MM-DD          | 日期值                   |
| TIME      | 3          | '-838:59:59'/'838:59:59'                                     | HH:MM:SS            | 时间值或持续时间         |
| YEAR      | 1          | 1901/2155                                                    | YYYY                | 年份值                   |
| DATETIME  | 8          | 1000-01-01 00:00:00/9999-12-31 23:59:59                      | YYYY-MM-DD HH:MM:SS | 混合日期和时间值         |
| TIMESTAMP | 4          | 1970-01-01 00:00:00/2038结束时间是第 **2147483647** 秒，北京时间 **2038-1-19 11:14:07**，格林尼治时间 2038年1月19日 凌晨 03:14:07 | YYYYMMDD HHMMSS     | 混合日期和时间值，时间戳 |

**创建数据表**

创建数据表需要以下信息：表名，表字段，定义每个表字段。通用语法如下：

```
create table table_name (column_name column_type);
```

下面是一个示例（**注意：**MySQL命令终止符为分号）：

```mysql
mysql> CREATE TABLE study_test(
   -> id INT NOT NULL AUTO_INCREMENT,
   -> title VARCHAR(100) NOT NULL,
   -> submission_date DATE,
   -> PRIMARY KEY (id)
   -> )ENGINE=InnoDB DEFAULT CHARSET=utf8;
Query OK, 0 rows affected (0.16 sec)
```

**删除数据表：**

```
mysql> drop table table_name;
```

#### 2.3 表项的增删查改

**向表中插入数据**，语法如下：

```mysql
INSERT INTO table_name ( field1, field2,...fieldN )
                       VALUES
                       ( value1, value2,...valueN );
```

举个例子：

```mysql
insert study_test (title, submission_date) values ("frist", now());
```

**使用SELECT语句来查询数据**，语法如下：

```mysql
SELECT column_name,column_name
FROM table_name
[WHERE Clause]
[LIMIT N][ OFFSET M]
```

- 查询语句中你可以使用一个或者多个表，表之间使用逗号(,)分割，并使用WHERE语句来设定查询条件。
- SELECT 命令可以读取一条或者多条记录。
- 你可以使用星号（*）来代替其他字段，SELECT语句会返回表的所有字段数据
- 你可以使用 WHERE 语句来包含任何条件。
- 你可以使用 LIMIT 属性来设定返回的记录数。
- 你可以通过OFFSET指定SELECT语句开始查询的数据偏移量。默认情况下偏移量为0。

```mysql
select * from study_test;   #返回表中所有数据
select * from study_test where title='frist';   #返回title为'first'的所有表项
```

**使用update更新数据，**语法如下：

```mysql
UPDATE table_name SET field1=new-value1, field2=new-value2
[WHERE Clause]
```

你可以同时更新一个或多个字段；可以在 WHERE 子句中指定任何条件；你可以在一个单独表中同时更新数据。举个例子：

```mysql
update study_test set title='english' where id=0;
```

**使用delete删除数据**，语法如下：

```mysql
DELETE FROM table_name [WHERE Clause]
```

如果没有指定 WHERE 子句，MySQL 表中的所有记录将被删除；可以在 WHERE 子句中指定任何条件;可以在单个表中一次性删除记录。

```mysql
delete from study_test where title='english';
```

