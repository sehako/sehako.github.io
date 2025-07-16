---
title: STOMP를 활용한 실시간 채팅 서버 구축 - 1

categories:
  - Spring

toc: true
toc_sticky: true
published: true
 
date: 2025-06-03
last_modified_at: 2025-06-03
---

Spring에서 STOMP를 활용하여 실시간 채팅 서버를 단계적으로 구축해나갈 것이다. 최종 구현 목표는 다음과 같다.

- 기본적인 채팅 기능 구현
- 이벤트 및 예외 처리
- 읽음 처리 구현

# STOMP란 무엇인가?

우선 STOMP가 무엇인지 간단하게 알아보도록 하자. STOMP는 Simple (or Streaming) Text Oriented Message Protocol의 약자이다. 이는 메시지 지향 미들웨어(RabbitMQ, Kafka)와 함께 작동되도록 설계된 간단한 텍스트 기반의 프로토콜이다.

STOMP는 다음 명령어들과 함께 동작한다.

- CONNECT
- SEND
- SUBSCRIBE
- UNSUBSCRIBE
- BEGIN
- COMMIT
- ABORT
- ACK
- NACK
- DISCONNECT

STOMP에서 메시지가 전달되는 예시는 다음과 같다.

```
SEND
destination:/queue/a
content-type:text/plain

hello queue a
^@
```

STOMP에서 클라이언트와 서버 간 통신은 여러 줄로 구성된 프레임을 통해 이루어진다. 어렵게 말해서 그렇지 조금 쉽게 표현하자면 각 상황에 따른 명령어가 포함된 요청 메시지라고 봐도 될 것 같다.

아무튼 STOMP를 활용하면 1대1 채팅을 비롯하여 단체 채팅 또한 구현하는 것이 수월하다. 그 이유는 STOMP는 기본적으로 pub/sub 모델을 기반으로 설계된 프로토콜이기 때문이다.

# 스프링에서 채팅 서버 구현하기

