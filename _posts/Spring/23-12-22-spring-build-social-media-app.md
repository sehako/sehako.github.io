---
title:  "[Spring] REST API로 소셜 미디어 어플 빌드 -1"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-22
---

# 사용자와 관련된 REST API

키 값, 이름, 생일을 가진 데이터 클래스 작성

```java
public class User {
    private Integer id;
    private String name;
    private LocalDate birthDate;

    // 생성자
    // getter and setter
    // toString
}

```

전체 사용자 조회, 특정 사용자를 id값을 통해 조회, 사용자 추가, 삭제 서비스 작성

```java
@Service
public class UserDaoService {
    private static List<User> users = new ArrayList<>();

    private static int userCount = 0;

    // 현재는 DB가 없으므로 리스트에 사용자 추가
    static {
        users.add(new User(++userCount, "A", LocalDate.now().minusYears(20)));
        users.add(new User(++userCount, "B", LocalDate.now().minusYears(23)));
        users.add(new User(++userCount, "C", LocalDate.now().minusYears(25)));
    }

    public List<User> findAll() {
        return users;
    }

    public User findUserById(int id) {
        Predicate<? super User> predicate = user -> user.getId().equals(id);
        return users.stream().filter(predicate).findFirst().orElse(null);
    }

    public User save(User user) {
        user.setId(++userCount);
        users.add(user);
        return user;
    }

    public boolean deleteById(int id) {
        Predicate<? super User> predicate = user -> user.getId().equals(id);
        return users.removeIf(predicate);
    }
}
```

해당 서비스를 통해 요청의 처리를 수행하는 REST API 작성

```java
@RestController
public class UserResource {

    private UserDaoService service;

    // 생성자 주입
    public UserResource(UserDaoService service) {
        this.service = service;
    }

    @GetMapping(path = "/users")
    public List<User> getAllUsers() {
        return service.findAll();
    }

    @GetMapping(path = "/users/{id}")
    public User getUser(@PathVariable int id) {
        User user = service.findUserById(id);
        return user;
    }

    @PostMapping(path = "/users")
    public ResponseEntity<User> createUser(@Valid @RequestBody User user) {
        User savedUser = service.save(user);
        return service.findAll();
    }

    @DeleteMapping(path = "/users/{id}")
    public List<User> deleteUser(@PathVariable int id) {
        if(service.deleteById(id)) {
            return service.findAll();
        }
        return service.findAll();
    }
}
```

# 적절한 응답 반환

사용자를 생성했을 때, 적절한 상태코드는 201이지만 현재는 200만을 반환

사용자 생성 성공 시 201을 반환하도록 코드 수정

```java
@PostMapping(path = "/users")
public ResponseEntity<User> createUser(@Valid @RequestBody User user) {
    User savedUser = service.save(user);

    //생성된 리소스의 주소값을 응답값 헤더에 넣어서 반환
    URI location = ServletUriComponentsBuilder.
            fromCurrentRequest().
            path("/{id}").
            buildAndExpand(savedUser.getId()).toUri();
    // 201 응답을 반환하도록 함
    return ResponseEntity.created(location).build();
}
```

# 예외 처리

위 코드에서 유효하지 않은 id를 입력하여도 200 값을 반환

존재하지 않는 사용자를 GET 요청했을 때 404 에러를 반환하도록 수정

```java
@GetMapping(path = "/users/{id}")
public User getUser(@PathVariable int id) {
    User user = service.findUserById(id);
    // 존재하는 사용자가 없다면 404에러 반환
    if(user == null) {
        throw new UserNotFoundException("id: " + id);
    }
    return user;
}
```

```java
@ResponseStatus(code = HttpStatus.NOT_FOUND)
public class UserNotFoundException extends RuntimeException {
    public UserNotFoundException(String message) {
        super(message);
    }
}
```

`HttpStatus`로 처리에 대한 응답을 설정할 수 있음

## 커스텀 예외 구조

직접 `ResponseEntityExceptionHandler`를 상속받아 메소드 오버라이딩으로 커스텀 예외 구조를 만들 수 있음

### 예외 구조 데이터 클래스 정의

현재 날짜와 시간, 메시지, 세부사항을 구조로 갖도록 정의

```java
public class ErrorDetails {
    private LocalDateTime timestamp;
    private String message;
    private String details;

    public ErrorDetails(LocalDateTime timestamp, String message, String details) {
        this.timestamp = timestamp;
        this.message = message;
        this.details = details;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }

    public String getMessage() {
        return message;
    }

    public String getDetails() {
        return details;
    }
}
```

### 예외 처리 정의

```java
@ControllerAdvice
public class CustomizedResponseEntityExceptionHandler extends ResponseEntityExceptionHandler {
    // 커스텀 예외 구조
    // 모든 예외에 대한 구조 정의
    @ExceptionHandler(Exception.class)
    public final ResponseEntity<ErrorDetails> handleAllException(Exception ex, WebRequest request) throws Exception {
        ErrorDetails errorDetails = new ErrorDetails(LocalDateTime.now(), ex.getMessage(), request.getDescription(false));
        return new ResponseEntity<ErrorDetails>(errorDetails, HttpStatus.INTERNAL_SERVER_ERROR);
    }
    // 잘못된 사용자 검색에 대한 구조 정의
    @ExceptionHandler(UserNotFoundException.class)
    public final ResponseEntity<ErrorDetails> handleUserNotFoundException(Exception ex, WebRequest request) throws Exception {
        ErrorDetails errorDetails = new ErrorDetails(LocalDateTime.now(), ex.getMessage(), request.getDescription(false));
        return new ResponseEntity<ErrorDetails>(errorDetails, HttpStatus.NOT_FOUND);
    }
}
```

# 유효성 검증

사용자를 추가할 때 이름이나 생일을 공백이나 미래로 설정해도 문제없이 추가됨

유효성 검증으로 이를 방지

### 의존성 추가

```
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-validation</artifactId>
</dependency>
```

### 데이터 클래스 수정

User 데이터 클래스 수정

```java
public class User {
    private Integer id;
    @Size(min = 2, message = "최소 2글자 이상이어야 함")
    private String name;
    @Past(message = "생일은 과거여야 함")
    private LocalDate birthDate;
    // ...
}
```

### 커스텀 검증 예외 구조

검증 에러 구조를 커스텀 하기 위해 `CustomizedResponseEntityExceptionHandler`에 다음 메소드 오버라이딩

```java
@Override
@Nullable
protected ResponseEntity<Object> handleMethodArgumentNotValid(MethodArgumentNotValidException ex, HttpHeaders headers, HttpStatusCode status, WebRequest request) {
    ErrorDetails errorDetails = new ErrorDetails(
            LocalDateTime.now(),
            "Total Errors: " + ex.getErrorCount() + " First Error: " + ex.getFieldError().getDefaultMessage(),
            request.getDescription(false)
    );
    // ex.getFieldError().getDefaultMessage(): 에러가 여럿 있어도 하나의 에러만을 불러옴
    // ex.getErrorCount(): 모든 에러의 개수
    return new ResponseEntity(errorDetails, HttpStatus.BAD_REQUEST);
}

@Override
@Nullable
protected ResponseEntity<Object> handleHandlerMethodValidationException(HandlerMethodValidationException ex, HttpHeaders headers, HttpStatusCode status, WebRequest request) {
    return this.handleExceptionInternal(ex, (Object)null, headers, status, request);
}
```