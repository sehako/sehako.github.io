---
title: 실시간 채팅 개발 - 채팅 저장과 예외 처리

categories:
  - Spring
  - WebSocket

toc: true
toc_sticky: true
published: true

date: 2025-08-16
last_modified_at: 2025-08-16
---

현재는 채팅방에 입장하고 다른 클라이언트들과 채팅을 나눈 다음에 채팅방을 재접속하면 이전에 나눈 채팅을 확인할 수 없다. 채팅 내역이 데이터베이스에 저장되지 않기 때문이다. 따라서 채팅 내역을 저장하여 클라이언트가 재접속하여도 이전의 채팅 내역을 확인할 수 있도록 할 것이다.

참고로 채팅 저장을 위한 데이터베이스로는 PostgreSQL을 사용하였다. 채팅을 구현할 때 MongoDB같은 NoSQL을 많이들 사용하는 것 같지만, 나는 임의의 채팅방을 만들고, 거기에 사용자를 미리 설정해두어 읽음 처리도 구현할 예정이어서 그냥 RDBMS에 채팅 관련한 모든 데이터를 저장하는 방식으로 활용할 것이다.

# 사전 설정

## ERD

![image.png](/assets/images/development/spring-websocket/25-08-16-live-chat-server-with-stomp-03/01.png)

간단한 학습을 위해서 정말 필요한 테이블과 컬럼만 구상하였다. 지금 와서 생각해보면 채팅 내역 테이블만 있어도 됐을 것 같다.

## PostgreSQL 컨테이너 실행

다음 도커 명령어로 PostgreSQL을 실행하도록 하자.

```bash
docker run --name postgres-chat \
    -e POSTGRES_USER=root \
    -e POSTGRES_PASSWORD=password \
    -p 5432:5432 \
    -d postgres:latest
```

이제 `localhost:5432` 와 root / password로 데이터베이스에 접속할 수 있다.

## 데이터베이스 접속 및 DDL 실행

