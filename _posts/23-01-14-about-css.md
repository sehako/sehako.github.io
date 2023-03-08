---
title:  "[CSS] 강의 정리"
excerpt: " "

categories:
  - WEB

toc: true
toc_sticky: true
 
date: 2023-01-14
---

# HTML에서 CSS 작성하기

HTML에서 CSS를 작성 및 적용하기 위해서는 HTML태그인 `style` 블록 안에 CSS를 작성해야 한다.

```html
<style>
  <!-- css 작성 -->
</style>
```

## CSS 예시

HTML 문서에서 `a` 태그를 사용하는 **모든** 텍스트의 색상을 변경한다고 가정한다면 `style` 블록 안에 작성되는 코드는 다음과 같다

```html
    <style>
        a {
            color: red;
        }
    </style>
```

위 코드에서 `a`는 선택자(selector) `color:red;` 부분은 효과(declaration)라고 한다.

물론 만약 HTML의 `font` 태그를 사용하여 `a` 태그를 사용하는 모든 텍스트의 색상을 변경할 수 있을것이다. 하지만 이는 `a` 태그를 사용하는 모든 부분을 찾고, 그 부분에 `font` 태그를 추가하는 작업을 거쳐야만 한다. 이런 작업 과정은 개발자에게 굉장한 부담과 코드의 가독성 저하를 불러 일으킬 것이다. 따라서 CSS를 이용한 디자인은 굉장히 효율적이라고 할 수 있다.

단일 텍스트의 색상은 아래 코드로 변경 가능하다.

```html
<a href="2.html" style = "color:black">CSS</a>
```

`style = ""`은 HTML의 속성이다. 위와 같은 코드에서는 선택자를 표기할 필요가 없다. 위 코드에 여러 개의 효과를 추가하고 싶다면 `;`으로 구분 및 추가하면 된다.

```html
<a href="2.html" style = "color:black; text-decoration: underline">CSS</a>
```

`style` 블록을 사용하는 부분에서 위 코드와 같은 효과를 보고자 한다면 추가 후 `;`으로 마무리 하면 된다.

```html
    <style>
        a {
            color: red;
            text-decoration: underline;
        }
    </style>
```

# 선택자 안에 들어갈 수 있는 효과

수업 중 예시로 사용된 효과를 간단히 정리해 보았다.

