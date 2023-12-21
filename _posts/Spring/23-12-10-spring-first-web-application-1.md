---
title:  "[Spring] 웹 어플리케이션 빌드 - 1"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-10
---

[강의](https://www.udemy.com/course/spring-boot-and-spring-framework-korean/)를 통해 클론할 웹 앱의 목표는 로그인 기능 구현과 할 일(todo)리스트를 출력하는 것이다.

# 로그인 기능 구현

## jsp 작성

**loginForm.jsp**

```jsp
<html>
    <head>
        <title>User Login</title>
    </head>
    <body>
        <h2>Login Page</h2>
        <pre>${error}</pre>
        <form method="post">
            UserName: <input type="text" name="username">
            Password: <input type="password" name="password">
            <button type="submit">Submit</button>
        </form>
    </body>
</html>
```

## 라우팅 설정

```java
@Controller
public class LoginController {

    @RequestMapping(value = "login", method = RequestMethod.GET)
    public String login() {
        return "loginForm";
    }
}
```

jsp에서 `form`은 보안(`get` 요청은 쿼리 스트링으로 주소 값에 입력값이 보이게 됨)을 위해 `post`로 요청을 보내기 때문에 이 상태로 앱을 실행하면 오류가 생긴다. `post` 요청을 처리하는 메소드와 리턴할 jsp를 만든다.

**welcome.jsp**

```jsp
<html>
    <head>
        <title>Welcome!</title>
    </head>
    <body>
        <div>welcome ${username}</div>
    </body>
</html>
```

```java
//POST 요청을 받으면 다음 jsp를 리턴
@RequestMapping(
        value = "login",
        method = RequestMethod.POST
)
public String welcome() {
    return "welcome";
}
```

## 인증 구현

더미 아이디와 패스워드를 설정하고 인증 로직을 구현하였다. 단일 책임 원칙에 따라 새로운 클래스를 만들어 작성하였다.

```java
@Service
public class AuthenticationService {
    public boolean authenticate(String username, String password) {
        boolean isValidUserName = username.equalsIgnoreCase("Test");
        boolean isValidPassword = password.equalsIgnoreCase("password");

        return isValidUserName && isValidPassword;
    }
}
```

```java
@Controller
public class LoginController {
    private AuthenticationService authenticationService;

    public LoginController(AuthenticationService authenticationService) {
        this.authenticationService = authenticationService;
    }

    //... 
    @RequestMapping(
            value = "login",
            method = RequestMethod.POST
    )
    public String welcome(
            @RequestParam String username,
            @RequestParam String password,
            ModelMap model
    ) {
        model.put("username", username);
        boolean isValid = authenticationService.authenticate(username, password);

        if(isValid) {
            return "welcome";
        }
        model.put("error", "Invalid User");
        return "loginForm";
    }
}
```

# 할 일 리스트 만들기

## 데이터 클래스 작성

```java
public class Todo {
    private int id;
    private String username;
    private String description;
    private LocalDate targetDate;
    private boolean done;

    public Todo(int id, String username, String description, LocalDate targetDate, boolean done) {
        this.id = id;
        this.username = username;
        this.description = description;
        this.targetDate = targetDate;
        this.done = done;
    }

    // Getter and Setter...

    @Override
    public String toString() {
        return "Todo{" +
                "id=" + id +
                ", username='" + username + '\'' +
                ", description='" + description + '\'' +
                ", targetDate=" + targetDate +
                ", done=" + done +
                '}';
    }
}
```

## 서비스 작성

```java
@Service
public class TodoService {
    private static List<Todo> todos = new ArrayList<>();
    static {
        todos.add(new Todo(1, "Test", "Learn Spring", LocalDate.now().plusYears(1), false));
        todos.add(new Todo(2, "Test", "Learn Docker", LocalDate.now().plusYears(1), false));
    }
}
```

## jsp 작성

**listTodos.jsp**

```jsp
<html>
    <head>
        <title>Todo list</title>
    </head>
    <body>
    <div>Your todos are ${todos}</div>
    </body>
</html>
```

## 컨트롤러 작성

```java
@Controller
public class TodoController {
    private TodoService todoService;
    public TodoController(TodoService todoService) {
        this.todoService = todoService;
    }

    @RequestMapping("todo-list")
    public String listAllTodos(ModelMap model) {
        List<Todo> todos = todoService.findByUserName("Test");
        model.put("todos", todos);
        return "listTodos";
    }
}
```

# 세션

요청의 범위는 항상 단일 요청에 대한 처리이다. 응답이 다시 전송되면, 요청 속성은 메모리에서 지워지게 된다. 가령 앞서 **welcome.jsp**에서 `username`을 모델로 전송하였다. 

해당 페이지 내에서 **listTodos.jsp**로 이동하게 되고 그곳에서 `username`을 다시 사용하려 하면 사용하지 못한다. 이미 그 값이 메모리에서 지워진 상태이기 때문이다.

세션을 이용하면 그 범위를 확장할 수 있다.

```java
@Controller
@SessionAttributes("username")
public class LoginController {
    //...
    @RequestMapping(
            value = "login",
            method = RequestMethod.POST
    )
    public String welcome(
            @RequestParam String username,
            @RequestParam String password,
            ModelMap model
    ) {
        //...
        model.put("username", username);
        //...
    }
}
```

값을 사용할 모든 클래스에 어노테이션을 적어야 한다.

```java
@Controller
@SessionAttributes("username")
public class TodoController {
    //...
}
```

대부분의 상황에서는 세션 범위보다는 요청 범위를 사용하는 것이 추천된다.