## 流畅的Python

### 纸牌类

```python
# 纸牌类实现
import collections
Card = collections.namedtuple('Card', ['rank', 'suit'])
class FrenchDeck:
    ranks = [str(n) for n in range(2, 11)] + list('JQKA')
    suits = 'spades diamonds clubs hearts'.split( )
    
    def __init__(self):
        self._cards = [Card(rank, suit) for suit in self.suits
                                        for rank in self.ranks]
        
    def __len__(self):
        return len(self._cards)
    
    def __getitem__(self, position):
        return self._cards[position]

    
#纸牌类使用示例
deck = FrenchDeck()
print(len(deck))   #类实现了__len__特殊方法，因此可以支持len方法
print(deck[0])     #类实现了__getitem__特殊方法，因此可以支持切片操作

from random import choice
print(choice(deck))#随机抽取一张纸牌

#纸牌排序
suit_values = dict(spades=3, hearts=2, diamonds=1, clubs=0)
def spades_high(card):
    rank_value = FrenchDeck.ranks.index(card.rank)
    return rank_value * len(suit_values) + suit_values[card.suit]
for card in sorted(deck, key=spades_high):
    print(card)
    

```

注：

- namedtuple用于构建只有少数属性但没有方法的对象，比如数据库条目等。这里我们利用namedtuple可以轻松地得到一个纸牌对象`beer_card = Card('7', 'diamonds')`。
- 在FrenchDeck类中，我们实现了一些特殊方法`__len__`和`__getitem__`，这样我们地FrenchDeck类就支持`len`和切片操作。通过实现特殊方法，我们可以更加方便地利用Python地标准库，从而不用重复发明轮子。



### 列表推导与生成器表达式

列表推导是构建列表地快捷方式，而生成器表达式则可以用来创建其他任何类型的序列

列表推导通常只用来创建新的列表，并且尽量简短。如果列表推导的代码超过了两行，可以考虑是不是用for循环重写了。表达式内部的变量和赋值只在局部起作用，表达式的上下文里的同名变量还可以被正常引用，局部变量并不会影响到它们。

列表推导可以帮助我们把一个序列或是其他可迭代类型中的元素过来或是加工，然后再新建一个列表。

```python
x = 'ABC'
dummy=[ord(x) for x in x]
print(x)   #'ABC' x的值被保留了
print(dummy)  #[65, 66, 67] 列表推导也创建了正确的列表
```



列表推导的作用只有一个：生成列表。如果想生成其他类型的序列，生成器就派上了用场。

生成器表达式可以逐个地产生元素，而不是先建立一个完整的列表，然后再把这个列表传递到某个构造函数里。县任前面哪种方式效率更高，更节省内存。

生成器表达式地语法和列表推导差不多，只不过**把方括号换成圆括号**而已。

```python
colors = ['black', 'white']
sizes = ['S', 'M', 'L']
for tshirt in ('%s %s'%(c,s) for c in colors for s in sizes):
    print(tshirt)
```





### 把函数视为对象

在Python中，函数是一等对象，即满足以下条件：

- 在运行时创建；
- 能赋值给变量或数据结构中地元素
- 能作为参数传给函数；
- 能作为函数地返回结果。

接受函数为参数，或者把函数作为结果返回的函数，我们称之为**高阶函数**。如内置函数sorted，可选参数用于提供一个函数应用于各个元素上进行排序。

```python
def factorial(n):
    '''retruns n!'''
    return 1 if n<2 else n*factorial(n-1)
>>> factorial.__doc__ #读取函数的__doc__属性
'retruns n!'
>>> fact = factorial
>>> fact(5)  #通过别的名称使用函数
120

>>> fruits = ['strawberry', 'fig', 'apple', 'cherry', 'banana']
>>> sorted(fruits, key=len)
['fig', 'apple', 'cherry', 'banana', 'strawberry']
```

Python中常用的高阶函数有map、filter和reduce，但由于列表推导和生成表达式的引入，它们没那么重要。列表推导或生成表达式具有map和filter两个函数的功能，而且更易读。

```python
>>> list(map(factorial, range(6)))
[1, 1, 2, 6, 24, 120]
>>> [factorial(n) for n in range(6)]
[1, 1, 2, 6, 24, 120]
>>> list(map(factorial, filter(lambda n :n%2, range(6))))
[1, 6, 120]
# 使用列表推导做相同的工作，换掉map和filter，并避免了使用lambda表达式
>>> [factorial(n) for n in range(6) if n%2]
[1, 6, 120]
```

lambda关键字在Python表达式内创建匿名函数。然而，Python简单的句法限制了lambda函数的定义体只能使用纯表达式。换句话说，lambda函数的定义体中不能赋值，也不能是哦那个while等Python语句。

除了作为参数传给高阶函数之外，Python很少使用匿名函数。由于句法上的限制，非平凡的lambda表达式要么难读，要么无法写出。

### 支持函数式编程的包

得益于operator和functools等包的支持，python也可以顺畅地使用函数式编程风格。

在函数式编程中，经常需要把算数运算符当作函数使用。operator模块为多个算术运算符提供了对应地函数，从而避免编写`lambda a,b : a*b`这种平凡的匿名函数。

