---
title: 스프링에서 HTTP 상태 코드를 처리하는 방법

categories:
  - Spring Tip

toc: true
toc_sticky: true
published: true
 
date: 2025-02-02
last_modified_at: 2025-02-02
---

# 스프링에서의 응답

```java
public record User(
    String username,
    int age
) {}
```

다음 객체가 존재할 때 이 객체를 반환한다고 가정해보자. 이때 이 객체를 반환하는 간단한 요청 메소드를 만들면 다음과 같다.

```java
@RestController
public class RequestController {
    @GetMapping("/get-user")
    public User getUser() {
        return new User("test-user", 20);
    }
}
```

스프링에서는 `@RestController`가 있으면 설정에 따라서 알맞는 REST API 응답 형식으로 맞춰서 반환해준다. 요즘에는 JSON 형식이 사실상의 표준 데이터 교환 형식이기 때문에 내부적으로 Jackson 라이브러리를 사용하여 반환 객체를 JSON 값으로 변환하여 응답한다. 

실제로 위의 요청 메소드를 실행시키면 HTTP 상태 코드 200과 함께 다음과 같은 결과를 받을 수 있다.

```json
{
"username": "test-user",
"age": 20
}
```

## HTTP 상태 코드

이때 만약 개발자가 HTTP 상태 코드를 제어하고 싶다면 어떻게 해야 할까? 예를 들어 어떤 `User` 객체를 만드는 POST 요청이 있다고 한다면, 해당 HTTP 상태 코드는 201 Created가 적절할 것이다. 

### @ResponseStatus

스프링에서는 이런 문제를 `@ResponseStatus`라는 어노테이션을 통해서 해결할 수 있다.

```java
@ResponseStatus(HttpStatus.CREATED)
@PostMapping("/create-user")
public User createUser(
        @RequestBody User user
) {
    return user;
}
```

이를 통해서 다음과 같은 요청을 보냈을 때

```
POST /create-user HTTP/1.1
Host: localhost:8080
Content-Type: application/json
Content-Length: 41

{
	"username": "created-user",
	"age": 20
}
```

201 Created HTTP 상태 코드와 함께 다음과 같은 응답을 서버로부터 받을 수 있는 것을 볼 수 있다.

```json
{
"username": "created-user",
"age": 20
}
```

### ResponseEntity<T>

이때 서버에서 응답 값 헤더에 어떠한 처리를 하고자 한다고 가정해보자. 예를 들어 위의 `User`를 생성하는 POST 요청에서, 클라이언트가 만들어진 사용자를 조회할 수 있는 URI로 이동하도록 헤더에 어떤 값을 넣어달라고 하는 것이다. 

이런 상황에서는 단순하게 `@ResponseStatus`를 사용하는 것으로 해결할 수 없다. 이때에는 `ResponseEntity<T>`를 사용한다. 사용 방법은 다음과 같다.

```java
@PostMapping("/create-user/v2")
public ResponseEntity<User> createUserV2(
        @RequestBody User user
) {
    return ResponseEntity.created(URI.create("/search-user/" + user.username())).body(user);
}
```

`created()`는 자동으로 응답 상태 코드를 201 Created로 만들어줌과 동시에, 응답 헤더에 `Location`이라는 필드에 새로 만들어진 자원을 조회할 수 있는 URI를 설정할 수 있다. 

또한 응답 메시지 바디에 `body()`라는 메소드를 이용하여 원하는 응답 값을 넣을 수 있다. 이때 정의한 제네릭 값이 바로 `body()`에 들어갈 객체가 되는 것이다.

## ResponseEntity 사용 이유 & 사용법

그렇다면 둘 중 어느것을 사용해야 할까? 나는 개인적으로 `ResponseEntity<T>`를 선호하는데, 그 이유는 헤더에 마땅한 값을 넣지 않는다 하더라도, `@ResponseStatus`는 HTTP 응답 상태 코드를 제어하는 단순한 기능으로 끝나기 때문에 결국 자세한 응답을 정의하기 위해서 `ResponseEntity<T>`를 필연적으로 사용할 수 밖에 없다고 생각하기 때문이다.

물론 단순한 응답에는 `@ResponseStatus`를, 자세한 응답에는 `ResponseEntity<T>`를 사욯하는 것도 좋은 선택일 수는 있지만, 이는 코드의 통일성을 망친다고 생각한다. 위의 두 요청 메소드를 모아서 보도록하자.