**[링크](https://www.w3schools.com/cssref/css4_pr_accent-color.php) 참조**

효과|기능
|:---:|:---:|
color|텍스트 색상 변경
text-decoration|밑 줄 등 텍스트 자체의 모양에 관여
font-size|글자 크기 변경
text-align|글자 정렬(left, right, center)
border|텍스트의 경계를 보여주고 편집
padding|텍스트의 경계 크기를 결정
margin|텍스트의 경계 간격을 결정
width|경계의 폭을 결정


# 선택자 기초
**[링크](https://www.w3schools.com/cssref/css_selectors.php) 참조**

```html
    <style>
        a {
            color: red;
        }
    </style>
```
앞서 작성한 `a` 태그를 이용한 CSS 작성은 편리하지만 한 가지 문제가 있다. 가령 `a` 태그를 사용하는 텍스트지만 현재 접속중인 링크일 떄 해당 링크를 다른 색상으로 변경하고자 한다면 어떻게 해야하는가?

위 질문에 대한 답변은 바로 `class` 속성과 `id` 속성이다.
```html
    <style>
        a {
            color: red;
        }
        .cls {
            color: gray;
        }
        #active {
            color: black;
        }
    </style>
    .
    .
    .
    <a href="2.html" class = "cls" id = "active">CSS</a>
```

선택자는 항상 우선순위를 갖는다. 우선 순위는 가장 강한 왼쪽 부터 다음과 같다.

`id > class > div(태그 선택자)`

`id`로 정의된(위 예시에서는 `active`) 값은 전체 코드에서 단 한 번만 등장해야 한다는 것이 권장된다.

## div & span
텍스트의 디자인 변경만을 위해서 태그를 사용하고자 할 때, 사용하는 선택자다. `div`는 블럭 레벨, `span`은 인라인 태그이다. 또한 이런 태그에 `id`값을 정의하여 사용도 가능하다.

```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset = "utf-8">
        <title></title>
    </head>
    <body>
        <div id = "grid">
        <div>NAVIGATION</div> 
        <span>ARTICLE</span>
        </div>
    </body>
</html>
```

# 박스 모델

CSS 박스 모델에 관한 자세한 정보는 [구글 검색](https://www.google.com/search?q=css+box+model&source=hp&ei=z5bDY7LIFNvr2roPnpG2kAY&iflsig=AK50M_UAAAAAY8Ok3z6rEh3jYuf6FBg23L_w1Hj5NV2o&oq=%E3%85%8A%E3%84%B4%E3%84%B4+%E3%85%A0%E3%85%90%E3%85%8C&gs_lcp=Cgdnd3Mtd2l6EAMYAjIFCAAQgAQyBQgAEIAEMgUIABCABDIFCAAQgAQyBQgAEIAEMgUIABCABDIFCAAQgAQyBQgAEIAEMgUIABCABDIFCAAQgAQ6BQguEIAEOggILhCABBCxAzoECC4QAzoLCAAQgAQQsQMQgwE6BAgAEAM6CAgAEIAEELEDOgsILhCABBCxAxDUAlAAWPQJYIgYaABwAHgAgAFziAH5BZIBAzEuNpgBAKABAQ&sclient=gws-wiz)참조

html의 태그는 각자 차지하는 화면 비율이 다르다. 화면 전체를 사용하는 태그를 블럭 레벨(block level) 태그라고 하고, 자신의 크기만큼을 가지는 태그를 인라인(inline) 태그라고 한다. 블럭 레벨 태그는 대표적으로 `h` 태그가 존재한다.
```css
<style>
    h1, a {
        border-width: 5px;
        border-color:red;
        border-style: solid;
    }
</style>

```
다음과 같은 태그가 있다면 출력되는 웹은 다음과 같다.

![image](/assets/images/web_image_03.png)

## 코드 줄이기
위 코드를 다음과 같이 줄일 수 있다.

```css
<style>
    h1, a {
        border: 5px solid red;
    }
</style>
```

## 박스 모델을 활용한 html웹 예제 코드
```html
<!DOCTYPE html>
<html>
    <head>
        <title>WEB Study</title>
        <meta charset="utf-8">
    </head>
    <style>
        a {
            color: black;
            text-decoration: none;
        }
        h1 {
            font-size: 50px;
            text-align: center;
            border-bottom: 1px solid gray;
            margin: 0;
            padding: 20px;
        }
        ol {
            border-right: 1px solid gray;
            width: 150px;
            margin: 0;
            padding: 20px;
        }
        body {
            margin: 0;
        }
        .saw {
            color:gray;
        }
        #active {
            color:red;  
        }
    </style>

    <body>
        <h1><a href="index.html">WEB</a></h1>
        <ol>
            <li><a href="1.html" class = "saw">HTML</a></li>
            <li><a href="2.html" class = "saw" id = "active">CSS</a></li>
            <li><a href="3.html">JavaScript</a></li>
        </ol>
        <h2>CSS</h2>
    </body>
</html>
```

# 그리드
```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset = "utf-8">
        <title>
        </title>
        <Style>
            #grid {
                border: 5px solid pink;
                display: grid;
                grid-template-columns: 150px 1fr;
            }
            div {
                border: 5px solid gray;
            }    
        </Style>
    </head>
    <body>
        <div id = "grid">
        <div>NAVIGATION</div> 
        <div>ARTICLE</div>
        </div>
    </body>
</html>
```
`display` 효과를 `grid`로 지정하면 그리드 형식으로 지정된다. 웹 형식을 일종의 표(?) 같이 다룰 수 있다.

그리드 기능을 지원하는 웹 브라우저인지 알고 싶다면 [caniuse.com](https://caniuse.com/?search=grid)을 참고한다.

# 반응형 디자인

브라우저의 상태에 따라서 웹의 표현 등이 다르게 보이게 하는 것을 반응형 디자인이라고 한다.

## 미디어 쿼리
```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset = "utf-8">
        <title></title>
        <style>
            div {
                border: 10px solid green;
                font-size: 60px;
            }
            @media(min-width: 800px) {
                div {
                    display:none
                }
            }
        </style>
    </head>
    <body>
        <div>
            Responsive
        </div>
    </body>
</html>
```

해당 코드는 화면의 크기에 따라서 Responsive라는 텍스트가 나타나고 사라지는 것을 정의한 미디어 쿼리 코드이다. 미디어 쿼리는 `@` 키워드로 시작한다.

```css
@media(min-width: 800px)
```

위 코드는 스크린의 크기가 최소 800px의 크기를 가진다는 것이다.

# 코드 재활용
만약 다수의 HTML 문서에서 동일한 CSS를 적용시켜야 한다면, 별도의 CSS 문서를 만들고 그 곳에 코드를 정의한 다음에 `link` 태그를 사용하여  CSS 정의 파일을 불러오면 된다. 만일 `style.css` 파일이 존재한다고 가정한다면

```html
<link rel = "stylesheet" href = "style.css">
```
다음과 같이 정의된다. 