```python
from functools import reduce
from operator import mul
def fact(n):
    return reduce(lambda a,b :a*b, range(1, n+1))
# 使用reduce和operator.mul函数计算阶乘
def fact(n):
    return reduce(mul, range(1, n+1))
```

operator模块中还有一类函数，能代替从序列中取出元素或读取对象属性的lambda表达式，即itemgetter和attrgetter。

operator模块提供的itemgetter函数用于获取对象的哪些维的数据，参数为一些序号。如果将多个参数传给itemgetter，它构建的函数会返回提取的值构成的元组。

要注意，operator.itemgetter函数获取的不是值，而是定义了一个函数，通过该函数作用到对象上才能获取值。

```python
>>> from operator import itemgetter
>>> a = [1, 2, 3]
>>> func = itemgetter(1)
>>> func(a)
2
>>> func = itemgetter(1, 0)
>>> func(a)
(2, 1)

# 用 operator 函数进行多级排序
>>> students = [('john', 'A', 15), ('jane', 'B', 12), ('dave', 'B', 10),]  
>>> sorted(students, key=itemgetter(1,2))  # sort by grade then by age  
[('john', 'A', 15), ('dave', 'B', 10), ('jane', 'B', 12)]  
# 对字典排序
>>> d = {'data1':3, 'data2':1, 'data3':2, 'data4':4}  
>>> sorted(d.iteritems(), key=itemgetter(1), reverse=True)  
[('data4', 4), ('data1', 3), ('data3', 2), ('data2', 1)] 
```

`itemgetter(1)`的作用于`lambda fields : fields[1]`一样，即创建一个接受集合的函数，返回索引为1的元素。

functools模块提供了一系列高阶函数，其中最为人熟知的或许是redudce。余下的函数中，最有用的是partial及其变体partialmethod。

partial函数用于部分应用一个函数，即基于一个函数创建一个新的柯调用对象，把原函数的某些参数固定。

```python
>>> from operator import mul
>>> from functools import partial
>>> triple = partial(mul, 3)
>>> triple(7)
21
>>> list(map(triple, range(1,10)))
[3, 6, 9, 12, 15, 18, 21, 24, 27]
```



### 对象引用与可变性

Python变量类似于Java中的引用式变量，因此可以把他们理解为附加在对象上的标注。

```python
# 变量a,b引用同一个列表，而不是那个列表的副本
>>> a = [1, 2, 3]
>>> b = a
>>> a.append(4)
>>> b
[1, 2, 3, 4]

# b是a的别名，is运算符和id函数确认了这一点
>>> a is b
True
>>> id(a), id(b)
(2163226511368, 2163226511368)
```

为了理解Python中的赋值语句，应该始终先读右边。对象在右边创建或获取，在此之后左边的变量才会绑定到对象上，这就像为对象贴上标签。

每个变量都有标识、类型和值。对象一旦创建，它的标识在对象生命周期中绝不会变；你也可以把标识理解为对象在内存中的地址。is运算符比较两个对象的标识；id函数返回对象标识的整数表示。

注意下==运算符和is运算符的区别：==运算符比较的是两个对象的值，is则是比较对象的标识。

**元组的相对不可变性**

元组与多数python集合一样，保存的是对象的引用。如果引用的元素是可变的，即便元组本身不可变，元素依然可变。也就是说，元组的不可变性其实是指tuple数据结构的物理内容（即保存的引用）不可变，与引用的对象无关。

```python
# 元组的值会随着引用的可变对象的变化而变化，不变的是元组中元素的标识
>>> t = (1, 2, [30, 40])
>>> id(t[-1])
2163225629640
>>> t[-1].append(99)
>>> t
(1, 2, [30, 40, 99])
>>> id(t[-1])
2163225629640
```

**深复制与浅复制**

复制列表（或多数内置的可变集合）最简单的方式是使用内置的类型构造方法。如

```python
>>> l1 = [3, [55, 44], (7, 8, 9)]
>>> l2 = list(l1)
>>> l2
[3, [55, 44], (7, 8, 9)]
>>> l2 is l1
False
>>> l2 == l1
True
```

然而，构造方法做的是**浅复制，即复制了最外层容器，副本中的元素是源容器中元素的引用**。如果所有元素都是不可变的，那么这样没有问题，还能节省内存。但是，如果有可变的元素，可能会导致意想不到的问题。

```python
>>> l1 = [3, [55, 44], (7, 8, 9)]
>>> l2 = list(l1)
>>> l1[1] += [33,22]
>>> l2
[3, [55, 44, 33, 22], (7, 8, 9)]
>>> l1[2] += (10, 11)
>>> l2
[3, [55, 44, 33, 22], (7, 8, 9)]
```

- 对于可变对象来说，如l1[1]引用的列表，+=运算符就地修改列表，这次修改也会体现在l2[1]中，因为它是l1[1]的别名。
- 对于元组来说，+=运算符创建一个新元组，然后重新绑定给变量l1[2]；这样l1和l2中最后位置上的元组不再是同一个对象。

**copy模块提供的deepcopy和copy函数能为任意对象做深复制和浅复制**。

