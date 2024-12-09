---
title: 메시지 기능으로 응답 메시지 관리

categories:
  - Spring

toc: true
toc_sticky: true
published: true
 
date: 2024-12-09
---

OAuth 관련 미니 프로젝트를 시작하려고 프로젝트의 사전 설정을 하던 도중에 응답에 관한 메시지를 스프링의 메시지 기능으로 관리하고 싶어졌다. 

### 메시지

스프링에서 제공하는 메시지 기능은  ***.properties**로 이루어진 파일에 정의된 다양한 메시지를 `MessageSource`의 구현체를 이용하여 불러오는 것이다. 이 방법의 장점은 별도 파일에 메시지를 모아서 관리할 수 있다는 것도 있지만, 가장 핵심은 국제화 기능이라고 생각한다.  
예를 들어 **resource** 디렉터리에 **errors.properties**를 메시지 파일로 사용한다고 가정했을 때, 영어를 사용하는 클라이언트의 요청에 대해서 메시지를 다르게 설정하고 싶다면 **errors_en.properties**라는 파일을 작성하고 다음과 같이 스프링 설정 파일에 파일 이름을 작성해주면 된다.

```yaml
# application.yml
spring:
 application:
  name: playground
 messages:
  basename: messages, errors
```

### 메시지 파일 작성

우선 properties 파일부터 작성해보자. 파일 이름은 앞서 예시로 들었던 **errors**와 **errors_en**으로 만든다.

```
# errors.properties
server.error=서버 내부 오류 발생
invalid.request=잘못된 요청

# 검증
NotNull={0} 값이 null로 전달됨
NotBlank={0} 값이 주어지지 않음.

# errors_en.properties
server.error=internal server error
invalid.request=Invalid Request

# 검증
NotNull={0} value cannot be null
NotBlank={0} value cannot be empty.
```

기존의 메시지는 `enum` 파일로 관리하였는데, 이 방법은 메시지를 하드코드해야 했다.

```java
// 응답 enum 예시
INVALID_REQUEST(4000, HttpStatus.BAD_REQUEST, "잘못된 요청!");
```

따라서 클라이언트의 `Accept-Language`와는 상관 없이 항상 같은 메시지를 응답한다. 하지만 이를 메시지 기능으로 관리하면 국제화를 손쉽게 할 수 있다. 또한 나중에 보게될 것이지만 검증 실패 시 클라이언트에게 실패한 검증 필드를 손쉽게 응답할 수도 있다.

```java
@Getter
@RequiredArgsConstructor
public enum ErrorCode {
    INVALID_REQUEST(4000, BAD_REQUEST, "invalid.request"),
    SERVER_ERROR(5000, INTERNAL_SERVER_ERROR, "server.error"),;

    private final int code;
    private final HttpStatus httpStatus;
    private final String messageCode;
}
```

### 메시지 파일 불러오기

이제 메시지 파일을 불러와야 한다. 스프링은 `MessageSource`인터페이스의 구현체를 어플리케이션 시작 때 설정 파일로부터 메시지 파일을 읽어와 등록해둔다. 따라서 의존성 주입을 이용하여 `MessageSource`를 주입하여 사용할 것이다.

```java
@Component
@RequiredArgsConstructor
public class MessageUtil {
    private static MessageSource messageSource;

    @Autowired
    public void setMessageSource(MessageSource messageSource) {
        MessageUtil.messageSource = messageSource;
    }

    public static String getMessage(String code) {
        Locale locale = LocaleContextHolder.getLocale();  // 현재 스레드의 locale을 자동으로 가져옴
        return messageSource.getMessage(code, null, locale);
    }
}
```

이때 수정자 주입을 사용한 이유는 `MessageSource`를 정적 메소드에서 사용해야 하기 때문에 정적 필드로 선언했기 때문이다. 이렇게 설계한 이유는 응답 객체를 보면 알 수 있다.

### 응답 객체 만들기

```java
@JsonPropertyOrder({"isSuccess", "code", "message", "result"})
public record JSONResponse<T>(
        @JsonProperty(value = "isSuccess") boolean isSuccess,
        int code,
        String message,
        @JsonInclude(Include.NON_NULL) T result
) {

    public static <T> JSONResponse<T> onSuccess(SuccessCode successCode, T data) {
        String message = MessageUtil.getMessage(successCode.getMessageCode());
        return new JSONResponse<>(true, successCode.getCode(), message, data);
    }

    public static <T> JSONResponse<T> onFailure(ErrorCode errorCode, T data) {
        String message = MessageUtil.getMessage(errorCode.getMessageCode());
        return new JSONResponse<>(false, errorCode.getCode(), message, data);
    }
}
```

응답 객체를 레코드로 관리하기 위해서 앞서서 수정자 주입을 사용했던 것이다. 덕분에 정적 메소드를 사용하여 코드를 깔끔하게 유지할 수 있다. `ErrorCode`또는 `SuccessCode`로 전달받은 `enum`에서 `messageCode`를 이용하여 메시지 파일로부터 메시지를 가져올 수 있다.

### ExceptionHandler 생성

예외 처리를 위해서 공통 예외 클래스를 만들고, 예외 처리를 하는 `@RestControllerAdvice`를 만든다.

```java
@Getter
public class CommonException extends RuntimeException {
    private final ErrorCode errorCode;

    public CommonException(ErrorCode errorCode) {
        this.errorCode = errorCode;
    }
}
```

