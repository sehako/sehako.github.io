---
title:  "[JavaScript] 강의 정리"
excerpt: "생활코딩 JavaScript 강의"

categories:
  - WEB

toc: true
toc_sticky: true
 
date: 2023-01-16
last_modified_at: 2023-01-16
---

# 웹에서의 상호작용
웹에서의 상호작용을 위해 사용되는 언어가 바로 JavaScript이다. JavaScript는 HTML을 제어하여, 웹 페이지를 동적으로 만든다고 볼 수 있다. HTML의 `script` 태그로 자바 스크립트를 입력할 수 있다.

```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset = "utf-8">
        <title></title>
    </head>
    <body>
        <script>
            document.write('hello world');
        </script>
    </body>
</html>
```

**콘솔**

웹 브라우저에서 키보드 F12를 누르면 나오는 DevTools의 Consol탭에 나오는 입력 라인에도 자바 스크립트를 입력 가능하다.

# 이벤트 

웹 브라우저에서 일어나는 어떤 사건을 이벤트라고 한다. `on`으로 시작하는 아래 부분이 바로 이벤트를 발생시키는 코드이다.
```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset = "utf-8">
        <title></title>
    </head>
    <body>
        <script>
            document.write('hello world');
        </script>
        <!-- 버튼 클릭 시 알림 팝업 -->
        <input type = "button" value = "hi" onclick = "alert('hi')"> 
        <!-- 텍스트 입력 후 앤터 클릭하면 알림 팝업 -->
        <input type = "text" onchange = "alert('changed')">
        <!-- 텍스트 바꾸면 알림 팝업 -->
        <input type = "text" onkeydown = "alert('key down!')">
    </body>
</html>
```

# 변수

자바 스크립트에서 변수를 사용할 때에는 `var`이라는 키워드를 사용하도록 한다.

# 야간 모드 간단 예제
```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
    </head>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <input type = "button" value = "night" onclick = "
            document.querySelector('body').style.backgroundColor = 'black';
            document.querySelector('body').style.color = 'white';
        ">
        <input type = "button" value = "day" onclick = "
        document.querySelector('body').style.backgroundColor = 'white';
        document.querySelector('body').style.color = 'black';
        ">
        <ol>
            <li><a href="1.html">HTML</a></li>
            <li><a href="2.html">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>JavaScript</h2>
    </body>
</html>
```

```js
document.querySelector('body').style.backgroundColor = 'white';
```
위 코드의 뜻은 `body` 태그를 찾아 거기에 `css` 속성을 추가한다는 것이다.

## 조건문 추가

위 html문서에서 버튼이 두 개가 아닌 하나로 만들어 야간모드를 키고 끄게 만들고자 한다. 이럴 떄 조건문을 사용하여 조건에 따라 버튼의 텍스트를 바꿔가며 야간 모드를 키고 끄게 만들 수 있다.
```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
    </head>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <!-- 조건문을 사용한 야간/주간 모드의 통합 -->
        <input type = "button" value = "night" onclick = "
            if (this.value == 'night') {
                document.querySelector('body').style.backgroundColor = 'black';
                document.querySelector('body').style.color = 'white';
                this.value = 'day'
            }
            else {
                document.querySelector('body').style.backgroundColor = 'white';
                document.querySelector('body').style.color = 'black';
                this.value = 'night'
            }
        ">
        <ol>
            <li><a href="1.html">HTML</a></li>
            <li><a href="2.html">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>JavaScript</h2>
    </body>
</html>
```

## 리팩토링

코드의 중복을 제거하고 변수명을 직관적으로 짓는 등의 과정을 통해 간결화하여 유지 보수를 용이하게 만드는 것을 말한다. 위 코드를 리팩토링 과정을 거친다면

```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
    </head>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <input type = "button" value = "night" onclick = "
            var target = document.querySelector('body');
            if (this.value == 'night') {
                target.style.backgroundColor = 'black';
                target.style.color = 'white';
                this.value = 'day'
            }
            else {
                target.style.backgroundColor = 'white';
                target.style.color = 'black';
                this.value = 'night'
            }
        ">
        <ol>
            <li><a href="1.html">HTML</a></li>
            <li><a href="2.html">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>JavaScript</h2>
    </body>
</html>
```

```js
document.querySelector('body')
```
다음 중복 코드를 변수 하나를 생성하여 대입하였다.

```js
var target = document.querySelector('body');
```
이런 과정을 리팩토링이라고 한다.

