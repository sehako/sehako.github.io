---
title:  "[Python] Comprehension"
excerpt: " "

categories:
  - Python

toc: true
toc_sticky: true
 
date: 2023-11-06
---

# 리스트 컴프리헨션

1부터 10까지의 요소를 가진 리스트를 생성할 때, 아마 가장 먼저 떠오르는 것은 다음과 같은 코드일 것이다.

```py
a_list = []
for i in range(10):
    a_list.append(i + 1)
```

파이썬은 다음 과정을 컴프리헨션을 이용하여 한 줄로 줄일 수 있다.

```py
a_list = [i + 1 for i in range(10)]
```

리스트 컴프리헨션은 다음 구조로 이루어져 있다.

```py
a_list = [element for element in range() if expression]
```

## 조건을 활용한 예시

다음은 조건이 있는 리스트 컴프리헨션의 한 예다.

```py
a = [1, 2, 3]
b = [2, 3, 4]
c = [x for x in a if x in b]
print(c)

>> [2, 3]
```

# 딕셔너리 컴프리헨션

딕셔너리 생성 시에도 컴프리헨션을 이용할 수 있다.

```py
a_list = [1, 2, 3]
a_dict = {num: 'int' for num in a_list}
```

마찬가지로 딕셔너리 컴프리헨션도 다음 구조로 이루어져 있다.

```py
a_dict = {element for element in range() if expression}
```

## 기존 딕셔너리를 이용한 컴프리헨션

딕셔너리 자료형은 `.items()` 메소드를 이용하여 키와 값을 추출할 수 있다. 

```py
a_dict = {'a': 1, 'b': 2, 'c': 3}
b_dict = {value, key for key, value in a_dict.items()}
print(b_dict)

>> {1: 'a', 2: 'b', 3: 'c'}
```