```java
@RestControllerAdvice
public class CommonExceptionHandler {
    // Valid 실패 시 발생하는 예외
    @ExceptionHandler(MethodArgumentNotValidException.class)
    public ResponseEntity<JSONResponse<Object>> handleMethodArgumentNotValidException(
            final MethodArgumentNotValidException e
    ) {
        List<FieldError> fieldErrors = e.getBindingResult().getFieldErrors();
        List<String> errorMessages = fieldErrors
                                            .stream()
                                            .map(fieldError -> MessageUtil
                                            .getMessage(
                                            fieldError.getCode(),
                                            new Object[] { fieldError.getField() }
                                            ))
                                            .collect(Collectors.toList());

        return ResponseEntity
                .status(BAD_REQUEST)
                .body(JSONResponse.onFailure(ErrorCode.INVALID_REQUEST, errorMessages));
    }

    // @PathVariable 잘못 입력 또는 요청 메시지 바디에 아무 값도 전달되지 않았을 때
    @ExceptionHandler({MethodArgumentTypeMismatchException.class, HttpMessageNotReadableException.class})
    public ResponseEntity<JSONResponse<Object>> handlerMethodArgumentTypeMismatchException(final Exception e) {
        return ResponseEntity
                .status(BAD_REQUEST)
                .body(JSONResponse.onFailure(ErrorCode.INVALID_REQUEST, null));
    }

    // 그 외 CommonException 상속받은 모든 예외를 이 메소드에서 처리
    @ExceptionHandler(CommonException.class)
    public ResponseEntity<JSONResponse<Object>> handlerCommonException(final CommonException e) {
        return ResponseEntity
                .status(e.getErrorCode().getHttpStatus())
                .body(JSONResponse.onFailure(e.getErrorCode(), null));
    }

    // 서버 내부 오류 (SQL 연결 오류 등) 처리
    @ExceptionHandler(Exception.class)
    public ResponseEntity<JSONResponse<Object>> handlerException(final Exception e) {
        return ResponseEntity
                .status(INTERNAL_SERVER_ERROR)
                .body(JSONResponse.onFailure(ErrorCode.SERVER_ERROR, null));
    }
}
```

### 테스트

테스트를 위해 컨트롤러와 요청 객체를 만든다.

```java
@RestController
@RequestMapping("/api/user")
public class UserController {
    @GetMapping("/test/{param}")
    public void paramTypeMisMatch(@PathVariable int param) throws Exception {
    }

    @PostMapping("/test")
    public void validFailure(@RequestBody @Valid UserRegisterRequest request) {
    }
}
```

```java
public record UserRegisterRequest(
		@NotNull
		Long id,
		@NotBlank
		String name
) {
}
```

이제 테스트를 해보자. 

**잘못된 경로 변수 입력(**`MethodArgumentTypeMismatchException`)

```
GET http://localhost:8080/api/user/test/string
```

```
{
    "isSuccess": false,
    "code": 4000,
    "message": "잘못된 요청"
}
```

**요청 키 값의 누락(**`HttpMessageNotReadableException`**)**

```
POST http://localhost:8080/api/user/test

BODY
{
    "name": "123"
}
```

```
{
    "isSuccess": false,
    "code": 4000,
    "message": "잘못된 요청"
}
```

**요청 값의 null 또는 공백(**`MethodArgumentNotValidException`)

이 방법의 가장 큰 장점이 바로 이 부분인 것 같다. 

```
POST http://localhost:8080/api/user/test
{
    "id": null,
    "name": ""
}
```

```
{
    "isSuccess": false,
    "code": 4000,
    "message": "잘못된 요청",
    "result":[
        "id 값이 null로 전달됨",
        "name 값이 주어지지 않음."
    ]
}
```

반복문을 돌며 검증에 실패한 필드에 대해서 오류 코드를 가져오고, 그것을 리스트로 만들어 데이터에 넣은 것이다.  
이제 마지막으로 국제화에 성공했는지 보자. `Accept-Language` 헤더를 `en-US`로 설정해보자.

```
POST http://localhost:8080/api/user/test

header: {
    "Accept-Language": "en-US",
}
{
    "id": null,
    "name": "",
}
```

```
{
    "isSuccess": false,
    "code": 4000,
    "message": "Invalid Request",
    "result":[
        "id value cannot be null",
        "name value cannot be empty."
    ]
}
```

### 정리

메시지 파일을 이용하여 메시지 관리를 하는 방법을 알아보았다. 쳇 GPT나 여러 강의 정리를 참고하면서 만든거긴 한데 여러 개선 사항이 보이긴 한다.  
예를 들어 수정자 주입을 통해 현재 객체의 의존성을 설정했는데, 이는 `MessageUtil`을 정적 메소드를 사용하려고 `MessageSource`를 정적 필드로 선언했기 때문이다. 좀 더 설계를 잘한다면 메시지 파일을 관리하는 특정한 컴포넌트를 응답 객체가 아닌 다른 객체에서 어떻게 할 수 있지 않을까 생각한다.  
이상으로 메시지 파일을 이용한 메시지 관리를 알아보았다. 국제화를 할 일이 많지 않을 것 같긴 한데 뭔가 해보고 싶었다. 

**[전체 코드 참고](https://github.com/sehako/playground/tree/exception)**