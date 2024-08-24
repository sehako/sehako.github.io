---
title:  "[Python] 웹 크롤링"
excerpt: " "

categories:
  - Python

toc: true
toc_sticky: true
 
date: 2023-12-07
---

# 크롤링

웹 사이트의 정보를 긁어오는 것이다. 긁어온 정보는 html 태그를 기준으로 찾고 처리한다. 파이썬에서는 크롤링을 지원하는 두 라이브러리가 존재한다.

## BeautifulSoup

```python
import requests
from bs4 import BeautifulSoup

url = 'https://kin.naver.com/search/list.nhn?query=%ED%8C%8C%EC%9D%B4%EC%8D%AC'

response = requests.get(url)
html = response.text
soup = BeautifulSoup(html, 'html.parser')
```

## Selenium

```python
from selenium import webdriver
from selenium.webdriver.common.by import By

edge_options = webdriver.EdgeOptions()
edge_options.add_experimental_option('detach', True)

driver = webdriver.Edge(options=edge_options)
driver.get(url="http://item.gmarket.co.kr/Item?goodscode=3228554913&buyboxtype=ad")

price = driver.find_element(By.CLASS_NAME, value="price_real")
price_name = driver.find_element(By.NAME, value="format-detection")
print(price_name.text)
print(price.text)

# 활성 탭 하나 종료
# driver.close()
# 브라우저 전체 종료
driver.quit()
```

셀레늄은 웹을 크롤링 하는 동시에 크롤링 하는 웹 사이트를 설정한 브라우저로 방문한다. 이를 통해 웹 페이지 요소를 클릭하거나 텍스트 상자에 텍스트를 써넣는 등의 자동화가 가능하다.

# 크롤링의 윤리적 문제

대부분의 웹 사이트는 이런 크롤링을 좋아하지 않는다. 구글을 예로 들면 [구글 주소/robots.txt](https://www.google.com/robots.txt)에 크롤링의 허용/비허용 부분을 정의해놓고, 몇몇 웹사이트 같은 경우 요청의 딜레이를 명시해 놓은 경우도 있다. 또한 CAPTCHA / reCAPTCHA로 자동화된 프로그램(봇)의 접근을 방지하기도 한다. 따라서 크롤링을 수행할 때 다음 두 개의 윤리적 규칙을 준수해야 한다.

1. 공공 API를 우선적으로 사용
2. 웹 사이트 제작자 존중(robots.txt)