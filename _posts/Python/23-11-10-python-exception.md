---
title:  "[Python] 예외 처리"
excerpt: " "

categories:
  - Python

toc: true
toc_sticky: true
 
date: 2023-11-10
---

# 에러

프로그램은 언제나 수 많은 잠재적 에러가 존재한다. 예를 들어 딕셔너리에 키 값을 입력했는데, 키 값에 대응되는 값이 없을 경우 키 에러를 반환하며 프로그램이 종료된다. 하지만 에러가 일어나도 프로그램이 종료되지 않고 그 에러에 대응하여 다른 처리를 하도록 만들 수 있는데, 그것이 바로 `try ~ finally` 예외 처리 구문이다.

## 예외 처리

파이썬의 예외 처리 키워드는 5개이다.

|키워드|설명|
|:---:|:---|
try|예외를 일으킬 수 있는 작업
except|예외 발생 시 실행
else|`try` 작업이 성공하면 실행
finally|예외 여부와 관계 없이 실행
raise|실제 에러와 관계 없이 에러를 출력

파일에 접근하는 코드를 작성했는데 파일이 없는 경우 다른 파일을 열거나 새로운 변수를 생성하는 코드를 예외 처리를 이용하여 작성한다면,

```py
try:
    with open("text.txt", 'r') as file:
        text = file.read()
except:
    text = "Hello"
```

하지만 다음 코드에서는 실제 파일이 존재해도 예외 처리가 실행된다.

```py
try:
    file = open("test_file.txt")
    text = file.read()
    a_dict = {"key": "value"}
    print(a_dict["aaa"])
except:
    text = "Hello"
```

그 이유는 파일이 존재해도 딕셔너리의 키 값이 존재하지 않기 때문이다. 따라서 파이썬은 각 에러에 관한 처리를 위해 에러 키워드가 존재한다. 파일이 존재하지 않는 경우 파이썬 터미널은 `FileNotFoundError`를 출력하고 키 값이 존재하지 않는 경우 `KeyError`를 출력한다.

따라서 각 에러에 대한 별개의 처리를 위해 에러 키워드를 이용하여 다음과 같이 작성이 가능하다.

```py
# test_file.txt가 존재한다고 가정
try:
    file = open("test_file.txt")
    text = file.read()
    a_dict = {"key": "value"}
    print(a_dict["aaa"])

# 파일에 대한 예외 처리만 담당
except FileNotFoundError:
    file = open("test_file.txt", 'w')
    file.write("file created")
# 딕셔너리 관련 예외 처리
except KeyError as error_msg:
    print(f"{error_msg} is not exist")
else:
    print(text)
```

이 코드에서 `else`에 존재하는 `print()`는 작동하지 않는다. 파일에 관한 예외 처리는 넘어갔지만 키 값에 대한 예외 처리가 실행되었기 때문이다. `else`는 모든 예외 처리가 실행되지 않았을 때 실행된다. 예외 처리와 관계 없는 코드는 `finally` 키워드 내에 작성한다.

예를 들어 위 코드에서는 `witn ~ as` 키워드가 아닌 일반적인 `open()`으로 파일에 접근했다. 따라서 파일을 다시 닫아줄 필요가 있다. 파일의 존재 유무와 관계없이 파일은 항상 열리거나 새로 만들어질 것이기 때문이다. 따라서 위 코드 맨 끝에 다음 코드를 추가한다.

```py
...
finally:
    file.close()
```

## 지정 오류 추가

자동차의 도시 내 주행에 대한 km/h를 m/s로 변환하는 간단한 코드를 작성한다고 가정하면 다음과 같을 것이다.

```py
km_per_hour = int(input("km/h: "))
meter_per_second = km_per_hour / 3.6
print(round(meter_per_second, 3))
```

위 코드는 잘 작동하지만 한 가지 문제가 있다. 바로 도시 주행 지정 속도를 초과 입력 하는 것에 대한 처리가 없다는 것이다. 이 때 사용되는 것이 사용자 지정 오류 `raise`이다.

```py
km_per_hour = int(input("km/h: "))
if km_per_hour > 50:
    raise ValueError("Over 50km/h!")
meter_per_second = km_per_hour / 3.6
print(round(meter_per_second, 3))
```

이 코드에서 속도를 50km/h 이상 입력할 경우 `ValueError`를 출력하고 그 이유를 문자열로 출력한다. 실제로 50이상 입력하는 것이 파이썬 내에서는 오류가 없지만 개발자가 임의로 오류를 출력한 것이다.