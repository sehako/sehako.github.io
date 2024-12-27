---
title: SMTP를 활용한 이메일 인증

categories:
  - Spring

toc: true
toc_sticky: true
published: true
 
date: 2024-12-27
---

# SMTP

SMTP(Simple Mail Transfer Protocol)는 전자 메일을 전송하는 데 사용되는 표준 통신 프로토콜이다. 주로 메일 서버 간 메시지 전송 및 메일 클라이언트와 서버 간의 통신을 처리하는 데 사용된다.

일반적으로 포트 25, 465(SSL/TLS), 587(TLS)에서 동작하고, 보안 강화를 위해 TLS 또는 SSL을 사용하는 것이 일반적이다. 

여기서는 구글 이메일을 통해서 SMTP를 구현해보겠다.

# 프로젝트에 SMTP 적용해보기

## 사전 설정

### SMTP 설정

구글 SMTP를 이용하기 위해서 계정 관리 -> 앱 비밀번호 발급을 해야 한다. 계정 관리 창에서 검색을 통해 '앱 비밀번호'라고 작성하면 쉽게 찾을 수 있다.

![앱 비밀번호 발급](/assets/images/email-verify_01.png)

여기서 앱 이름을 입력하고 만들면 16자리의 비밀번호가 나오게 되는데, 이를 이용하여 SMTP 서버에 로그인 할 수 있다.

### 라이브러리 설정

라이브러리는 스프링에서 제공하는 SMTP 관련 라이브러리를 사용하였다.

```groovy
implementation group: 'org.springframework.boot', name: 'spring-boot-starter-mail', version: '3.4.1'
```

### 설정 파일 작성

설정 파일에 `spring.mail`로 시작하는 다양한 옵션이 존재하는데, 이를 이용하여 SMTP 서버에 대한 다양한 정보를 설정하면 된다.

```yaml
# application.yml
spring:
  mail:
    host: smtp.gmail.com
    port: 587
    username: ${EMAIL_USERNAME}
    password: ${EMAIL_PASSWORD}
    properties:
      mail.smtp.auth: true
      mail.smtp.starttls.enable: true
```


이렇게 설정해두면 스프링이 실행될 때 `MailSenderAutoConfiguration`라는 객체를 만들어 스프링 컨테이너에 저장해둔다. 따라서 굳이 하나하나 설정하지 않아도 되어 간편하다. 다만 나는 여러 설정파일을 `import`를 통해서 관리하고자 하나하나 나누었는데 이 경우에는 그런 식으로 따로 설정 파일을 만들어 `import` 하면 인텔리제이에서 컴파일 에러를 띄웠다. 이게 스프링에서도 빈이 없다고 발생하는 문제인지는 확인하지 않았지만 굳이 빨간 줄을 보고 싶지 않아서 그냥 원본 설정 파일에 설정하고 프로젝트의 환경변수에 내 이메일 주소와 앞서 발급한 앱 비밀번호를 설정해두어 불러왔다.

## 코드 작성

기본적으로 사용자가 어떤 이메일을 요청 메시지 바디를 통해 보내면 이 정보를 기반으로 이메일을 보내고, 이메일에 인증 링크를 두어 해당 링크에 접속하면 데이터베이스에 최종적으로 사용자가 저장되도록 구현하였다.

### 컨트롤러 작성

```java
@RestController
@RequestMapping("/api/email")
@RequiredArgsConstructor
public class SmtpController {
    private final SmtpService smtpService;

    @PostMapping("/send")
    public ResponseEntity<JSONResponse<Object>> sendAuthentication(@RequestBody EmailSendRequest request) {
        smtpService.sendEmail(request.toDto());
        return ResponseEntity.ok(JSONResponse.onSuccess(null));
    }

    @GetMapping("/verify")
    public ResponseEntity<JSONResponse<Object>> emailVerify(
            @RequestParam("token") String token
    ) {
        smtpService.verify(token);
        return ResponseEntity.ok(JSONResponse.onSuccess(null));
    }
}
```

`api/email/send` 요청은 인증을 위한 링크를 사용자 이메일로 전송한다. 인증을 위한 링크는 `api/email/verify?token=??` 형식으로 주어지는데, 여기서는 사용자가 `/send` 요청을 했을 때의 UUID를 가지고 레디스에 접근하여 사용자가 있으면 인증이 되었다고 간주하고 사용자 정보가 최종적으로 데이터 베이스에 저장된다.

### 서비스 작성

```java
@Service
@RequiredArgsConstructor
public class SmtpService {
    private final JavaMailSender mailSender;
    private final RedisService redisService;
    private final LoginRepository loginRepository;

    private static final String AUTH_URL = "http://localhost:8080/api/email/verify?token=";
    private static final long EXPIRATION_TIME = 300000L;

    public void sendEmail(EmailInfoDto info) {
        MimeMessage message = mailSender.createMimeMessage();
        try {
            String authKey = TokenGenerator.generateEncryptedToken();
            redisService.save(authKey, info.to(), EXPIRATION_TIME);

            MimeMessageHelper helper = new MimeMessageHelper(message, true);
            helper.setTo(info.to());
            helper.setSubject("auth test");
            helper.setText(AUTH_URL + authKey);
            mailSender.send(message);
        } catch (MessagingException e) {
            e.printStackTrace();
            throw new EmailSendException(ErrorCode.INVALID_REQUEST);
        }
    }

    public void verify(String token) {
        String email = redisService.get(token);
        if (email == null) {
            throw new VerifyLinkException(ErrorCode.INVALID_REQUEST);
        }
        User user = User.builder()
                .email(email)
                .nickname("임시 닉네임")
                .authType(AuthType.EMAIL)
                .build();
        loginRepository.save(user);
        redisService.delete(token);
    }
}
```
`MimeMessage`로 메시지 객체를 만들어서 `MimeMessageHelper`로 메시지를 수신할 대상과 제목, 그리고 내용을 작성해서 스프링이 자동으로 만들어뒀던 `JavaMailSender` 객체(`MailSenderAutoConfiguration`)로 메일을 보낼 수 있다.

