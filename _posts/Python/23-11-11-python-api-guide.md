---
title:  "[Python] API & requests 라이브러리"
excerpt: " "

categories:
  - Python

toc: true
toc_sticky: true
 
date: 2023-11-11
---

# API

어떤 리그에 속한 모든 축구 선수들이 거쳐온 경력과 기록을 보여주는 프로그램을 작성한다고 가정해보면, 당연히 이에 관한 데이터가 필요할 것이다. 이런 데이터를 개발자가 하나하나 추가해야 할까? 그런 과정은 끝이 보이지 않는 노동이 될 것이다. 이런 방대한 데이터나 기존의 구현 기능을 이용할 수 있게 해주는 것이 바로 API이다. API는 다음 역할을 한다.

1. 서버와 DB간 출입구 역할
2. 기기와 어플리케이션 간 원할한 통신
3. 모든 접속의 표준화

API하면 항상 따라오는 단어가 바로 앤드포인트이다. 앤드포인트는 API가 서버의 자원에 접근할 수 있도록 하는 URL이다. `requests` 라이브러리는 이런 앤드포인트를 이용하여 다양한 처리를 수행한다. 

## API 사용 예시

[국제 우주 정거장의 현재 위도와 경도](http://open-notify.org/Open-Notify-API/ISS-Location-Now/)를 보여주는 API가 존재한다. 이 API를 이용한 `requests` 라이브러리는 다음과 같다. 

```py
import requests

response = requests.get(url="http://api.open-notify.org/iss-now.json")
response.raise_for_status()

data = response.json()["iss_position"]

latitude = data["latitude"]
longitude = data["longitude"]

position = (latitude, longitude)

print(position)
```

모든 HTTP 응답은 숫자로 이루어져 있다. 그리고 요청이 성공적이면 200을 반환한다. `raise_for_status()`는 반환값이 200이 아닌 다른 값이면 항상 에러를 출력하고 프로그램이 종료되도록 하는 메소드이다. 그리고 이런 요청에 대한 반환은 json 형식으로 이루어져 있다. 파이썬에서는 이 응답에 대한 `json()` 메소드를 사용하면 딕셔너리 형태로 응답값을 사용할 수 있다.

### API와 문서

API는 코드와는 다르게 엄연히 제각각의 사용법이 존재한다. 그리고 API를 제공하는 곳에서는 항상 이 API에 대한 사용 방법을 정리해둔다. 따라서 API를 사용하기 전에 늘 문서를 꼼꼼하게 참고해야 한다. 예를 들어 날씨 정보를 저장해놓은 [OpenWeather](https://openweathermap.org/)의 [One Call API 3.0](https://openweathermap.org/api/one-call-3) 문서를 살펴보면 요청에 필요한 필수 Jquery의 값과 자료형, 그리고 선택적인 Jquery 값을 살펴볼 수 있다. 위 API의 경우 필수적으로 위도(lat)와 경도(lon), 그리고 API key(appid)가 필요하다. 이런 값들을 파이썬에서 전달하는 방법은 다음과 같다.

```py
import requests

weather_endpoint = "https://api.openweathermap.org/data/3.0/onecall"
weather_params = {
    "lat": 0,
    "lon": 0,
    "appid": "asdqwesa2412df",
}

response = requests.get(url=weather_endpoint, params=weather_params)
```

# HTTP 통신

requests는 파이썬에서 HTTP 통신을 하기 위한 라이브러리다. 위 코드의 `get()` 메소드는 HTTP 통신의 GET 요청이다.

|통신|설명|
|:---:|:---:|
GET|데이터 읽기 요청
POST|데이터 쓰기 요청
PUT|데이터 수정 요청
DELETE|데이터 삭제 요청

또한 API는 GET 만을 사용하지 않는다. 예를 들어 습관 기록 API인 [Pixela](https://pixe.la/)는 모든 HTTP 통신을 활용한다. 또한 모든 메소드가 Jquery가 아닌 json값으로 전달할 수 있다. 둘 모두 딕셔너리 자료형으로 전달한다.

```py
import requests

response = requests.post(url=endpoint, json=json_value)
```

## Header

HTTP 통신에서 헤더는 부가적인 정보를 전송하고, Jquery와는 다르게 헤더는 주소창에 그 내용이 표기되지 않기 때문에 일부 API는 헤더나 앞서 설명한 json를 통해 API 키 값을 주고 받는 경우가 있다. 마찬가지로 딕셔너리 자료형을 사용한다.

```py
import requests

header_value = {
    "Content-Type": "Application/json",
}

response = requests.post(url=endpoint, headers=header_value, json=json_value)
```

또한 몇몇 API는 데이터를 보내는 형식인 `Content-Type`의 정의를 요구하는 경우가 있다. 이 역시 헤더에 정의하여 전송한다. 물론 위 경우 이미 json 형식으로 요청을 보내기 때문에 굳이 헤더에 정의하여 보내지 않아도 작동은 잘 된다.

## 보안

몇몇 API는 보안을 위해 API 키 이외에도 로그인이나 토큰을 요구하는 경우가 있다. 다음은 이런 로그인을 요구하는 API를 위해 작성하는 코드의 두 가지 예시다.

```py
import requests
from requests.auth import HTTPBasicAuth

basic_auth = HTTPBasicAuth('id', 'password')
response = requests.post(url=endpoint, json=json_value, auth=basic_auth)
```

```py
import requets

header_value = {
    "Authorization": "auth_value",
}
response = requests.post(url=endpoint, headers=header_value)
```

물론 이 또한 각 API 마다 정리해 놓은 문서를 살펴보고 요구하는 형식을 맞춰 요청을 해야한다. 위 두 개의 코드는 구글 스프레드 시트에 접근할 수 있는 API인 [Sheety](https://sheety.co/)가 요구하는 인증 정책이다.

### 환경 변수로 민감 정보 숨기기

몇몇 API는 사용량에 따른 요금을 부과하는 경우도 존재한다. 이런 경우 API 키를 하드 코딩하는 것은 바람직하지 않을 것이다. 따라서 환경 변수를 이용하여 이런 API 키를 포함한 보안에 민감한 변수는 감춘다. 리눅스 실행 환경에서는 환경 변수를 다음과 같이 설정한다.

```bash
export API_KEY=value
```

윈도우에서는 파이썬의 `os` 모듈을 통해 환경 변수를 설정한다.

```py
os.environ["API_KEY"] = "value"
#환경 변수 접근
print(os.environ.get("API_KEY"))
# 환경 변수 삭제
print(os.environ.pop("API_KEY"))
```