본론에 앞서, 채팅 구현에 대한 테스트는 다음 [페이지](https://jiangxy.github.io/websocket-debug-tool/)에서 진행할 것이다. 간단하게 웹 소켓을 테스트 할 수 있는 사이트이다. 스프링에서 STOMP를 활용하여 채팅 서버를 구현하는 방법은 매우 간단하다. 우선 다음 의존성이 필요하다. 

```groovy
implementation 'org.springframework.boot:spring-boot-starter-websocket'
```

## 연결 설정

연결 설정도 설정 빈 하나를 등록하면서 끝이다. 다음 코드를 보도록 하자.

{% include code-header.html %}
```java
@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {
    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOrigins("*");
    }
    
    @Override
    public void configureMessageBroker(MessageBrokerRegistry registry) {
		    // 토픽 구독을 위한 접두사
        registry.enableSimpleBroker("/topic", "/queue");
        // 어플리케이션에 요청을 보낼 때 작성해야 하는 접두사
        registry.setApplicationDestinationPrefixes("/pub");
    }
}
```

이렇게 작성하면 `{서버주소}/ws`로 연결 요청을 보내면 연결이 된다. 참고로 `registerStompEndpoints`메소드에서 다음과 같이 등록할 수도 있다.

{% include code-header.html %}
```java
@Override
public void registerStompEndpoints(StompEndpointRegistry registry) {
    registry.addEndpoint("/ws").setAllowedOrigins("*").withSockJS();
}
```

이는 브라우저가 WebSocket을 지원하지 않는 경우, 대신 HTTP 기반의 통신 방식(Long Polling, XHR Streaming 등)을 자동으로 사용하는 SockJS 에뮬레이션 기능을 활성화하는 옵션이라고 한다. 

즉 해당 옵션이 있는 경우, WebSocket을 지원하지 않는 브라우저에서도 SockJS가 제공하는 대체 방식(Long Polling 등)을 통해 WebSocket 연결과 유사한 통신이 가능하다고 한다.

# 스프링에서 채팅 서버 구현하기

본론에 앞서, 채팅 구현에 대한 테스트는 다음 [페이지](https://jiangxy.github.io/websocket-debug-tool/)에서 진행할 것이다. 간단하게 웹 소켓을 테스트 할 수 있는 사이트이다. 스프링에서 STOMP를 활용하여 채팅 서버를 구현하는 방법은 매우 간단하다. 우선 다음 의존성이 필요하다. 

{% include code-header.html %}
```groovy
implementation 'org.springframework.boot:spring-boot-starter-websocket'
```

## 연결 설정

연결 설정도 설정 빈 하나를 등록하면서 끝이다. 다음 코드를 보도록 하자.

{% include code-header.html %}
```java
@Configuration
@EnableWebSocketMessageBroker
public class StompConfig implements WebSocketMessageBrokerConfigurer {
    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOrigins("*");
    }
    
    @Override
    public void configureMessageBroker(MessageBrokerRegistry registry) {
		    // 토픽 구독을 위한 접두사
        registry.enableSimpleBroker("/topic", "/queue");
        // 어플리케이션에 요청을 보낼 때 작성해야 하는 접두사
        registry.setApplicationDestinationPrefixes("/pub");
    }
}
```

이렇게 작성하면 `{서버주소}/ws`로 연결 요청을 보내면 제대로 연결이 되고, 클라이언트가 요청을 보내려고 하면 요청 주소 앞에 `/pub`을, 구독을 하고자 하면 구독 주소 앞에 `/topic` 이나 `/queue`를 명시하면 된다. `/topic`은 브로드캐스트 용도, `/queue`는 요청을 보낸 사용자에게 응답하는 용도라고 생각하면 된다.

## 채팅 구현

채팅도 매우 간단하다. 

{% include code-header.html %}
```java
public record MessageRequest(
        Long userId
        String message,
) {
}
```

{% include code-header.html %}
```java
public record MessageResponse(
        Long userId,
        String message,
        LocalDateTime at
) {
    public static MessageResponse of(Long userId, String message, LocalDateTime time) {
        return new MessageResponse(userId, message, time);
    }
}
```

{% include code-header.html %}
```java
@Slf4j
@Controller
public class StompController {
    @MessageMapping("/chat")
    @SendTo("/topic/chat")
    public MessageResponse basic(
            Message<MessageRequest> message
    ) {
        MessageRequest request = message.getPayload();
        return MessageResponse.of(
                request.userId(), 
                request.message(), 
                LocalDateTime.now()
        );
    }
}
```

`Message` 객체로 한 번 감싸준 것을 볼 수 있다. 사실 그냥 `MessageRequest`를 그대로 사용해도 무방하지만, 나중에 요청을 보낸 클라이언트의 세션 ID 값을 알아야 할 때에는 저런 식으로 처리하여 `MessageHeader`를 추출하던가, 인자에 `MessageHeader`를 명시해야 한다.

아무튼 중요한 것은 클라이언트가 `/pub/chat`으로 채팅 메시지를 전송하면 `/topic/chat`으로 브로드 캐스트 한다는 것이다.

### 특정 토픽에만 브로드 캐스트 하기

위의 방식을 사용하면 고정된 토픽에만 브로드 캐스팅이 가능하다. 하지만 채팅방이 여러 개인 경우에는 채팅방의 ID같은 고유 값을 통해서 특정 토픽에만 브로드 캐스트 하는 방법이 유용할 수 있다. 

이런 경우에는 `@SendTo`를 사용하지 않고 `SimpMessagingTemplate`을 사용하여 특정 토픽에만 메시지를 발행하도록 처리해야 한다.

{% include code-header.html %}
```java
@Slf4j
@Controller
@RequiredArgsConstructor
public class StompController {
    private final SimpMessagingTemplate messagingTemplate;

    @MessageMapping("/chat.{chatroom-id}")
    public void basic(
            @DestinationVariable("chatroom-id") Long chatroomId,
            Message<MessageRequest> message
    ) {
        MessageRequest request = message.getPayload();

        log.info("headers: {}", message.getHeaders());
        log.info("message: {}", message);

        messagingTemplate.convertAndSend(
                "/topic/chat." + chatroomId,
                MessageResponse.of(
                        request.userId(),
                        request.message(),
                        LocalDateTime.now()
                )
        );
    }
}
```

`@DestinationVariable`은 STOMP 메시지의 destination 경로에서 값을 추출하는 데 사용되며, REST API의 `@PathVariable`과 유사한 역할을 한다고 볼 수 있다.

REST에서는 주로 `/`를 이용해 리소스를 구분한다. 물론 STOMP에서도 `/`로 경로를 구성하지만, topic 네임스페이스를 표현할 때 `.`을 사용한다. 다만 `.`이 STOMP의 필수 구분자는 아니며 관례에 따른 선택이라고 한다.

## 테스트

이제 테스트를 진행해보자. 각각 일반 창과 시크릿 창으로 테스트 사이트에 접속하여 `ws://localhost:8080/ws`으로 소켓 연결을 하였고, 하나는 `/pub/chat.1`에 발행을, 다른 하나는 `/topic/chat.1`을 구독한 상태에서 다음 메시지를 전송해보았다.

{% include code-header.html %}
```json
{"userId": 1, "message": "Hello"}
```

다음 로그를 확인할 수 있다.

**발행자**

```
$ _INFO_:Connect STOMP server success, url = ws://localhost:8080/ws, connectHeader = 
$ _INFO_:send STOMP message, destination = /pub/chat.1, content = {"userId": 1, "message": "Hello"}, header = 
```

**구독자**

```
$ _INFO_:subscribe destination /topic/chat.1 success
$ _INFO_:Receive subscribed message from destination /topic/chat.1, content = MESSAGE
content-length:65
message-id:a3971557-aab2-a055-5fb0-8858c7b86f14-0
subscription:sub-0
content-type:application/json
destination:/topic/chat.1
content-length:65

{"userId":1,"message":"Hello","at":"2025-06-01T23:31:21.8594068"}
```

서버에서 헤더와 메시지에 대한 로그는 다음과 같이 출력되었다.

```
headers: {simpMessageType=MESSAGE, stompCommand=SEND, nativeHeaders={destination=[/pub/chat.1], content-length=[33]}, DestinationVariableMethodArgumentResolver.templateVariables={chatroom-id=1}, simpSessionAttributes={}, simpHeartbeat=[J@754a838, lookupDestination=/chat.1, simpSessionId=b64bd69d-4f12-d14c-8494-b9b12d698b5c, simpDestination=/pub/chat.1}
message: GenericMessage [payload=MessageRequest[userId=1, message=Hello], headers={simpMessageType=MESSAGE, stompCommand=SEND, nativeHeaders={destination=[/pub/chat.1], content-length=[33]}, DestinationVariableMethodArgumentResolver.templateVariables={chatroom-id=1}, simpSessionAttributes={}, simpHeartbeat=[J@754a838, lookupDestination=/chat.1, simpSessionId=b64bd69d-4f12-d14c-8494-b9b12d698b5c, simpDestination=/pub/chat.1}]
```

여기서 `simpSessionId`를 확인할 수 있는데, 이것이 바로 현재 요청을 보낸 클라이언트의 접속 세션 ID 값이다.

---

이것으로 실시간 채팅 기능에 대한 구현이 완료되었다. 생각보다 코드의 양도 많지 않고, 어려운 것도 없었다. 아마 그만큼 스프링이 내부적으로 어려운 부분은 잘 처리해주기 때문일 것이다. 

참고로 STOMP를 학습하기 위해 관련 무료 강의를 수강했는데 1시간이라는 짧은 시간에 기본적인 구현에 필요한 것들은 모두 알려주는 강의인 것 같아 공유하도록 하겠다. 

# 참고 자료

[**Streaming Text Oriented Messaging Protocol**](https://en.wikipedia.org/wiki/Streaming_Text_Oriented_Messaging_Protocol)

[**웹 소켓 테스트 사이트**](https://jiangxy.github.io/websocket-debug-tool/)

[**WebSocket, STOMP in springboot**](https://www.inflearn.com/course/websocket-stomp-springboot/dashboard)