그리고 보낸 링크를 클릭하면 컨트롤러를 거쳐서 서비스에서 `verify`가 실행되고, 여기서 레디스에 접근하여 사용자 정보가 있으면 인증된 사용자로 간주하여 데이터베이스에 삽입한다. 블로그를 쓰면서 생각이 난건데 `email`이 없는 경우는 레디스에 설정된 저장 시간이 만료되었다는 것이고, 이 경우는 사용자가 제한 시간 내 인증 링크를 클릭하지 않았다는 것이므로 `VerifyExpiredException` 같은 걸로 예외를 던져줘도 될 것 같다.

여기서 `TokenGenerator`는 신경 쓸 필요 없다. UUID를 만들 때 뭔가 암호화 해야 하나 싶어서 챗 GPT에게 부탁해서 대충 코드를 적긴 했는데 생각해보면 어차피 레디스를 통해서 사용자 이메일이 관리되기 때문에 단순한 UUID 값이 쿼리 파라미터로 구성될 거라서 굳이 이렇게 할 이유가 없는 것 같다. 그래도 코드는 붙여넣도록 하겠다.

```java
public class TokenGenerator {
    // Key for AES encryption (256-bit key)
    private static final String SECRET_KEY = "0123456789abcdef0123456789abcdef"; // Replace with a secure key
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final int GCM_TAG_LENGTH = 128;

    // Generate Encrypted Token
    public static String generateEncryptedToken() {
        // Generate a random Initialization Vector (IV)
        byte[] iv = new byte[12];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);

        try {
            // Encrypt data
            Cipher cipher = Cipher.getInstance(ALGORITHM);
            SecretKey secretKey = new SecretKeySpec(SECRET_KEY.getBytes(StandardCharsets.UTF_8), "AES");
            GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, spec);
            byte[] encryptedData = cipher.doFinal(UUID.randomUUID().toString().getBytes(StandardCharsets.UTF_8));

            // Combine IV and encrypted data, then encode in Base64
            byte[] combined = new byte[iv.length + encryptedData.length];
            System.arraycopy(iv, 0, combined, 0, iv.length);
            System.arraycopy(encryptedData, 0, combined, iv.length, encryptedData.length);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(combined);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
```

## 비동기 설정

이제 SMTP를 완성하였다. 생각보다 간단하였지만 한 가지 간과한 것이 있다. 바로 비동기 설정이다. 왜 비동기 설정이 필요할까? 비동기 설정을 하지 않으면 SMTP를 사용하는 요청을 할 때, 서버에서도 SMTP 서버에 요청을 하게 되고, 이 요청이 끝날때까지 기다려야 한다. 

이는 곧 클라이언트에서 요청을 보내고 SMTP 요청이 끝날때까지 대기를 해야 한다는 것이다. 따라서 SMTP 서버에 요청하는 코드를 비동기로 작동시켜 실제 이메일 전송 여부와는 상관 없이 모든 처리를 끝내고 클라이언트에게 응답 코드를 반환하도록 만들어야 한다.

이를 위해 스프링에서 제공하는 `AsyncConfigurer`를 구현하여 스프링 빈으로 등록하자.

```java
@Slf4j
@EnableAsync
@Configuration
public class AsyncConfig implements AsyncConfigurer {
    @Bean(name = "mailExecutor")
    public TaskExecutor taskExecutor() {
        ThreadPoolTaskExecutor taskExecutor = new ThreadPoolTaskExecutor();
        // 기본 스레드 수
        taskExecutor.setCorePoolSize(10);
        // 최대 스레드 수
        taskExecutor.setMaxPoolSize(20);
        // 작업 요청 대기열 크기
        taskExecutor.setQueueCapacity(40);
        // 생성된 스레드 이름의 접두사
        taskExecutor.setThreadNamePrefix("Async-");
        return taskExecutor;
    }

    // 예외가 발생했을 때 처리 (단순하게 로그를 출력하도록 하였다)
    @Override
    public AsyncUncaughtExceptionHandler getAsyncUncaughtExceptionHandler() {
        return new AsyncUncaughtExceptionHandler() {
            @Override
            public void handleUncaughtException(Throwable ex, Method method, Object... params) {
                log.info("ex = {}, method = {}", ex, method);
            }
        };
    }
}
```

이렇게 등록해두면 비동기를 처리하는 스레드 풀을 만들어서 스프링 컨테이너에 관리해둔다. 이를 적용하는 방법은 아주 간단한데 비동기가 필요한 메소드에 `@Async` 어노테이션을 붙이면 된다.

```java
public class SmtpService {
    private final JavaMailSender mailSender;
    private final RedisService redisService;
    private final LoginRepository loginRepository;

    private static final String AUTH_URL = "http://localhost:8080/api/email/verify?token=";
    private static final long EXPIRATION_TIME = 300000L;

    @Async("mailExecutor")
    public void sendEmail(EmailInfoDto info) {}
```

# 참고자료

### [SMTP](https://velog.io/@hwaya2828/SMTP)