```java
@ResponseStatus(HttpStatus.CREATED)
@PostMapping("/create-user")
public User createUser(
        @RequestBody User user
) {
    return user;
}

@PostMapping("/create-user/v2")
public ResponseEntity<User> createUserV2(
        @RequestBody User user
) {
    return ResponseEntity.created(URI.create("/search-user/" + user.username())).body(user);
}
```

지금이야 메소드가 두 개 밖에 없기 때문에 이해가 쉽지만, 이런 식의 혼용이 계속해서 이루어진다면 다른 개발자가 이러한 코드를 봤을 때 헷갈릴 수 있다고 생각한다.

또한 `ResponseEntity<T>`를 사용한다면 개발이 용이해질 수 있다고 생각한다. 개발을 하다보면 `enum`을 이용해서 REST API 별도 정의 코드, HTTP 상태 코드, 메시지를 보여주게 된다.

```java
@RequiredArgsConstructor
public enum SuccessCode {
    REQUEST_SUCCESS(HttpStatus.OK, 2000, "요청이 성공적으로 처리되었습니다"),
    USER_CREATION_SUCCESS(HttpStatus.CREATED, 2002, "사용자가 성공적으로 생성되었습니다")
    ;

    private final HttpStatus status;
    // 이 코드 값에 따라서 클라이언트가 특정 처리를 하도록 유도할 수 있음
    private final int code;
    private final String message;
}
```

이런 식으로 `enum`을 정의할 때, `@ResponseStatus`를 사용하여 응답 구조를 설계하는 것은 상당히 번거로울 것이다. 그 이유는 위의 enum 값에 이미 `HttpStatus.OK`가 정의되어 있는데, 또 다시 `@ResponseStatus`에 별도로 `HttpStatus.OK`를 명시해 줘야 하기 때문이다.

이때 이런 `enum`을 활용하면서 `ResponseEntity`를 사용하여 응답하고자 한다면 다음과 같은 응답 객체를 만들어서 `ResponseEntity<T>`에 정의하여 사용할 수 었다.

```java
@JsonPropertyOrder({"isSuccess", "code", "message", "result"})
public record ApiResponse<T>(
        @JsonProperty(value = "isSuccess") boolean isSuccess,
        int code,
        String message,
        @JsonInclude(Include.NON_NULL) T result
) {

    public static <T> ApiResponse<T> onSuccess(T data) {
        SuccessCode code = SuccessCode.REQUEST_SUCCESS;
        return new ApiResponse<>(true, code.getCode(), code.getMessage(), data);
    }

    public static <T> ApiResponse<T> of(SuccessCode successCode, T data) {
        return new ApiResponse<>(true, successCode.getCode(), successCode.getMessage(), data);
    }

    public static <T> ApiResponse<T> onFailure(FailureCode errorCode, T data) {
        return new ApiResponse<>(false, errorCode.getCode(), errorCode.getMessage(), data);
    }
}
```

위의 객체를 이용하여 다음과 같은 요청 메소드를 작성할 수 있다.

```java
@PostMapping("/create-user/v3")
public ResponseEntity<ApiResponse<User>> createUserV3(
        @RequestBody User user
) {
    return ResponseEntity.created(URI.create("/search-user/" + user.username())).body(
            ApiResponse.of(SuccessCode.USER_CREATION_SUCCESS, user)
    );
}
```

그러면 요청에 대한 결과로 201 Created와 다음과 같은 응답 값이 반환된다.

```json
{
"isSuccess": true,
"code": 2002,
"message": "사용자가 성공적으로 생성되었습니다",
"result":{
    "username": "created-user",
    "age": 20
}
}
```

### 예외 처리에서의 사용법

응답 성공시에는 앞서 봤던 것 처럼 적절한 응답 상태에 따라서 `ApiResponse`의 정적 메소드를 호출하는 방식이다. 

하지만 서버는 응답 도중 발생한 예외로 인한 처리 실패에 대해서 클라이언트에게 알릴 의무가 있는데, 스프링에서는 이런 예외를 한 곳에 모아서 처리할 수 있도록 `@RestControllerAdvice`라는 어노테이션을 지원한다. 그러면 다음 `enum`과 이를 처리하는 예외를 살펴보자.

```java
@Getter
@RequiredArgsConstructor
public enum FailureCode {
    SERVER_ERROR(HttpStatus.INTERNAL_SERVER_ERROR, 5000, "서버 내부 오류가 발생했습니다"),
    BAD_REQUEST(HttpStatus.BAD_REQUEST, 4000, "잘못된 요청 형식입니다"),
    INVALID_REQUEST(HttpStatus.BAD_REQUEST, 4001, "유효하지 않은 요청입니다"),
    ;

    private final HttpStatus status;
    private final int code;
    private final String message;
}
```

예외

