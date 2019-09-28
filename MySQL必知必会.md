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

| 类型      | 大小(字节) | 格式                | 用途                     |
| --------- | ---------- | ------------------- | ------------------------ |
| DATE      | 3          | YYYY-MM-DD          | 日期值                   |
| TIME      | 3          | HH:MM:SS            | 时间值或持续时间         |
| YEAR      | 1          | YYYY                | 年份值                   |
| DATETIME  | 8          | YYYY-MM-DD HH:MM:SS | 混合日期和时间值         |
| TIMESTAMP | 4          | YYYYMMDD HHMMSS     | 混合日期和时间值，时间戳 |

字符串类型主要用到的有：定长字符串（CHAR），变长字符串（VARCHAR），长文本数据（TEXT）。CHAR 和 VARCHAR 类型类似，但它们保存和检索的方式不同。它们的最大长度和是否尾部空格被保留等方面也不同。在存储或检索过程中不进行大小写转换。

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

**where子句与like子句**

where子句用于根据指定条件从表中选取数据，可以通过`and`或者`or`来指定多个条件，可以运用与select、update、delect命令中。like子句可以配合where子句，使用通配符进行模糊搜索，用于搜索含有指定字符的所有记录。语法如下：

```mysql
select field1, field2, ..., fieldn
from table_name
where field1 like condition and[or] condition2 
```

在 where like 的条件查询中，SQL 提供了四种匹配方式。

1. `%`：表示任意 0 个或多个字符。可匹配任意类型和长度的字符，有些情况下若是中文，请使用两个百分号（%%）表示。
2. ` _`：表示任意单个字符。匹配单个任意字符，它常用来限制表达式的字符长度语句。
3. `[]`：表示括号内所列字符中的一个（类似正则表达式）。指定一个字符、字符串或范围，要求所匹配对象为它们中的任一个。
4. `[^]` ：表示不在括号所列之内的单个字符。其取值和 [] 相同，但它要求所匹配对象为指定字符以外的任一个字符。

查询内容包含通配符时,由于通配符的缘故，导致我们查询特殊字符`%`、`_`、`[` 的语句无法正常实现，而把特殊字符用 `[ ]`括起便可正常查询。

```mysql
select * from runoob_tbl where runoob_author like '%COM'
```

**union操作符**

union操作符用于连接两个以上的select语句的结果，将其组合到一个结果集合中，多个select语句会删除重复的数据。语法如下：

```mysql
select field1, field2,..., fieldn
from tables
[where conditions]
union [all | distinct]
select field1, field2,..., fieldn
from tables
[where conditions];
```

示例如下：

```mysql
select country from websites
union all
select country from apps
order by country;
```

**使用order by进行排序**

如果我们需要对读取的数据进行排序，我们就可以使用 **order by** 子句来设定你想按哪个字段哪种方式来进行排序，再返回搜索结果。可以使用 ASC 或 DESC 关键字来设置查询结果是按升序或降序排列。 默认情况下，它是按升序排列。

```mysql
select field1, field2,...fieldn table_name1, table_name2...
order by field1 [asc|desc], [field2...] [asc|desc]]
```

#### 2.4 数据的汇总与分组

先介绍几个数据汇总相关的函数：

- arg() ，返回某列的平均值。
- count() ，返回某列的行数。
- max() ，返回某列的最大值。
- min() ，返回某列的最小值。
- sum() ，返回某列值之和。

示例如下：

```mysql
select avg(prod_price) as avg_price
from products
where vend_id = 1003;
```

上述函数对NULL值的处理，一般忽略NULL值。

count函数有两种使用方式：1）使用`count(*)`对表中行的数目进行计数，不管表列中包含的是控制（NULL）还是非空值；2）使用`count(column)`对特定列中具有值的行进行计数，忽略NULL值。

max函数和min函数一般用来找出最大或最小的数值或日期值，但MySQL允许将它用来返回任意列中的最大值，包括返回文本列中的最大值。

**group by语句**

GROUP BY 语句根据一个或多个列对结果集进行分组，在这些分组的列上，我们可以再使用count，sum，avg等函数对分组进行汇总。

```mysql
select vend_id, count(*) as num_prods
from products
group by vend_id;
```

使用group by的一些规定：

- group by子句可以包含任意数目的列，这使得能对分组进行嵌套，为数据分组提供更细致的控制。
- group by子句中列出的每个列都必须是检索列或有效的表达式（不能是聚集函数）。如果select中使用表达式，则必须在group by子句中指定相同的表达式，不能使用别名。
- 除聚集计算语句外，select语句中的每个列都必须在group by子句中给出。
- 如果分组列中具有NULL值，则NULL将作为一个分组返回。如果列中有多行NULL值，它们将分为一组。
- group by子句必须出现在where子句之后，order by子句之前。

group by子句不使用where来过滤分组（注意这里说的是过滤分组，而不是过滤行），而是使用having来实现过滤分组功能。

```mysql
select cust_id, count(*) as orders
from products
group by cust_id
having count(*) >= 2;
```

**having与where的差别：可以这样理解，where在数据分组前进行过滤，having在数据分组后进行过滤。**





### 3. 数据库设计

优良的设计：减少数据冗余，避免数据维护异常，节约存储空间，高效的访问。

#### 3.1 设计范式

为了建立冗余较小、结构合理的数据库，设计数据库时必须遵循一定的规则。在关系型数据库中这种规则就称为范式。范式是符合某一种设计要求的总结。要想设计一个结构合理的关系型数据库，必须满足一定的范式.

1. 第一范式：确保每列保持原子性。
2. 第二范式：确保表中每列都和主键相关。
3. 第三范式：确保每列都和主键列直接相关，而不是间接相关。

#### 3.2 字段类型选择

**字段类型的选择原则**：列的数据类型影响存储空间开销，另一方面影响查询性能；当一个列可以选择多种数据类型时，优先选择数字类型，其次是日期和二进制类型，最后才是字符串类型。

以上原则主要是从下面两个角度考虑：

- 在对数据进行比较（查询条件、JOIN条件及排序）操作时，同样的数据，字符处理往往比数据处理慢；
- 在数据库中，数据处理以页为单位，列的长度越小，越有利于性能提升。


**char和varchar选择原则**：

- 如果列中要存储的数据长度差不多是一致的，则应该考虑用char；否则应该考虑用varchar。
- 如果列中的最大数据长度小鱼50Byte，则一般也考虑用char。当然，如果这个列很少用，则基于节省空间和减少IO的考虑，还是选择用varchar。
- 一般不宜定义大于50Byte的char类型列。

**decimal与float选择原则**：

- decimal用于存储精确数据，而float智能用于存储非精确数据。古精确数据智能选择用decimal类型。
- 由于float的存储空间开销一般比decimal小，故非精确数据优先选择float类型。


