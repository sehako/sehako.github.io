---
title:  "[Spring] 웹 어플리케이션 빌드 - 4"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-19
---

# Spring Security

현재는 할 일 목록을 가지는 주소값을 입력하면 아무런 로그인 검증 없이 할 일 목록에 접근이 가능하다.

이를 방지하기 위해 보안 기능을 사용한다. 

## 사전준비 

앞서 로그인 기능을 구현하려고 로그인 컨트롤러를 삭제하고 다음 컨트롤러를 하나 생성한다.

```java
@Controller
@SessionAttributes("username")
public class WelcomeController {
    @RequestMapping(value = "/", method = RequestMethod.GET)
    public String showWelcomePage(ModelMap model) {
        model.put("name", "Test");
        return "welcome";
    }
}
```

## 의존성 추가

다음 의존성을 추가하면 자동으로 홈에 접근 시 인증되지 않은 사용자는 로그인 페이지로 이동된다.

```
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-security</artifactId>
</dependency>
```

## 사용자 아이디, 패스워드 설정

```java
@Configuration
public class SpringSecurityConfiguration {
    @Bean
    public InMemoryUserDetailsManager createUserDetailManager() {
        UserDetails userDetails1 = getUserDetails("Test", "password");
        UserDetails userDetails2 = getUserDetails("TTT", "pppppp");

        return new InMemoryUserDetailsManager(userDetails1, userDetails2);
    }

    private UserDetails getUserDetails(String username, String password) {
        Function<String, String> passwordEncoder
                = input -> passwordEncoder().encode(input);
        UserDetails userDetails = User.builder()
                .passwordEncoder(passwordEncoder)
                .username(username)
                .password(password)
                .roles("USER", "ADMIN")
                .build();
        return userDetails;
    }

    @Bean
    public PasswordEncoder passwordEncoder() {
        return new BCryptPasswordEncoder();
    }
}
```

## 아이디 가져오기

사용자 아이디를 가져오는 방법

```java
private String getLoggedInUsername() {
    Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
    return authentication.getName();
}
```


# h2 데이터 베이스 사용

의존성 추가

```
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-data-jpa</artifactId>
</dependency>
<dependency>
    <groupId>com.h2database</groupId>
    <artifactId>h2</artifactId>
    <scope>runtime</scope>
</dependency>
```

스프링 시큐리티에 의해 오류 발생

1. 모든 url이 보호됨
2. 승인되지 않은 접근에 대해서 로그인 창을 리턴

사이트 간 요청 위조를 비활성화하고 프레임을 허용

**SpringSecurityConfiguration.class**에 다음 메소드를 추가

```java
@Bean
public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
    http.authorizeHttpRequests(
            auth -> auth.anyRequest().authenticated()
    );
    http.formLogin(withDefaults());

    http.csrf().disable();
    http.headers().frameOptions().disable();
    return http.build();
}
```

## 엔티티 설정

데이터 클래스를 엔티티로 설정

```java
@Entity
public class Todo {
    @Id
    @GeneratedValue
    private int id;

    // 기본 생성자가 필요
    public Todo() {}
    // ...
}
```

## sql 파일로 데이터 추가 테스트

```sql
insert into todo (ID, USERNAME, DESCRIPTION, TARGET_DATE, DONE)
values(10001, 'TEST', 'Get a job', CURRENT_DATE(), false);
```

테이블이 생성되기 전에 해당 sql이 실행되므로 오류 발생 

다음 설정 추가

```
spring.jpa.defer-datasource-initialization=true
```

## 리포지토리 인터페이스 생성

```java
public interface TodoRepository extends JpaRepository<Todo, Integer> {
    public List<Todo> findByUsername(String username);
}
```

## 컨트롤러에 적용

### 할 일 리스트

```java
@Controller
@SessionAttributes("username")
public class TodoControllerJpa {
    public TodoControllerJpa(TodoRepository todoRepository) {
        this.todoRepository = todoRepository;
    }

    private TodoRepository todoRepository;

    @RequestMapping("todo-list")
    public String listAllTodos(ModelMap model) {
        String username = getLoggedInUsername();
        List<Todo> todos = todoRepository.findByUsername(username);
        model.put("todos", todos);
        return "listTodos";
    }
}
```

### 할 일 추가

```java
@RequestMapping(value="add-todo", method = RequestMethod.POST)
public String addNewTodo(ModelMap model, @Valid Todo todo, BindingResult result) {
    if(result.hasErrors()) {
        return "todo";
    }
    String username = (String)getLoggedInUsername();
    // 리포지토리는 모든 속성을 받는 메소드가 없기 때문에 todo에 사용자 아이디를 직접 설정
    // save는 todo만 받음
    todo.setUsername(username);
    todoRepository.save(todo);
    return "redirect:todo-list";
}
```

### 할 일 업데이트

```java
@RequestMapping(value="update-todo", method = RequestMethod.GET)
public String showUpdateTodoPage(@RequestParam int id, ModelMap model) {
    String username = (String)getLoggedInUsername();
    // Optional을 리턴하기 때문에 .get()이 필요
    Todo todo = todoRepository.findById(id).get();
    model.addAttribute("todo", todo);
    return "todo";
}

@RequestMapping(value="update-todo", method = RequestMethod.POST)
public String updateTodo(ModelMap model, @Valid Todo todo, BindingResult result) {
    if(result.hasErrors()) {
        return "todo";
    }
    String username = (String)getLoggedInUsername();
    todo.setUsername(username);
    todoRepository.save(todo);
    return "redirect:todo-list";
}
```

### 할 일 삭제

```java
@RequestMapping(value="delete-todo")
public String deleteTodo(@RequestParam int id) {
    todoRepository.deleteById(id);
    return "redirect:todo-list";
}
```

# Docker로 MySQL 연결

Docker 실행 후 cmd에서 다음 명령어로 sql을 실행(?)

```
docker run --detach --env MYSQL_ROOT_PASSWORD=dummypassword --env MYSQL_USER=todos-user --env MYSQL_PASSWORD=dummytodos --env MYSQL_DATABASE=todos --name mysql --publish 3306:3306 mysql:8-oracle
```

h2 데이터베이스 의존성을 다음 sql 의존성으로 변경
```
<dependency>
    <groupId>mysql</groupId>
    <artifactId>mysql-connector-java</artifactId>
    <version>8.0.33</version>
</dependency>
```

**application.properties**에서 다음 설정들을 작성

```
spring.datasource.url=jdbc:mysql://localhost:3306/todos
spring.datasource.username=todos-user
spring.datasource.password=dummytodos
spring.jpa.properties.hibernate.dialect=org.hibernate.dialect.MySQLDialect

# 시작할 때 테이블 생성
spring.jpa.hibernate.ddl-auto=update
```

