---
title:  "[Python] 플라스크"
excerpt: " "

categories:
  - Python

toc: true
toc_sticky: true
 
date: 2023-12-09
---

# Flask

파이썬으로 백앤드 개발을 할 때 필요한 웹 프레임워크로, 작은 규모의 프로젝트나 마이크로 서비스를 개발할 때 적합하다.

## 간단 사용 예시

```
pip install flask
```

```py
from flask import Flask
app = Flask(__name__)


@app.route('/')
def hello_world():
    return 'Hello, World!'


if __name__ == "__main__":
    app.run()
```

단순히 실행하거나 다음 명령어로 실행

```
python -m flask run
```

## 데코레이터

플라스크 예시 코드에 보이는 `@app.route('/')` 부분을 데코레이터라고 한다. 위 경우 서버에 접근할 경로를 설정하는데 사용한다. 파이썬의 데코레이터를 알기 위해서 다음 개념들이 필요하다.

### 일급 객체

파이썬에서 함수는 일급 객체로 간주되어 다른 함수의 매개변수로 전달 가능

```py
def plus(x, y):
    return x + y


def calculator(func, a, b):
    return func(a, b)

result = calculator(plus, 2, 3)
```

### 중첩 함수

함수 내에 다른 함수를 정의

```py
def outer_function():
    print("I'm outer")

    def nested_function():
        print("I'm inner")

    nested_function()

outer_function()
```

### 함수를 반환하는 함수

```py
def outer_function():
    print("I'm outer")

    def nested_function():
        print("I'm inner")

    return nested_function

inner_function = outer_function()
inner_function()
```

### 파이썬 데코레이터

```py
def decorator(function):
    def wrapper_function():
        #Do something before
        function()
        function()
        #Do something after
    return wrapper_function


@decorator
def say_hello():
    print("Hello")

say_hello()
```

```
Hello
Hello
```

데코레이터는 아래 코드를 `@함수명`을 통해 축약한 것이다.

```py
def say_greeting():
    print("How are you?")
decorated_function = decorator(say_greeting)
decorated_function()
```

정리하자면 데코레이터는 위 세개의 개념을 활용한 프로그래밍 코드를 간결하게 만드는(이 경우엔 `@함수명`, Syntax sugar라고 부름) 문법적 편의 기능이다.

