---
title:  "[Spring] 웹 어플리케이션 빌드 - 2"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-14
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

```jsp
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

```jsp
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


# 할 일 추가 기능 구현

**listTodos.jsp**에 버튼 생성

```jsp
<!-- ... -->
<a href="add-todo" class="btn btn-success">Add Todo</a>
```

**todo.jsp** 설정

```jsp
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>

<html>
    <head>
        <link href="webjars/bootstrap/5.3.2/css/bootstrap.min.css" rel="stylesheet">
        <title>Todo list</title>
    </head>
    <body>
    <div class="container">
        <h1>Todo Details</h1>
        <form method="post">
            Description: <input type="text" name="description" required="required" />
            <input type="submit" class="btn btn-success" />
        </form>
    </div>
    <script src="webjars/bootstrap/5.3.2/js/bootstrap.min.js"></script>
    <script src="webjars/jquery/3.7.1/js/jquery.min.js"></script>
    </body>
</html>
```

## 서비스에 메소드 추가

```java
private static int todosCount = 0;

public void addTodo(String username, String description, LocalDate targetDate, boolean isDone) {
    Todo todo = new Todo(++todosCount, username, description, targetDate, isDone);
    todos.add(todo);
}
```

id를 추가한 개수만큼 자동적으로 갱신하기 위해서 약간의 수정을 거치고 메소드 추가

## 라우팅 설정

```java
@RequestMapping(value="add-todo", method = RequestMethod.GET)
public String showNewTodoPage() {
    return "todo";
}

@RequestMapping(value="add-todo", method = RequestMethod.POST)
public String addNewTodo(@RequestParam String description, ModelMap model) {
    String username = (String)model.get("name");
    todoService.addTodo(username, description, LocalDate.now().plusYears(1), false);
    //리다이렉트
    return "redirect:todo-list";
}
```

`redirect:`을 이용하면 기존에 반환한 jsp를 선택해 반환할 수 있다.

## 스프링 부트를 이용한 검증

앞서 `required` 속성을 이용하여 html 태그 내에서 간단한 검증을 구현하였지만 보안을 고려하면 바람직하지는 않다.

스프링 검증 모듈을 통한 방법으로 보안을 강화한다.

### 의존성 추가

```
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-validation</artifactId>
</dependency>
```

### 데이터 클래스 수정 

데이터 클래스의 특정 변수의 최소 입력 길이가 5 이상이어야 한다면 다음 어노테이션만 추가해주면 된다.

```java
@Size(min = 5, message = "Under 5 characters")
private String value;
```

`message` 변수는 검증에 통과하지 못했을 때 나타나는 문구를 지정한 것이다.

### 커멘드 빈 구현

```java
@RequestMapping(value="add-todo", method = RequestMethod.GET)
public String showNewTodoPage(ModelMap model) {
    String username = (String)model.get("name");
    Todo todo = new Todo(0, username, "", LocalDate.now().plusYears(1), false);
    model.put("todo", todo);
    return "todo";
}

@RequestMapping(value="add-todo", method = RequestMethod.POST)
public String addNewTodo(ModelMap model, @Valid Todo todo, BindingResult result) {
    if(result.hasErrors()) {
        return "todo";
    }
    String username = (String)model.get("name");
    todoService.addTodo(username, todo.getDescription(), LocalDate.now().plusYears(1), false);
    //리다이렉트
    return "redirect:todo-list";
}
```

`@Valid`는 해당 데이터 클래스를 검증 과정에 넣는다는 것이다.

`BindingResult`는 검증의 결과를 가지는 인터페이스다. `hasErrors()`는 검증이 실패했다면 `true`를 반환한다.

### JSP 수정

```jsp
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="form" uri="http://www.springframework.org/tags/form" %>

<html>
    <head>
        <link href="webjars/bootstrap/5.3.2/css/bootstrap.min.css" rel="stylesheet">
        <title>Todo list</title>
    </head>
    <body>
    <div class="container">
        <h1>Todo Details</h1>
        <form:form method="post" modelAttribute="todo">
            Description: <form:input type="text" path="description" required="required" />
                        <form:errors path="description" />
            <form:input type="hidden" path="id" />
            <form:input type="hidden" path="done" />
            <input type="submit" class="btn btn-success" />
        </form:form>
    </div>
    <script src="webjars/bootstrap/5.3.2/js/bootstrap.min.js"></script>
    <script src="webjars/jquery/3.7.1/js/jquery.min.js"></script>
    </body>
</html>
```

몇몇 태그에 스프링 태그 라이브러리를 사용하여 수정하였다. 

`modelAttribute`로 라우트 함수 내 같은 이름을 가진 데이터 클래스를 매핑한다. 그렇기 때문에 `GET`을 담당하는 함수 내에서도 `todo` 데이터 클래스가 필요하여 임시로 빈 `Todo` 객체를 생성하여 넣어준 것이다.

`path`를 사용하여 라우트 함수 내 같은 이름의 변수를 매핑한다.

`form:errors`는 검증이 실패 시 기본 오류 페이지로 넘어가는 것이 아닌 개발자가 설정한 검증 실패 알림 문구를 띄우는 태그이다.

`hidden` 속성을 가진 두 개의 입력 태그는 검증을 회피하기 위한 수단이다. 검증 모듈은 `Null`값을 자동으로 감지하기 때문에 숨겨진 입력 태그를 만들고 `path`를 통해 라우트 함수 내 변수에 매핑하여 검증을 회피하였다.