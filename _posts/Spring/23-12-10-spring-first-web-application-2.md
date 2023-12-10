---
title:  "[Spring] 웹 어플리케이션 빌드 - 2"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-10
---

# JSTL

앞서 할 일 목록을 보기 위해 `${todos}`라는 키값으로 단순히 jsp에 입력하였다. 그 결과는 다음과 같다.

```
Your todos are [Todo{id=1, username='Test', description='Learn Spring', targetDate=2024-12-10, done=false}, Todo{id=2, username='Test', description='Learn Docker', targetDate=2024-12-10, done=false}]
```

이걸 리스트화 하기 위해 jsp내에서 다양한 제어 작업을 지원하는 태그 라이브러리인 [JSTL](https://docs.oracle.com/javaee/5/jstl/1.1/docs/tlddocs/c/tld-summary.html)을 사용한다.

## 의존성 추가

```
<dependency>
    <groupId>jakarta.servlet.jsp.jstl</groupId>
    <artifactId>jakarta.servlet.jsp.jstl-api</artifactId>
</dependency>
<dependency>
    <groupId>org.eclipse.jetty</groupId>
    <artifactId>glassfish-jstl</artifactId>
    <version>11.0.18</version>
</dependency>
```

각종 [의존성](https://mvnrepository.com/)의 최신 버전을 정의한 페이지가 있다.

## JSP 수정

**listTodos.jsp**

```html
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<html>
    <head>
        <title>Todo list</title>
    </head>
    <body>
    <div>Your todos are</div>
    <table>
        <thead>
            <tr>
                <th>Id</th>
                <th>Description</th>
                <th>Target Date</th>
                <th>Is Done?</th>
            </tr>
        </thead>
        <tbody>
            <c:forEach items="${todos}" var="todo">
                <tr>
                    <td>${todo.id}</td>
                    <td>${todo.description}</td>
                    <td>${todo.targetDate}</td>
                    <td>${todo.done}</td>
                </tr>
            </c:forEach>
        </tbody>
    </table>
    </body>
</html>
```

할 일 목록이 다음과 같이 만들어진다.

```
Id	Description	Target Date	Is Done?
1	Learn Spring	2024-12-10	false
2	Learn Docker	2024-12-10	false
```

# CSS 사용

jsp파일에 css를 적용하기 위해 bootstrap CSS 프레임워크를 사용한다.

## 의존성 추가

```
<dependency>
    <groupId>org.webjars</groupId>
    <artifactId>bootstrap</artifactId>
    <version>5.3.2</version>
</dependency>
<dependency>
    <groupId>org.webjars</groupId>
    <artifactId>jquery</artifactId>
    <version>3.7.1</version>
</dependency>
```

## JSP 수정

```html
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<html>
    <head>
        <link href="webjars/bootstrap/5.3.2/css/bootstrap.min.css" rel="stylesheet">
        <title>Todo list</title>
    </head>
    <body>
    <div class="container">
        <div>Your todos are</div>
        <table class="table">
            <!-- 할일 목록 -->
        </table>
    </div>
    <script src="webjars/bootstrap/5.3.2/js/bootstrap.min.js"></script>
    <script src="webjars/jquery/3.7.1/js/jquery.min.js"></script>
    </body>
</html>
```

bootstrap은 각 태그에 대한 클래스를 지원한다. `body`의 전체 내용은 `<div class = "container"></div>`로 감싸는 것이 권장된다. 