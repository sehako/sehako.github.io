---
title:  "[Python] 함수의 매개변수"
excerpt: " "

categories:
  - Python

toc: true
toc_sticky: true
 
date: 2023-11-08
---

# 함수

특정 일을 수행하는 명령어의 집합, 파이썬에서는 다음과 같이 선언한다.

```py
def add(x, y):
    print(x + y)

add(1, 2)

>> 3
```

이때 `x`와 `y`값을 매개변수라고 하며 함수를 실행할 때 값을 전달할 수 있게 한다.

## 타입 힌트

최근 파이썬은 타입 힌트를 지원한다. 예를 들어 `name`이라는 변수가 항상 문자열을 저장해야 한다면,

```py
name: str
```

다음과 같이 타입 힌트를 표기할 수 있다. 물론 자료형이 유연한 파이썬의 특성상 다른 자료형의 변수를 대입해도 상관은 없다. IDE에서 주의 문구를 알려줄 뿐이다.

함수 역시 매개변수와 반환값에 관한 타입 힌트를 표기할 수 있다.

```py
def booze_check(age: int) -> bool:
    if age > 19:
        return True
    else:
        return False 
    
print(cigar_check(20))

>> True
```

# 여러 매개 변수 전달 받기

## args

파이썬은 여러 매개 변수를 전달받는 함수를 작성할 수 있는 두 방법이 존재한다. 그 중 하나가 바로 `*args`이다. 정확히는 `*` 키워드가 매개변수를 여러 개 받을 수 있도록 해준다. `args`는 일종의 관례라고도 볼 수 있다. 이렇게 받은 매개 변수는 튜플 자료형으로 넘겨받는다.

```py
def add(*args):
    total = 0
    for i in args:
        total += i
    return total

print(add(1, 2, 3))
print(add(1, 2, 3, 4, 5))

>> 6
>> 15
```

## kwargs

다음은 `**kwargs`다. 이 경우 여러 개의 인수를 딕셔너리 형태로 전달받는다.

```py
def calculate(**kwargs):
    total = 0
    total += kwargs["add"]
    total *= kwargs["multiply"]
    return total

print(calculate(add=3, multiply=5))

>> 15
```

## 클래스 생성자와 매개변수

클래스 생성자에도 여러 개의 매개변수를 전달 할 수 있다.

```py
class Car:
    def __init__(self, *args, **kw):
        self.year = args[0]
        self.gas_mileage = args[1]
        self.brand = kw["brand"]
        self.model = kw["model"]

car = Car(2016, 26, brand="Ford", model="Mustang")

print(car.year, car.gas_mileage, car.brand, car.model)
```

물론 `args`는 튜플 자료형이고 `kwargs`는 딕셔너리이기 때문에 인덱스 범위 초과 오류와 키 에러를 항상 조심해야 한다.