```java
@Getter
public class CommonException extends RuntimeException {
    private final FailureCode failureCode;

    public CommonException(FailureCode failureCode) {
        super(failureCode.getMessage());
        this.failureCode = failureCode;
    }
}
```

그러면 서버에서 예외를 처리하는 코드의 예시를 살펴보자.

```java
@Slf4j
@RestControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(CommonException.class)
    public ResponseEntity<Object> handleCommonException(final CommonException e) {
        log.error("exception = {}, message = {}", e.getClass().getName(), e.getMessage());
        return ResponseEntity.status(e.getFailureCode().getStatus()).body(ApiResponse.onFailure(e.getFailureCode(),null));
    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity<ApiResponse<Object>> handleException(final Exception e){
        log.error(e.getMessage(), e);
        return ResponseEntity.internalServerError()
                .body(ApiResponse.onFailure(FailureCode.SERVER_ERROR, null));
    }
}
```

스프링은 `CommonException`을 상속한 모든 예외에 대해서 `handleCommonException()`을 실행시킨다. 그러면 `ResponseEntity<T>`를 사용하여 적절한 예외를 던지고, 이때 `status()` 메소드에 `FailureCode`에 정의한 enum 값의 `HttpStatus`가 사용되는 구조다. 이를 통해서 예외 처리를 수월하게 할 수 있다. 

예를 들어 어떤 사용자를 검색했는데 해당 사용자가 없다고 가정해보자. 이때 개발자는 단순히 `CommonException`을 상속받은 `UserNotFoundException`을 던지기만 하면 된다.

```java
public class UserNotFoundException extends CommonException {
    public UserNotFoundException(FailureCode failureCode) {
        super(failureCode);
    }
}
```

```java
@GetMapping("/find-user")
public ResponseEntity<ApiResponse<User>> findUser(
        @RequestParam String username
) {
    // USER_NOT_FOUND(HttpStatus.NOT_FOUND, 4004, "사용자가 존재하지 않습니다"),
    throw new UserNotFoundException(FailureCode.USER_NOT_FOUND);
}
```

이를 통해서 개발자는 예외에 대한 메시지를 정의할 때 그저 enum에 값을 추가하고, `CommonException`을 상속받는 적절한 예외를 만들어서 던지기만 하면 된다.

여담으로 단순히 사용자에게 메시지만 보여주면 예외를 추가로 만들지 않고 `CommonException`을 바로 던져도 무방하지만, 이는 어플리케이션의 규모가 커질수록 디버그를 방해하는 요소가 될 수 있다고 생각한다. 따라서 예외는 세부적으로 정의하는 것이 나중에 디버깅할 때 편하다고 생각한다.

### 개인적인 리팩토링

이 부분은 완전히 개인 의견으로 넘어가도 무방하다.(사실 나도 잘 안쓸 것 같다) 개인적으로 `ApiResponse`는 어중간하다고 생각하는데, 그 이유는 컨트롤러에서 매번 `ResponseEntity<T>`를 작성해야 하기 때문이다. 요청 메소드의 반환 부분을 보자.

```java
@PostMapping("/create-user/v3")
public ResponseEntity<ApiResponse<User>> createUserV3(
        @RequestBody User user
) {
    return ResponseEntity.created(URI.create("/search-user/" + user.username())).body(
            ApiResponse.of(SuccessCode.USER_CREATION_SUCCESS, user)
    );
}
```

이는 메소드가 많아질 수록 개발적 피곤함으로 다가올 수 있을 것이다. 따라서 `ResponseEntityHelper` 같은 이름의 클래스를 만들어서 `ResponseEntity<T>`를 반환하도록 하면 어떨까 하는 생각이 들었다.