# 배열과 반복문 예시

```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
    </head>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <input type = "button" value = "night" onclick = "
            var i = 0;
            var links = document.querySelectorAll('a');
            var target = document.querySelector('body');
            if (this.value == 'night') {
                target.style.backgroundColor = 'black';
                target.style.color = 'white';
                while(i < links.length) {
                    links[i].style.color = 'powderblue'
                    i++;
                }
                this.value = 'day'
            }
            else {
                target.style.backgroundColor = 'white';
                target.style.color = 'black';
                while(i < links.length) {
                    links[i].style.color = 'blue'
                    i++;
                }
                this.value = 'night'
            }
        ">
        <ol>
            <li><a href="1.html">HTML</a></li>
            <li><a href="2.html">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>JavaScript</h2>
    </body>
</html>
```

# 함수
```js
function function_name() {

}
```
해당 형식으로 자바 스크립트에서 함수를 선언할 수 있다. html 문에서 함수를 선언하려면 `script` 태그 안에 함수를 정의해야 한다.

## 매개변수를 이용한 함수
```js
function sum(a, b) {
    document.write(a + b + '<br>');
}
```
다른 프로그래밍 언어에서 사용하는 함수처럼, 자바 스크립트 또한 매개변수를 이용한 함수의 활용을 할 수 있다.

## 리턴을 이용한 함수
```js
function sum(a, b) {
    return a + b;
} 
```

리턴 값 또한 마찬가지이다. 

## 함수화 예제

기존의 야간/주간 모드 변환에 대한 코드를 함수로 만든 예제

```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
        <script>
            function night_day(self) {
                var i = 0;
                var links = document.querySelectorAll('a');
                var target = document.querySelector('body');
                if (self.value == 'night') {
                    target.style.backgroundColor = 'black';
                    target.style.color = 'white';
                    while(i < links.length) {
                        links[i].style.color = 'powderblue'
                        i++;
                    }
                    self.value = 'day'
                }
                else {
                    target.style.backgroundColor = 'white';
                    target.style.color = 'black';
                    while(i < links.length) {
                        links[i].style.color = 'blue'
                        i++;
                    }
                    self.value = 'night'
                }
            }
        </script>
    </head>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <input type = "button" value = "night" onclick = "night_day(this)">
        <ol>
            <li><a href="1.html">HTML</a></li>
            <li><a href="2.html">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>JavaScript</h2>
    </body>
</html>
```

# 객체
연관된 함수(메소드)와 변수(프로퍼티)들을 순서에 상관없이 그룹화하는 것이다.
```js
var object = {};
```
객체는 다음과 같이 중괄호로 선언이 가능하다.

## 객체에 프로퍼티와 메소드 추가

먼저, `names`라는 객체가 생성되었다고 가정했을 떄, 프로퍼티를 추가하는 것은 다음과 같다.
```js
names.bookkeeper = "qq";
```
`name` 객체에 `bookkeeper`라는 키 값을 가진 `qq`라는 프로퍼티가 추가되었다.

메소드는 다음과 같이 추가할 수 있다.

```js
names.showAll = function() {
    for(var key in names) {
        document.write(key + ':' + names[key]+ '<br>');
    }
}
```
`names` 객체에 `showAll`이라는 함수를 추가한 코드이다. 해당 코드는 `for` 반복문으로 객체 내 모든 키 값을 추출하고 그것을 출력하는 코드이다.

## 객체화 예시
```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
        <script>
            var Link = {
                setColor:function(color) {
                    var i = 0;
                    var links = document.querySelectorAll('a');
                    while(i < links.length) {
                        links[i].style.color = color
                        i++;
                    }
                }
            }
            var Body = {
                setBackground:function(color) {
                    document.querySelector('body').style.backgroundColor = color;
                },
                setColor:function(color) {
                    document.querySelector('body').style.color = color;
                }
            }

            function night_day(self) {
                if (self.value == 'night') {
                    Body.setBackground('black');
                    Body.setColor('white');
                    Link.setColor('powderblue');
                    self.value = 'day';
                }
                else {
                    Body.setBackground('white');
                    Body.setColor('black');
                    Link.setColor('blue');
                    self.value = 'night';
                }
            }
        </script>
    </head>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <input type = "button" value = "night" onclick = "night_day(this)">
        <ol>
            <li><a href="1.html">HTML</a></li>
            <li><a href="2.html">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>JavaScript</h2>
    </body>
</html>
```
