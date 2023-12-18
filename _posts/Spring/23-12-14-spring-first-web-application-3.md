---
title:  "[Spring] 웹 어플리케이션 빌드 - 3"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-14
---

# 할 일 삭제 기능 구현  

## 표 요소 추가

```html
<td><a href="delete-todo?id=${todo.id}" class="btn btn-warning">DELETE</a></td>
```

## 서비스에 메소드 추가

```java
public void deleteById(int id) {
    // 람다로 구현
    // 모든 id를 대상으로 아래 조건을 실행
    Predicate<? super Todo> predicate = todo -> todo.getId() == id;
    todos.removeIf(predicate);
}
```

`Predicate`는 해당되는 모든 객체를 하나씩 비교해보고 조건에 따라 `boolean`값을 반환

## 컨트롤러 메소드 추가

```java
@RequestMapping(value="delete-todo")
public String deleteTodo(@RequestParam int id) {
    todoService.deleteById(id);
    return "redirect:todo-list";
}
```

# 할 일 업데이트 기능 구현

## 표 요소 추가

```html
<td><a href="update-todo?id=${todo.id}" class="btn btn-success">UPDATE</a></td>
```

## 서비스에 메소드 추가

```java
public Todo findById(int id) {
    Predicate<? super Todo> predicate = todo -> todo.getId() == id;
    return todos.stream().filter(predicate).findFirst().get();
}

public void updateTodo(@Valid Todo todo) {
    deleteById(todo.getId());
    todos.add(todo);
}
```

## 컨트롤러 추가

```java
@RequestMapping(value="update-todo", method = RequestMethod.GET)
public String showUpdateTodoPage(@RequestParam int id, ModelMap model) {
    String username = (String)model.get("name");
    Todo todo = todoService.findById(id);
    model.addAttribute("todo", todo);
    return "todo";
}

@RequestMapping(value="update-todo", method = RequestMethod.POST)
public String updateTodo(ModelMap model, @Valid Todo todo, BindingResult result) {
    if(result.hasErrors()) {
        return "todo";
    }
    String username = (String)model.get("name");
    todo.setUsername(username);
    todoService.updateTodo(todo);
    return "redirect:todo-list";
}
```

# 날짜 기능 수정

## JSP 수정

날짜 기능을 컨트롤러에 매핑하기 위해 `input` 태그를 하나 만든다. 또한 같은 역할을 가지고 있는 태그를 `fieldset`으로 묶는다.

```html
<fieldset class="mb-3">
    <form:label path="targetDate">Target Date</form:label>
    <form:input type="text" path="targetDate" required="required" />
    <form:errors path="targetDate" cssClass="text-warning" />
</fieldset>
```

## 컨트롤러 수정

```java
@RequestMapping(value="add-todo", method = RequestMethod.GET)
public String showNewTodoPage(ModelMap model) {
    String username = (String)model.get("name");
    Todo todo = new Todo(0, username, "", LocalDate.now(), false);
    model.addAttribute("todo", todo);
    return "todo";
}

@RequestMapping(value="add-todo", method = RequestMethod.POST)
public String addNewTodo(ModelMap model, @Valid Todo todo, BindingResult result) {
    if(result.hasErrors()) {
        return "todo";
    }
    String username = (String)model.get("name");
    todoService.addTodo(username, todo.getDescription(), todo.getTargetDate(), false);
    //리다이렉트
    return "redirect:todo-list";
}
```

`todo.getTargetDate()`로 jsp의 요소값을 가져온다.


# 달력을 이용하여 날짜 선택

사용자가 직접 입력하면 잘못된 값을 입력할 수도 있다. 따라서 요소를 클릭하면 달력이 나와 그 달력을 선택하고 추가하도록 한다.

## 의존성 추가

```
<dependency>
    <groupId>org.webjars</groupId>
    <artifactId>bootstrap-datepicker</artifactId>
    <version>1.10.0</version>
</dependency>
```

## jsp 수정

프레임워크를 불러오고 자바 스크립트 코드를 작성한다.

```html
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="form" uri="http://www.springframework.org/tags/form" %>

<html>
    <head>
        <!-- ... -->
        <link href="webjars/bootstrap-datepicker/1.10.0/css/bootstrap-datepicker.standalone.css" rel="stylesheet">
    </head>
    <body>
        <!-- ... -->
        <script src="webjars/bootstrap-datepicker/1.10.0/js/bootstrap-datepicker.min.js"></script>
        <script type="text/javascript">
            $("#targetDate").datepicker({
                format: 'yyyy-mm-dd',
            });
        </script>
    </body>
</html>
```