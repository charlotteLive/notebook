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

