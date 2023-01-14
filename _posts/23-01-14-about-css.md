---
title:  "[CSS] 강의 정리"
excerpt: "생활코딩 CSS 강의"

categories:
  - WEB

toc: true
toc_sticky: true
 
date: 2023-01-14
last_modified_at: 2023-01-14
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

**참고용**

|:---:|:---:|
효과|기능
color|텍스트 색상 변경
text-decoration|밑 줄 등 텍스트 자체의 모양에 관여
font-size|글자 크기 변경
text-align|글자 정렬(left, right, center)