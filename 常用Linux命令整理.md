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