나는 IntelliJ에서 지원하는 데이터베이스 접속 도구를 활용했지만, 만약에 IntelliJ가 커뮤니티 버전이라면 별도의 도구를 사용해야 한다. 개인적으로는 [DBeaver](https://dbeaver.io/)가 커뮤니티 버전에서 다양한 RDBMS를 지원하고, PostgreSQL도 이 중 하나이기 때문에 괜찮은 것 같다.

접속이 완료되었으면 앞서 설계한 ERD대로 테이블을 만들기 위해 다음 DDL 명령어를 입력하도록 하자.

```sql
CREATE TABLE "user" (
    id INTEGER PRIMARY KEY
);

CREATE TABLE chatroom (
    id INTEGER PRIMARY KEY
);

CREATE TABLE chat_log (
    id BIGINT PRIMARY KEY,
    chatroom_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    message TEXT,
    created_at TIMESTAMP DEFAULT now()
);

CREATE TABLE chatroom_user (
    id BIGINT PRIMARY KEY,
    chatroom_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    CONSTRAINT uk_chatroom_user_pair UNIQUE (chatroom_id, user_id)
);

--- PK 자동 증가 설정

CREATE SEQUENCE chat_log_id_seq;
ALTER TABLE chat_log
ALTER COLUMN id SET DEFAULT nextval('chat_log_id_seq');
ALTER SEQUENCE chat_log_id_seq OWNED BY chat_log.id;
SELECT setval('chat_log_id_seq', COALESCE((SELECT MAX(id) FROM chat_log), 0) + 1, false);

```

참고로 테이블에서 “user”로 작성한 이유는 Postgres에서 user는 예약어이기 때문이다.

### 사전 데이터 설정

일반적으로는 채팅방을 생성하고, 가입하는 시나리오가 존재해야 한다. 하지만 여기서는 그러한 시나리오가 이미 완료되었다고 가정하고 다음과 같은 데이터를 쿼리를 통해 삽입하였다.

```sql
INSERT INTO "user"(id) VALUES (1);
INSERT INTO "user"(id) VALUES (2);
INSERT INTO "user"(id) VALUES (3);

INSERT INTO chatroom(id) VALUES (1);

INSERT INTO chatroom_user(id, chatroom_id, user_id) VALUES (1, 1, 1);
INSERT INTO chatroom_user(id, chatroom_id, user_id) VALUES (2, 1, 2);
INSERT INTO chatroom_user(id, chatroom_id, user_id) VALUES (3, 1, 3);
```

3명의 사용자가 1번 채팅방에 대해서 모두 가입되었다고 가정할 것이다.

## 애플리케이션 설정

데이터베이스에 연결하기 위해서 build.gradle에 의존성을 추가하고, application.yml에 설정을 해준다.

```groovy
runtimeOnly 'org.postgresql:postgresql'
implementation 'org.springframework.boot:spring-boot-starter-data-jpa'
```

JPA를 활용하여 편리하게 엔티티를 다룰 것이다.

```yaml
# application.yml
spring:
  # 데이터베이스 접속 정보
  datasource:
    url: jdbc:postgresql://localhost:5432/postgres
    driver-class-name: org.postgresql.Driver
    username: root
    password: password
```

## 엔티티 설계

마지막 사전 설정이다. 테이블과 매칭되는 엔티티들을 다음과 같이 설계하도록 하자.

```java
@Entity
@Table(name = "\"user\"")
@Getter
public class User {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;
}
```

```java
@Entity
@Getter
@Table(name = "chatroom")
public class Chatroom {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;
}
```

```java
@Entity
@Table(name = "chatroom_user")
@Getter
public class ChatroomUser {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name = "chatroom_id")
    private Long chatroomId;

    @Column(name = "user_id")
    private Long userId;
}
```

```java
@Entity
@Getter
@NoArgsConstructor(access = AccessLevel.PROTECTED)
@Table(name = "chat_log")
public class ChatLog {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name = "chatroom_id")
    private Long chatroomId;

    @Column(name = "user_id")
    private Long userId;

    @Column(columnDefinition = "text")
    private String message;

    @CreationTimestamp
    @Column(name = "created_at", nullable = false)
    private LocalDateTime createdAt;

    @Builder
    private ChatLog(Long chatroomId, Long userId, String message, LocalDateTime createdAt) {
        this.chatroomId = chatroomId;
        this.userId = userId;
        this.message = message;
        this.createdAt = createdAt;
    }
}
```

채팅 기능 구현에 집중하기 위해 각 엔티티의 연관관계는 일부러 정의하지 않고 각 테이블의 `id` 값을 외래키로 명시하였다. 이제 드디어 기능 구현을 할 차례이다.

# 채팅 저장

이전 포스팅에서 인증 처리를 위해서 `ChannelInterceptor`를 활용하여 토큰을 받았었다. 이번 포스팅에서는 원할한 구현을 위해 다음 부분을 삭제하고 진행하였다.

## 레포지토리

`ChatLog` 엔티티를 저장하는 JPA레포지토리를 다음과 같이 간단하게 구현하였다.

```java
@Repository
public interface ChatLogRepository extends JpaRepository<ChatLog, Long> {
}
```

## 서비스

```java
@Service
@RequiredArgsConstructor
public class ChatService {
    private final ChatLogRepository chatLogRepository;

    public MessageResponse saveMessage(MessageSendingDto dto) {
        ChatLog savedChatLog = saveChatLog(dto);
        return MessageResponse.of(
                savedChatLog.getId(),
                savedChatLog.getUserId(),
                savedChatLog.getMessage(),
                savedChatLog.getCreatedAt()
        );
    }

    private ChatLog saveChatLog(MessageSendingDto dto) {
        return chatLogRepository.save(dto.toChatLog());
    }
}
```

서비스 계층에서는 주어진 데이터를 엔티티로 변환하고 저장한 다음에, `MessageResponse`로 변환하여 반환하도록 작성하였다.

```java
public record MessageSendingDto(
        Long chatroomId,
        Long userId,
        String message
) {
    public ChatLog toChatLog() {
        return ChatLog.builder()
                .chatroomId(chatroomId)
                .userId(userId)
                .message(message)
                .build();
    }
}
```

```java
public record MessageResponse(
        Long id,
        Long userId,
        String message,
        LocalDateTime at
) {
    public static MessageResponse of(Long id, Long userId, String message, LocalDateTime time) {
        return new MessageResponse(id, userId, message, time);
    }
}
```

## 컨트롤러

컨트롤러에서는 전달받은 메시지 객체를 적절하게 변환하여 서비스에 처리를 위임하고, 그 결과를 활용하여 채팅방에 브로드캐스트 한다.

```java
@Slf4j
@Controller
@RequiredArgsConstructor
public class StompController {
    private final ChatService chatService;

    @MessageMapping("/chat.{chatroom-id}")
    public void sendMessage(
            @DestinationVariable("chatroom-id") Long chatroomId,
            MessageRequest message
    ) {
        MessageResponse savedMessage = chatService
                .saveMessage(message.toMessageSendingDto(chatroomId));

        messagingTemplate.convertAndSend(
                "/topic/chat." + chatroomId,
                savedMessage
        );
    }
}
```

이를 위해서 다음과 같이 요청 객체에 DTO를 변환하는 메서드를 작성하였다.

```java
public record MessageRequest(
        Long userId,
        String message
) {
    public MessageSendingDto toMessageSendingDto(
            Long chatroomId
    ) {
        return new MessageSendingDto(
                chatroomId,
                userId,
                message
        );
    }
}
```

## 테스트

2명 이상의 사용자를 서버에 접속하고, 1번 채팅방에서 채팅을 나누어보자. 참고로 크롬 기준으로 일반 창과 시크릿 창을 띄우면 두 개의 별도 세션이 구성되기 때문에 별 문제 없이 2명 사용자를 만들 수 있다. 다른 브라우저도 활용하면 2명 이상의 사용자도 문제없이 연결할 수 있을 것이다.

테스트는 지난 포스팅과 마찬가지로 다음 [사이트](https://jiangxy.github.io/websocket-debug-tool/)를 활용하였다. 1번 사용자와 2번 사용자가 접속되어 있다고 가정해보자. 이때 1번 사용자가 채팅을 전송했을 때, 2번 사용자가 1번 사용자의 채팅을 수신할 수 있는 지 확인할 것이다.

**채팅 전송**

```
$ _INFO_:Connect STOMP server success, url = ws://localhost:8080/ws, connectHeader =
$ _INFO_:send STOMP message, destination = /app/chat.1, content = {"userId": 1, "message": "Hello"}, header =
```

**채팅 수신**

```
$ _INFO_:Connect STOMP server success, url = ws://localhost:8080/ws, connectHeader =
$ _INFO_:subscribe destination /topic/chat.1 success
$ _INFO_:Receive subscribed message from destination /topic/chat.1, content = MESSAGE
content-length:64
message-id:f30ff991-9373-7c78-5bb9-95384685f4cf-0
subscription:sub-0
content-type:application/json
destination:/topic/chat.1
content-length:64

{"userId":1,"message":"Hello","at":"2025-08-15T17:00:22.607594"}
```

정상적으로 수신이 된 것을 확인할 수 있다. 그렇다면 데이터베이스에도 정상적으로 저장이 되었을까? 확인을 위해 `SELECT`로 `chat_log` 테이블을 확인해보자.

```sql
SELECT * FROM chat_log;
```

다음과 같은 조회 결과가 출력된다.

| id  | chatroom_id | user_id | content | created_at                 |
| --- | ----------- | ------- | ------- | -------------------------- |
| 1   | 1           | 1       | Hello   | 2025-08-15 17:00:22.588947 |

이렇게 정상적으로 데이터가 저장된 것을 확인할 수 있다.

# 채팅 내역 조회

단순히 서버로 전달된 채팅 데이터를 저장만 한다면 의미가 없을 것이다. 중요한 것은 채팅방에 접속한 사용자가 해당 채팅방에 저장된 채팅 내역을 볼 수 있어야 한다. 가장 간단한 것은 클라이언트에게 커서 기반 페이지네이션을 통해 채팅 내역을 조회할 수 있는 REST API를 하나 구현하고, 이를 채팅방 접속 시에 추가적으로 요청을 보내도록 하면 될 것이다.

> 참고로 OFFSET 기반 페이지네이션은 중복 데이터 문제와 페이지 증가에 따른 쿼리 성능 저하로 적절하지 않다고 생각하였다.

나도 처음에 채팅을 구현할 때에는 REST API로 구현하였고, 별 문제 없이 제대로 실행되었다. 하지만 이미 서버와 연결이 된 상태에서 굳이 그렇게 할 필요가 있을까 하는 생각이 들었고, STOMP SEND 요청을 활용하여 채팅방 ID와 커서를 전달받아 채팅 내역을 조회하고, 특정 사용자에게 전송하면 될 것 같다고 생각하였다.

## 페이지네이션을 위한 사전 설정

그 전에 우선 페이지네이션을 구현하기 위해서 1번 채팅방에 대해 1번 부터 3번 사용자까지 채팅을 나눈 무작위 100개 데이터를 Postgres에 INSERT 하였다.

```sql
INSERT INTO chat_log (chatroom_id, user_id, message, created_at)
SELECT
    1,
    (1 + (random() * 3)::int),
    substring(md5(random()::text) from 1 for (1 + (random() * 10)::int)),
    now() + (g * interval '1 second')
FROM generate_series(1, 100) g;
```

그리고 커서 기반 페이지네이션 작업을 처리할 수 있는 메서드를 만들자.

```java
public interface ChatLogRepository extends JpaRepository<ChatLog, Long> {
    // 최초 입장: 최신 20개 (내림차순)
    List<ChatLog> findTop20ByChatroomIdOrderByIdDesc(Long chatroomId);

    // 과거 로딩: 현재 보유한 가장 오래된 id(=oldestId) 기준, 그 이전 20개
    List<ChatLog> findTop20ByChatroomIdAndIdLessThanOrderByIdDesc(Long chatroomId, Long oldestId);
}
```

최초 입장 시에는 커서 값이 없기 때문에 채팅방 ID에 해당하는 채팅을 채팅 ID의 내림차순으로 20개를 반환하도록 하고, 그 다음부터는 커서 값이 존재하므로 커서보다 더 작은 채팅 내역 대상으로 채팅 ID의 내림차순으로 정렬된 데이터 20개를 반환하도록 하였다.

## 특정 사용자에게 데이터를 전송하는 두 가지 방법

요청을 통해 조회된 채팅 내역을 `@SendTo` 어노테이션이나 `SimpMessagingTemplate.convertAndSend()`를 사용하여 전송하면 해당 토픽을 구독한 사용자들이 채팅 내역을 지속적으로 중복하여 전달받는다.

이를 위해서 `@SendToUser` 어노테이션이나 `SimpMessagingTemplate.convertAndSendToUser()`를 사용하면 같은 토픽을 구독한 사용자들 중에서 요청을 보낸 사용자에게만 데이터를 전송할 수 있다. 하나씩 알아보도록 하자.

### @SendToUser

`@SendToUser`를 사용하기 위해서 저번 포스팅에서 다뤘던 것처럼 반환값이 있어야 한다. 조회한 채팅 내역을 `List<MessageResponse>`로 변환하여 반환하는 서비스를 먼저 작성하였다.

```java
@Service
@RequiredArgsConstructor
public class ChatService {
    private final ChatLogRepository chatLogRepository;

    public List<MessageResponse> findChatLog(Long chatroomId, Long cursor) {
        List<ChatLog> chatLogs = cursor == null
                ? chatLogRepository.findTop20ByChatroomIdOrderByIdDesc(chatroomId)
                : chatLogRepository.findTop20ByChatroomIdAndIdLessThanOrderByIdDesc(chatroomId, cursor);

        return chatLogs.stream().map(
                chatLog ->
                        MessageResponse.of(
                                chatLog.getId(),
                                chatLog.getUserId(),
                                chatLog.getMessage(),
                                chatLog.getCreatedAt())
        ).toList();
    }
}
```

`cursor`가 `null`이면 최초 접속으로 간주하고, `null`이 아니면 커서값을 기반으로 페이지네이션을 수행하도록 작성하였다.

그리고 이 메서드를 호출하여 반환하는 컨트롤러를 작성하였다. 이때 한 가지 생각할 점은 사용자로부터 가장 마지막으로 읽은 채팅 내역 ID값을 받아서 커서로 받아야 한다. 이때 일반적인 REST API처럼 쿼리 파라미터로 받는 방법도 있겠지만 GPT에게 물어보니 다음과 같이 답변을 해주었다.

- STOMP는 HTTP와 달리 쿼리파라미터 개념이 없음 (프레임 기반).
- 쿼리파라미터는 연결 시점에만 쓰이고, 동적 데이터 전달엔 부적절.
- 커서는 구독/메시지 단위의 컨텍스트이므로 `SUBSCRIBE`나 `SEND` 프레임의 **헤더**로 전달해야 함.
- 보안/유지보수 측면에서도 헤더 방식이 권장됨.

따라서 STOMP SEND 요청 헤더에 커서 값을 담아서 보내는 방식으로 작성하였다.

```java
@Controller
@RequiredArgsConstructor
public class StompController {
    private final ChatService chatService;

    @MessageMapping("/chat.{chatroom-id}.log")
    @SendToUser("/queue/chat.log")
    public List<MessageResponse> sendChatLog(
            @DestinationVariable("chatroom-id") Long chatroomId,
            @Header(name = "cursor", required = false) Long cursor
    ) {
        return chatService.findChatLog(chatroomId, cursor);
    }
}
```

실행 결과는 `SimpMessagingTemplate`를 통한 데이터 전송 후에 살펴보도록 하자.

### SimpMessagingTemplate

```java
@Controller
@RequiredArgsConstructor
public class StompController {
    private final ChatService chatService;
    private final SimpMessagingTemplate messagingTemplate;

    @MessageMapping("/chat.{chatroom-id}.log")
    public void sendChatLog(
            @DestinationVariable("chatroom-id") Long chatroomId,
            @Header(name = "cursor", required = false) Long cursor,
            @Header(name = "simpSessionId") String simpSessionId
    ) {
        log.info("{}", simpSessionId);
        List<MessageResponse> messageLogs = chatService
                .findChatLog(chatroomId, cursor);

        messagingTemplate.convertAndSendToUser(
                simpSessionId,
                "/queue/chat.log",
                messageLogs,
                createHeaders(simpSessionId)
        );
    }

    private MessageHeaders createHeaders(@Nullable String sessionId) {
        SimpMessageHeaderAccessor headerAccessor = SimpMessageHeaderAccessor
                .create(SimpMessageType.MESSAGE);

        if (sessionId != null) {
            headerAccessor.setSessionId(sessionId);
        }
        headerAccessor.setLeaveMutable(true);
        return headerAccessor.getMessageHeaders();
    }
}
```

`convertAndSendToUser()`의 내부 구현은 다음과 같다.

```java
@Override
public void convertAndSendToUser(
        String user,
        String destination,
        Object payload,
		@Nullable Map<String, Object> headers
) throws MessagingException {
	convertAndSendToUser(user, destination, payload, headers, null);
}
```

이때 한 가지 궁금한 것이 있을 수 있다. 왜 새롭게 헤더를 만들어서 사용자에게 전송하는 것일까? 그 이유는 헤더를 사용자 정보 기반으로 새롭게 만들어주지 않으면 메시지는 유실되기 때문이다. 스프링에서 `convertAndSendToUser(...)`는 내부적으로 `Principal` 기반으로 특정 사용자 세션(`/user/{sessionId}/queue/...`)에 메시지를 매핑 및 라우팅하므로, `sessionId`만으로 보내려면 해당 세션을 명확히 지정해줘야 한다.

## 테스트

이제 페이지네이션이 동작하는지 확인하도록 한다. 이때 한 가지 주의해야 할 점은 클라이언트는 `/queue/chat.log` 토픽을 구독하는 것이 아니라, `/user/queue/chat.log` 토픽을 구독해야 한다는 것이다. 이는 앞서 설명한 사용자 세션에 메시지를 매핑 및 라우팅 하기 때문이다.

STOMP 설정에 사용된 `StompConfig`를 다시 살펴보도록 하자.

```java
@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {
    // ...
    @Override
    public void configureMessageBroker(MessageBrokerRegistry registry) {
        // ...
        // 사용자 세션 접근을 위한 접두사
        registry.setUserDestinationPrefix("/user");
    }
}
```

이에 따라서 `/user/queue/chat.log`에 구독을 하였다.

```
$ _INFO_:subscribe destination /user/queue/chat.log success
$ _INFO_:send STOMP message, destination = /app/chat.1.log, content = , header =
```

그 다음 아무런 헤더 없이 1번 채팅방의 채팅 내역을 `/app/chat.1.log`에 SEND 요청을 보내어 확인해보도록 하자.

```
$ _INFO_:send STOMP message, destination = /app/chat.1.log, content = , header =
$ _INFO_:Receive subscribed message from destination /user/queue/chat.log, content = MESSAGE
content-length:1473
message-id:25312829-3674-a7c2-68de-6024cf0a8430-0
subscription:sub-0
content-type:application/json
destination:/user/queue/chat.log
content-length:1473

[{"id":100,"userId":1,"message":"bdbf1689","at":"2025-08-16T06:42:54.636083"},
...
{"id":82,"userId":1,"message":"2345fb","at":"2025-08-16T06:42:36.636083"},
{"id":81,"userId":3,"message":"6d6d2","at":"2025-08-16T06:42:35.636083"}]
```

여기서 마지막으로 조회한 가장 오래된 채팅 ID가 81인 것을 확인할 수 있다. 따라서 커서에 마지막으로 조회한 채팅 ID인 81을 전달하여 다시 요청을 보내보도록 하자.

```
$ _INFO_:send STOMP message, destination = /app/chat.1.log, content = , header = {"cursor": "82"}
$ _INFO_:Receive subscribed message from destination /user/queue/chat.log, content = MESSAGE
content-length:1491
message-id:25312829-3674-a7c2-68de-6024cf0a8430-1
subscription:sub-0
content-type:application/json
destination:/user/queue/chat.log
content-length:1491

[{"id":80,"userId":3,"message":"0650c0569","at":"2025-08-16T06:42:34.636083"},
...
{"id":62,"userId":3,"message":"6ed","at":"2025-08-16T06:42:16.636083"},
{"id":61,"userId":2,"message":"02267b6eb","at":"2025-08-16T06:42:15.636083"}]
```

80번 채팅 ID 부터 61번 채팅 ID까지 조회된 것을 통해 정상적으로 페이지네이션 기능이 동작하는 것을 확인할 수 있다.

# 예외 처리 구현

애플리케이션은 항상 성공 케이스뿐 아니라 예외 케이스가 존재한다. 예를 들어 채팅 저장 과정에서 데이터베이스가 다운되어 채팅 서버와의 커넥션이 유실되면, 사용자는 자신의 메시지가 왜 채팅방에 표시되지 않는지 알기 어렵다. 따라서 서버에서 예외가 발생했을 때 이를 사용자에게 명확히 전달하는 예외 처리가 필요하다.

이때 앞서 토큰 인증 처리에서 구현했던 `StompSubProtocolErrorHandler`로 `ERROR` 프레임을 보내는 방법은 적절하지 않다. STOMP 1.2 명세는 서버가 `ERROR` 프레임을 전송한 직후 연결을 닫아야 한다(MUST)고 규정한다.

> 서버는 무언가 잘못되었을 경우 `ERROR` 프레임을 전송할 수 있다. 이렇게 `ERROR` 프레임을 전송한 이후에는 반드시(MUST) 연결을 종료해야 한다.

이는 스프링 관련 객체를 다룬 Javadoc 역시 언급하고 있는 부분이다.

> Note that the STOMP protocol requires a server to close the connection after sending an ERROR frame. To prevent an ERROR frame from being sent, a handler could return `null` and send a notification message through the broker instead, for example, via a user destination.

하지만 단순 저장 실패 같은 애플리케이션 예외에 대해 세션을 끊어버리면 사용자 경험이 나빠진다고 생각한다.

또한 `StompSubProtocolErrorHandler`는 클라이언트 STOMP 프레임 처리 중 발생한 예외를 `ERROR` 프레임으로 조립하거나 억제하는 용도로 설계된 컴포넌트다. 애플리케이션 레벨 예외 알림 채널로 쓰기보다는, 프로토콜 수준 오류에 한정해 사용하는 편이 적합하다.

## @MessageExceptionHandler

따라서 나는 이 문제에 대해서 스프링이 제공하는 `@MessageExceptionHandler`를 사용해 예외를 잡고, 사용자 전용 목적지(`/user/**`)로 안내 메시지를 보내 연결은 유지하는 방식을 사용하였다. 기본적인 사용법은 다음과 같다.

```java
@ControllerAdvice
@RequiredArgsConstructor
public class StompExceptionHandler {
    @MessageExceptionHandler
    public void handleException(Exception exception) {
        // 예외 처리 로직 구현
    }
}
```

여기서 인자에 정의한 예외가 발생하면 해당 예외에 대해서 적절하게 처리하도록 코드를 작성하면 된다. 메서드의 인자로 전달 받을 수 있는 객체를 공식문서를 참고하여 다음과 같이 정리하였다.

| 메서드 인자                                                                   | 설명                                                                          |
| ----------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| `Message<?>`                                                                  | 전체 메시지 객체                                                              |
| `MessageHeaders`                                                              | 메시지 헤더                                                                   |
| `MessageHeaderAccessor` / `SimpMessageHeaderAccessor` / `StompHeaderAccessor` | 타입 안전한 메시지 헤더                                                       |
| `@Payload`                                                                    | 메시지 페이로드                                                               |
| `@Header("name")`                                                             | 특정 헤더 값                                                                  |
| `@Headers`                                                                    | 모든 헤더에 대한 `Map` 자료구조 (따라서 인자는 `java.util.Map` 타입이어야 함) |
| `@DestinationVariable`                                                        | 메시지 목적지 경로에서 추출된 템플릿 변수                                     |
| `java.security.Principal`                                                     | 웹소켓 HTTP 핸드셰이크 시점의 로그인 사용자 정보                              |

대부분의 STOMP 처리에서 사용되는 객체를 사용할 수 있는 것을 볼 수 있다. 이를 활용하여 예외가 발생하면 특정 사용자에게 현재 데이터베이스가 문제가 생겼다는 메시지를 전송하도록 하자.

서버 구동 중 데이터베이스가 꺼지면 HikariPool이 없다는 경고 로그가 뜨고, 일정 시간이 지나면 `TransactionException`의 하위 클래스인 `CannotCreateTransactionException`이 발생하는 것을 확인하였다. 이를 참고하여 다음과 같이 예외 처리를 하는 메서드를 작성하였다.

```java
@Slf4j
@ControllerAdvice
@RequiredArgsConstructor
public class StompExceptionHandler {

    private final SimpMessagingTemplate messagingTemplate;

    @MessageExceptionHandler
    public void handleTransactionException(
            TransactionException e,
            @Header(name = "simpSessionId") String simpSessionId
    ) {
        log.info("DB 예외 발생!!");

        messagingTemplate.convertAndSendToUser(
                simpSessionId,
                "/queue/chat.error",
                "데이터베이스 에러 발생",
                createHeaders(simpSessionId)
        );
    }

    private MessageHeaders createHeaders(@Nullable String sessionId) {
        SimpMessageHeaderAccessor headerAccessor = SimpMessageHeaderAccessor
                .create(SimpMessageType.MESSAGE);

        if (sessionId != null) {
            headerAccessor.setSessionId(sessionId);
        }
        headerAccessor.setLeaveMutable(true);
        return headerAccessor.getMessageHeaders();
    }
}
```

이제 `/user/queue/chat.error`에 클라이언트를 구독하고, `/app/chat.1`으로 1번 채팅방에 메시지를 보내보고 기다려보면 서버에서 `CannotCreateTransactionException` 이 발생하고, 이에 대해서 다음과 같이 클라이언트에게 메시지가 전송된 것을 확인할 수 있다.

```
$ _INFO_:send STOMP message, destination = /app/chat.1, content = {"userId": "1", "message": "ERROR"}, header =
$ _INFO_:subscribe destination /user/queue/chat.error success
$ _INFO_:Receive subscribed message from destination /user/queue/chat.error, content = MESSAGE
content-length:13
message-id:88f01019-1716-8aca-e091-64a14c64649c-0
subscription:sub-0
content-type:text/plain;charset=UTF-8
destination:/user/queue/chat.error
content-length:13

데이터베이스 에러 발생
```

## 참고: 멀티플랙싱을 통한 토픽 관리

현재 애플리케이션은 채팅용 토픽, 채팅 내역 전송용 토픽, 채팅 중 발생한 오류 처리용 토픽 이렇게 3개를 구독해야 정상적으로 채팅 서비스를 이용할 수 있다. 하지만 채팅 애플리케이션은 여기서 끝이 아니다.

카카오톡을 생각해보자. 단적으로 사용자가 소속된 채팅방의 가장 최근 채팅 내역을 보여주고, 또 사용자가 읽지 않은 채팅 개수를 표기하기도 한다. 심지어 개인적으로는 좋아하지 않는 기능이지만 사용자가 채팅을 입력중이면 해당 사용자가 입력중이라는 표시도 최근에 구현되었다.

이렇게 여러 필수 / 부가 기능들을 각 기능별로 토픽을 생성하여 처리하면, 채팅 서비스에 소켓 관련 기능이 점점 많아질수록 관리해야할 토픽의 개수가 증가할 것이고, 이는 유지보수의 어려움으로 될 가능성이 높다.

이를 위해 멀티플랙싱을 고려할 수 있다. 멀티플랙싱은 네트워크 용어인데, 쉽게 정리하면 여러 개의 입력을 하나의 출력으로 변환한다는 것이다. 이를 채팅 애플리케이션으로 보면 여러 요청을 하나의 토픽에 발행한다는 것으로 생각할 수 있다.

그렇다면 현재 서비스에서는 어떤 토픽을 하나의 토픽으로 관리할 수 있을까? 이에 대해서 나는 채팅 내역을 전송하기 위한 토픽과 채팅 시 발생하는 예외를 전송하기 위한 토픽을 `/user/queue/chat.info`로 처리하면 어떨까 생각하였다. 또한 하나의 토픽으로 합쳐진만큼 클라이언트에서는 해당 토픽을 통해 발행된 메시지가 어떤 타입인지 알아야 해당 타입에 따른 적절한 처리를 할 수 있다고 생각하였다. 따라서 다음과 같은 객체를 작성해보았다.

```java
public record QueueIntegrationResponse<T>(
        Type type,
        T data
) {
    public static <T> QueueIntegrationResponse<T> onError(T data) {
        return new QueueIntegrationResponse<>(Type.ERROR, data);
    }
}

enum Type {
    LOG, ERROR
}
```

그리고 예외 처리 부분에서 단순히 문자열을 담아 메시지를 보낸 것을 다음과 같이 변경하였다.

```java
@MessageExceptionHandler
public void handleTransactionException(
        TransactionException e,
        @Header(name = "simpSessionId") String simpSessionId
) {
    log.info("DB 예외 발생!!");

    messagingTemplate.convertAndSendToUser(
            simpSessionId,
            "/queue/chat.info",
            QueueIntegrationResponse.onError("데이터베이스 에러 발생"),
            createHeaders(simpSessionId)
    );
}
```

이제 클라이언트 로그를 살펴보자.

```
$ _INFO_:Receive subscribed message from destination /user/queue/chat.info, content = MESSAGE
content-length:39
message-id:02ff56a7-8711-4b3e-b7bd-56b5754c181d-0
subscription:sub-0
content-type:application/json
destination:/user/queue/chat.info
content-length:39

{"type":"ERROR","data":"데이터베이스 에러 발생"}
```

이렇게 클라이언트에게 해당 메시지 타입은 어떤 것인지 알리면 클라이언트가 해당 타입을 확인하여 전송한 데이터를 적절하게 처리할 것이다.

---

채팅 저장 관련 기능과 이에 따른 예외 처리까지 알아보았다. 다음 포스팅에서는 카카오톡 기능 중 하나인 채팅을 읽지 않은 인원을 표시하기 위한 방법을 알아보고, 이벤트 리스너를 통해서 채팅방에 접속한 사용자 정보를 조회할 수 있도록 할 것이다.

# 참고 자료

[**웹 소켓 테스트 사이트**](https://jiangxy.github.io/websocket-debug-tool/)

[**WebSocket, STOMP in springboot**](https://www.inflearn.com/course/websocket-stomp-springboot/dashboard)

[**Events**](https://docs.spring.io/spring-framework/reference/web/websocket/stomp/application-context-events.html)

[**커서 기반 페이지네이션(Cursor-based-pagination) vs 오프셋 기반 페이지 네이션(offset-based-pagination**](https://0soo.tistory.com/130)

[**STOMP Protocol Specification, Version 1.2**](https://stomp.github.io/stomp-specification-1.2.html)

[**Class StompSubProtocolErrorHandler**](https://docs.spring.io/spring-framework/docs/current/javadoc-api/org/springframework/web/socket/messaging/StompSubProtocolErrorHandler.html?utm_source=chatgpt.com)

[**Annotated Controllers**](https://docs.spring.io/spring-framework/reference/web/websocket/stomp/handle-annotations.html)

[**[네트워크] 멀티플렉싱(Multiplexing)**](https://12bme.tistory.com/741)
