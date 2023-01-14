---
title:  "[HTML] 강의 정리"
excerpt: "생활코딩 HTML 강의"

categories:
  - WEB

toc: true
toc_sticky: true
 
date: 2023-01-12
last_modified_at: 2023-01-14
---

# 태그

## strong

텍스트를 진하게 만드는 태그
``` html
<strong>내용</strong>
```

## u

텍스트에 밑 줄을 추가하는 태그
```html
<u>내용</u>
```

## h

텍스트의 제목과 관련된 태그로, `h1`부터 `h6`까지 존재한다
```html
<h1>This is heading 1</h1>
<h2>This is heading 2</h2>
<h3>This is heading 3</h3>
<h4>This is heading 4</h4>
<h5>This is heading 5</h5>
<h6>This is heading 6</h6>
```

## 줄바꿈

줄바꿈 태그는 두 가지가 존재한다.

### br
줄바꿈을 한 번 수행하는 태그로 시작 부분만 존재하고 종료 부분은 존재하지 않는다.
```html
<br>
```

### p
정해진 여백 만큼 줄바꿈을 수행하는 태그
```html
<p>내용</p>
```

**CSS**

CSS를 이용하여 여백의 크기를 조정 가능하다
```html
<p style = "margin-top: 10px;">내용</p>
```

## img

웹에 이미지를 추가하는 태그

```html
<img src = "이미지 이름">
```

**CSS**

`img` 태그 또한 CSS를 이용하여 이미지의 크기를 조정할 수 있다

```html
<img src = "이미지 이름" width = "100%">
```

## 리스트

### li

텍스트를 리스트로 만드는 태그

```html
<li>a</li>
<li>b</li>
<li>c</li>
```

### ul

`li` 태그의 부모 태그로 공통된 리스트를 하나로 묶는 역할을 한다.(Unordered List)
```html
<ul>
    <li>a</li>
    <li>b</li>
    <li>c</li>
</ul>
```
### ol
`li` 태그의 부모 태그로 공통된 리스트를 하나로 묶고 순서를 매긴다.(Ordered List)
```html
<ol>
    <li>a</li>
    <li>b</li>
    <li>c</li>
</ol>
```

## html, head, body

html 전체 문서를 감싸는 태그로 기본 형식은 다음과 같다.
```html
<html>
    <head>
    </head>
    <body>
    </body>
</html>
```

### head

`head` 태그에는 웹브라우저의 탭(타이틀) 부분의 이름을 정하는 `title` 태그와 출력 형식을 지정하는 `meta` 태그가 표기된다
```html
<head>
    <title>제목</title>
    <meta charset = "utf-8">
</head>
```

## 링크 삽입

`a` 태그를 이용하여 웹의 텍스트에 링크를 삽입할 수 있다.

```html
<a href = "링크 주소">텍스트</a>
```

## 동영상 삽입

https://www.youtube.com/watch?v=lB3z8r6lNB8

해당 유튜브 동영상을 공유한다고 가정한다면, 유튜브 공유 기능의 퍼가기를 선택하여 html 소스를 붙여넣기 하면 된다. `iframe` 태그를 이용하며 위 동영상의 html 소스는 다음과 같다.

```html
<iframe width="560" height="315" src="https://www.youtube.com/embed/lB3z8r6lNB8" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>
```

## 방문자 분석기

구글에서 제공하는 방문자 분석기를 이용하여 웹의 방문자 숫자를 알 수 있다.
[링크](https://www.youtube.com/watch?v=4QCZxcCBtCI)참조

# 인터넷
가장 기초적인 인터넷을 구성하는데 필요한 컴퓨터는 총 2대다. 하나는 웹 브라우저, 다른 하나는 웹 서버로 구성되어 있다.

웹 브라우저(클라이언트)는 정보를 요청하고 웹 서버(서버)는 요청된 정보에 대해 응답을 한다.

![image](/assets/images/web_image_01.png)

## 깃허브를 이용한 웹 호스팅
깃허브를 이용하여 웹 호스팅을 할 수 있다. 깃허브 리포지토리의 pages 기능을 이용하는 방법이다. 웹 호스팅이 완료되면 [링크](https://sehako.github.io/my-first-web-site/)가 생성된다.

웹 호스팅 과정을 그림으로 표현하면 다음과 같다.
![image](/assets/images/web_image_02.png)

대부분 웹 호스팅 시스템을 사용한다. 제작자는 웹을 구성하기만 하면 끝나기 떄문이다. 방문자의 요청에 따라서 웹을 서비스 하는 것은 호스팅하는 곳의 역할이다.

## 웹 서버 운영
Visual Studio의 확장 기능인 Live Server로 웹 서버를 운영해본다. Live Server 확장을 설치한 후 오른쪽 아래 Go Live 버튼을 누르면 웹 서버가 만들어진다. 