```java
@JsonPropertyOrder({"isSuccess", "code", "message", "result"})
public record ResponseEntityHelper<T>(
        @JsonProperty(value = "isSuccess") boolean isSuccess,
        int code,
        String message,
        @JsonInclude(Include.NON_NULL) T result
) {

    public static <T> ResponseEntity<ResponseEntityHelper<T>> onSuccess() {
        SuccessCode code = SuccessCode.REQUEST_SUCCESS;
        return ResponseEntity.ok(new ResponseEntityHelper<>(true, code.getCode(), code.getMessage(), null));
    }

    public static <T> ResponseEntity<ResponseEntityHelper<T>> onSuccess(T data) {
        SuccessCode code = SuccessCode.REQUEST_SUCCESS;
        return ResponseEntity.ok(new ResponseEntityHelper<>(true, code.getCode(), code.getMessage(), data));
    }
    
    public static <T> ResponseEntity<ResponseEntityHelper<T>> onCreation(String location, SuccessCode code) {
        return ResponseEntity.created(URI.create(location))
                .body(new ResponseEntityHelper<>(true, code.getCode(), code.getMessage(), null));
    }

    public static <T> ResponseEntity<ResponseEntityHelper<T>> of(SuccessCode code) {
        if (code.getStatus() == HttpStatus.NOT_FOUND) {
            return ResponseEntity.noContent().build();
        }
        return ResponseEntity.status(code.getStatus()).body(
                new ResponseEntityHelper<>(true, code.getCode(), code.getMessage(), null)
        );
    }

    public static <T> ResponseEntity<ResponseEntityHelper<T>> of(SuccessCode code, T data) {
        return ResponseEntity.status(code.getStatus()).body(
                new ResponseEntityHelper<>(true, code.getCode(), code.getMessage(), data)
        );
    }
    
    public static <T> ResponseEntity<ResponseEntityHelper<T>> of(HttpHeaders headers, SuccessCode code, T data) {
        return ResponseEntity.status(code.getStatus())
                .headers(headers)
                .body(
                new ResponseEntityHelper<>(true, code.getCode(), code.getMessage(), data)
        );
    }
    
    public static <T> ResponseEntity<ResponseEntityHelper<T>> onFailure(FailureCode code) {
        return ResponseEntity.status(code.getStatus()).body(
                new ResponseEntityHelper<>(false, code.getCode(), code.getMessage(), null)
        );
    }

    public static <T> ResponseEntity<ResponseEntityHelper<T>> onFailure(FailureCode code, T data) {
        return ResponseEntity.status(code.getStatus()).body(
                new ResponseEntityHelper<>(false, code.getCode(), code.getMessage(), data)
        );
    }
}
```

대충 이런식으로 짜면 어떨까 싶었다. 이를 사용한 컨틀롤러 예시는 다음과 같다.

```java
@GetMapping("/find-user")
public ResponseEntity<ResponseEntityHelper<User>> findUser(
        @RequestParam String username
) {
    return ResponseEntityHelper.onSuccess(
            new User("test-user", 20)
    );
}
```

잘 응답되는 것을 확인할 수 있다.

```json
{
"isSuccess": true,
"code": 2000,
"message": "요청이 성공적으로 처리되었습니다",
"result":{
    "username": "test-user",
    "age": 20
}
}
```

이러면 개발자는 상황에 맞게 반환할 객체를 명시하거나, `of()` 메소드를 사용하여 상태 코드만 줄 수 있게된다. 하지만 이 방식은 개발자가 `ResponseEntityHelper`의 내부 동작을 어느 정도 이해해야 하고, 또한 `ResponseEntity`에서 제공하는 `noContent()` 같은 편리한 메소드를 활용할 수 없다는 점이다. 

또한 성공시에는 대부분 200 OK 응답을 보내기에 굳이 이런 식으로 `of()`를 세밀하게 나누는 것이 과연 의미가 있을까? 그냥 `ResponseEntity.ok(T data)`에 `ApiResponse<T>`를 사용하여 반환하는 것이 더 직관적이지 않을까 생각한다.

그리고 이미 구현이 잘 되어있는 `ResponseEntity`를 이런 식으로 한 번 더 감싸는 것이 과연 생산성을 높이는 것인지 잘 모르겠다.

따라서 개인 프로젝트에서는 이 방식을 도입해볼 수 있을 것 같지만, 위에서 언급한 부분들 때문에 이 방식을 다른 사람들에게 선뜻 권할 수 있을지는 고민이 된다.

---

이렇게 스프링에서 HTTP 상태 코드를 처리하는 두 가지 방법, `ResponseEntity<T>`를 활용한 응답 구조 개선, 그리고 개인적인 리팩토링 방법까지 살펴보았다.

REST API를 설계할 때 적절한 HTTP 상태 코드와 일관된 응답 구조를 유지하는 것은 매우 중요한 요소다. 단순한 응답이라면 `@ResponseStatus`를, 보다 세밀한 제어가 필요하다면 `ResponseEntity<T>`를 사용해도 되지만 코드의 일관성 문제 때문에 `ResponseEntity<T>`로 통일하는 것이 낫다는 결론에 이르게 되었다.

또한 개인적인 리팩토링을 통해서 설계한 `ResponseEntityHelper`를 활용하면 개발자는 필요한 값만 간단히 전달하여 일관된 응답을 만들 수 있도록 설계했다. 이 방식이 최선인지 아직 확신할 수는 없지만, 이를 고민하며 해결하려 했던 과정이 의미 있었다고 생각한다.