在Linux内核中，使用cdev结构体描述一个字符设备，其定义如下：

```C++
struct cdev { 
	struct kobject kobj;                  //内嵌的内核对象.
	struct module *owner;                 //该字符设备所在的内核模块的对象指针.
	const struct file_operations *ops;    //该结构描述了字符设备所能实现的方法，是极为关键的一个结构体.
	struct list_head list;                //用来将已经向内核注册的所有字符设备形成链表.
	dev_t dev;                            //字符设备的设备号，由主设备号和次设备号构成.
	unsigned int count;                   //隶属于同一主设备号的次设备号的个数.
}
```

cdev结构体中的一个重要成员file_operations定义了字符设备驱动提供给虚拟文件系统的接口函数，如常见的open、read、write等。

Linux内核提供了一组函数以用于操作cdev结构体，用于完成cdev的初始化、注册、注销等功能。

**cdev_init**函数用于初始化cdev的成员，并建立cdev和file_operations之间的连接，其源代码如下：

```C++
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
	memset(cdev, 0, sizeof *cdev);
	INIT_LIST_HEAD(&cdev->list);
	kobject_init(&cdev->kobj, &ktype_cdev_default);
	cdev->ops = fops;
}
```

**cdev_alloc()**函数用于动态申请一个cdev内存，其源代码如下：

```C++
struct cdev *cdev_alloc(void)
{
	struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
	if (p) {
		INIT_LIST_HEAD(&p->list);
		kobject_init(&p->kobj, &ktype_cdev_dynamic);
	}
	return p;
}
```

在上面的两个初始化的函数中，我们没有看到关于owner成员、dev成员、count成员的初始化；其实，owner成员的存在体现了驱动程序与内核模块间的亲密关系，struct module是内核对于一个模块的抽象，该成员在字符设备中可以体现该设备隶属于哪个模块，在驱动程序的编写中一般由用户显式的初始化 .owner = THIS_MODULE, 该成员可以防止设备的方法正在被使用时，设备所在模块被卸载。而dev成员和count成员则在cdev_add中才会赋上有效的值。

**int cdev_add(struct cdev \*p, dev_t dev, unsigned count);**

**该函数向内核注册一个struct cdev结构**，即正式通知内核由struct cdev *p代表的字符设备已经可以使用了。

**void cdev_del(struct cdev *p)；**

**该函数向内核注销一个struct cdev结构**，即正式通知内核由struct cdev *p代表的字符设备已经不可以使用了。