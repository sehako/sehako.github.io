---
title: STOMP를 활용한 실시간 채팅 서버 구축 - 2

categories:
  - Spring

toc: true
toc_sticky: true
published: true
 
date: 2025-08-05
last_modified_at: 2025-08-05
---

# 전통적 웹 애플리케이션에서의 인증

STOMP를 활용하여 간단한 채팅 애플리케이션을 손쉽게 구축할 수 있었다. 그렇다면 인증 / 인가된 사용자만 채팅 애플리케이션을 사용할 수 있도록 하고 싶다면 어떻게 해야할까?

만약 전통적인 스프링 웹 애플리케이션(뷰 까지 한 번에 보여주는 형태)이라면, 추가적인 인증 / 인가 처리가 필요 없다고 한다. 이는 스프링의 웹 소켓의 인증 부분에 대한 공식 문서를 살펴보면 알 수 있다.

> STOMP over WebSocket 세션은 HTTP 요청(웹소켓 핸드셰이크 또는 SockJS HTTP 요청)으로 시작하며, 대부분의 웹 애플리케이션은 이미 Spring Security 등을 통해 HTTP 요청 단계에서 사용자가 인증되어 있다. 이때 인증 정보는 HTTP 세션에 저장되고, Spring은 이를 WebSocket 또는 SockJS 세션과 자동으로 연계해 STOMP 메시지의 `user` 헤더로 전달한다.
> 
> 
> 따라서 일반적인 웹 애플리케이션은 WebSocket을 위해 별도의 인증 로직을 추가할 필요가 없으며, STOMP의 `CONNECT` 프레임에 존재하는 `login`과 `passcode` 헤더는 TCP 기반 STOMP에서만 유효하고 WebSocket 기반 STOMP에서는 Spring이 이를 무시하고 HTTP 수준 인증만 신뢰한다.
> 

# 토큰 인증 처리 구현

JWT 인증 기반의 애플리케이션이라면 이야기가 조금 달라진다. 스프링 웹 소켓 공식 문서에서 토큰 인증 부분에 대해서 다음과 같이 작성되어 있다.

> 동시에, 이러한 쿠키 기반 세션이 항상 정답은 아니다. RFC 6455의 웹소켓 프로토콜에 따르면, 웹 소켓 핸드셰이크 중에 서버가 클라이언트를 인증하는 특정 방법을 규정하지 않는다. 브라우저 클라이언트는 HTTP 표준 인증 헤더 또는 쿠키만 사용할 수 있으며, 사용자 지정 헤더를 제공할 수 없다.
> 
> 
> 마찬가지로 클라이언트(SockJS)는 전송 요청과 함께 HTTP 헤더를 전송하는 방법을 제공하지 않고, 토큰 전송을 위한 쿼리 파라미터를 사용할 수 있지만 토큰이 서버 로그에 URL과 함께 기록될 수 있는 문제를 가지고 있다.
> 

따라서 토큰 기반 인증 처리를 위해서 STOMP 연결을 할 때(CONNECT 요청) 헤더에 인증 정보를 담아서 보내고 서버에서는 `ChannelInterceptor` 를 통해 인증을 처리하여 클라이언트를 식별하도록 할 수 있다고 한다.

토큰 인증 처리 구현에 대한 시나리오를 구성하기 위해서 우선 클라이언트에서 Authorization 헤더에 EXAMPLE_TOKEN을 담아서 보낸다고 가정하였다.

## ChanneInterceptor 설정

공식 문서에 나온 `ChannelInterceptor` 코드는 다음과 같다.

```java
@Configuration
@EnableWebSocketMessageBroker
public class WebSocketConfiguration implements WebSocketMessageBrokerConfigurer {

	@Override
	public void configureClientInboundChannel(ChannelRegistration registration) {
		registration.interceptors(new ChannelInterceptor() {
			@Override
			public Message<?> preSend(Message<?> message, MessageChannel channel) {
				StompHeaderAccessor accessor = MessageHeaderAccessor.getAccessor(message, StompHeaderAccessor.class);
				if (StompCommand.CONNECT.equals(accessor.getCommand())) {
					// Access authentication header(s) and invoke accessor.setUser(user)
				}
				return message;
			}
		});
	}
}
```

`WebSocketMessageBrokerConfigurer` 인터페이스를 구현한 것을 볼 수 있다. 앞서 `StompConfig` 역시 해당 인터페이스를 구현하였으므로 나는 다음과 같이 코드를 작성하였다.

```java
@Slf4j
@Configuration
@EnableWebSocketMessageBroker
public class StompConfig implements WebSocketMessageBrokerConfigurer {
		// 연결 설정 부분 생략
		
    @Override
    public void configureClientInboundChannel(ChannelRegistration registration) {
        registration.interceptors(new ChannelInterceptor() {
            @Override
            public Message<?> preSend(Message<?> message, MessageChannel channel) {
                StompHeaderAccessor accessor = MessageHeaderAccessor.getAccessor(message, StompHeaderAccessor.class);
                // 설정된 요청이 CONNECT일 때
                if (StompCommand.CONNECT.equals(accessor.getCommand())) {
                    log.info("accessor: {}", accessor);
                }
                return message;
            }
        });
    }
}
```

`StompHeaderAccessor` 객체를 확인하기 위해서 로그를 작성해봤다. 그 다음 다음과 같이 연결 하여 로그를 확인해보도록 하자.

![image.png](https://velog.velcdn.com/images/gkrdh99/post/01361241-281f-4d32-ad25-3e6f7938ad81/image.png)

헤더에서 JSON의 키-값 구조로 토큰을 보냈다는 것을 확인할 수 있다. 애플리케이션에서는 다음과 같은 로그를 출력하였다.

```
accessor: StompHeaderAccessor 
[headers={simpMessageType=CONNECT, stompCommand=CONNECT, 
nativeHeaders={Authorization=[EXAMPLE_TOKN], accept-version=[1.1,1.0], 
heart-beat=[10000,10000]}, simpSessionAttributes={}, 
simpHeartbeat=[J@566f4bc8, simpSessionId=26f313e8-87b7-e0b0-04cf-765c038e6067}]
```

`nativeHeaders`에 클라이언트가 전달한 토큰이 존재한다는 것을 확인할 수 있다. 이는 `getNativeHeader(String headerName)` 를 이용하여 조회할 수 있다. 따라서 다음과 같이 인증 절차를 완성하도록 하자.

```java
@Override
public void configureClientInboundChannel(ChannelRegistration registration) {
    registration.interceptors(new ChannelInterceptor() {
        @Override
        public Message<?> preSend(Message<?> message, MessageChannel channel) {
            StompHeaderAccessor accessor = MessageHeaderAccessor.getAccessor(message, StompHeaderAccessor.class);
            // 설정된 요청이 CONNECT일 때
            if (StompCommand.CONNECT.equals(accessor.getCommand())) {
                String token = Objects.requireNonNull(
		                accessor.getNativeHeader("Authorization")).get(0);

                if (token == null || !"EXAMPLE_TOKEN".equals(token)) {
                    log.info("비정상 토큰 감지: {}", token);
                    throw new RuntimeException("Invalid token");
                }
            }
            return message;
        }
    });
}
```

### 테스트

우선 EXAMPLE_TOKEN을 전달하면 정상적으로 연결이 진행되는 것을 확인할 수 있었다.

![image.png](https://velog.velcdn.com/images/gkrdh99/post/ee655022-82e9-4a01-adce-95dda0395d96/image.png)

하지만 그 외에 토큰값을 전달하면 애플리케이션에서 에러가 발생하여 제대로 연결이 되지 않고 서버에서 다음과 같은 로그가 출력되는 것을 확인하였다.

```java
비정상 토큰 감지: EXAMPLE
```

## 적절한 예외 처리 구현

다음 코드를 살펴보도록 하자.

```java
if (token == null || !"EXAMPLE_TOKEN".equals(token)) {
    log.info("비정상 토큰 감지: {}", token);
    throw new RuntimeException("Invalid token");
}
```

현재는 토큰 인증이 실패했을 때 서버에서 클라이언트에게 아무런 정보를 알려주지 않는다. 클라이언트에게 토큰 인증이 실패했다는 것을 알려주기 위해서 두 가지 방법을 사용할 수 있다.

### MessageDeliveryException

단순하게 `MessageDeliveryException` 예외 클래스를 던지면 클라이언트에서 토큰 인증이 처리되지 않았다는 것을 확인할 수 있다.

```java
if (token == null || !"EXAMPLE_TOKEN".equals(token)) {
    log.info("비정상 토큰 감지: {}", token);
    throw new MessageDeliveryException("UNAUTHORIZED");
}
```

이러면 다음과 같은 STOMP ERROR 프레임이 클라이언트에게 전송된다.

```
<<< ERROR
message:UNAUTHORIZED
content-length:0
```

### StompSubProtocolErrorHandler

인증 실패에 대한 여러 상황에 맞는 메시지를 전달하고자 한다면 `StompSubProtocolErrorHandler`를 상속하여 `handleClientMessageProcessingError`를 재정의하고, 여기에 각각의 예외 클래스에 따라서 메시지를 다르게 전달할 수 있다.

다음 예외를 던져봤다.

```java
public class AuthenticationFailedException extends RuntimeException {
    public AuthenticationFailedException(String message) {
        super(message);
    }
}
```

```java
if (token == null || !"EXAMPLE_TOKEN".equals(token)) {
    throw new AuthenticationFailedException("UNAUTHORIZED");
}
```

이를 `Throwable.getCause()`를 통해 다음과 같이 예외 발생 시 상황에 맞는 오류 메시지를 클라이언트에게 전달할 수 있다.

```java
@Configuration
public class StompErrorHandler extends StompSubProtocolErrorHandler {

    @Override
    public Message<byte[]> handleClientMessageProcessingError(
            Message<byte[]> clientMessage, Throwable ex) {

        // preSend에서 던진 "UNAUTHORIZED" 메시지 처리
        if (ex.getCause() instanceof AuthenticationFailedException) {
            return createErrorMessage("토큰 인증에 실패했습니다.");
        }
        return super.handleClientMessageProcessingError(clientMessage, ex);
    }

    private Message<byte[]> createErrorMessage(String errMsg) {
        StompHeaderAccessor accessor = StompHeaderAccessor.create(StompCommand.ERROR);
        accessor.setMessage(errMsg);
        accessor.setLeaveMutable(true);
        return MessageBuilder.createMessage(errMsg.getBytes(StandardCharsets.UTF_8),
                accessor.getMessageHeaders());
    }
}
```

이후 `StompConfig`에 다음과 같이 작성한 `StompErrorHandler`를 등록하였다. 

```java
@Slf4j
@Configuration
@EnableWebSocketMessageBroker
public class StompConfig implements WebSocketMessageBrokerConfigurer {
    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOrigins("*");

				// 에러 핸들러 등록
        registry.setErrorHandler(new StompErrorHandler());
    }
    
    // ...
    
}
```

이제 잘못된 토큰으로 연결을 시도하면 다음과 같은 에러 메시지가 반환되는 것을 확인할 수 있다.

```
<<< ERROR
message:토큰 인증에 실패했습니다.
content-length:36

토큰 인증에 실패했습니다.
```

## 참고: 스프링 시큐리티와 함께 사용하는 경우

공식 문서에서 스프링 시큐리티와 함께 `ChannelInterceptor`를 사용하는 것에 대한 주의 사항을 짧게 적어놓았다.

> 스프링 시큐리티의 메시지 권한 부여 기능을 사용할 때는 `ChannelInterceptor` 구성이 Spring Security보다 우선적으로 처리되어야 한다. 이를 위해서는 `@Order(Ordered.HIGHEST_PRECEDENCE + 99)`로 표시된 `WebSocketMessageBrokerConfigurer` 구현체에 커스텀 인터셉터를 선언해야 한다.
> 

따라서 스프링 시큐리티를 사용하는 경우에 `StompConfig`에 다음 어노테이션을 선언하도록 하자.

```java
@Slf4j
@Configuration
@EnableWebSocketMessageBroker
@Order(Ordered.HIGHEST_PRECEDENCE + 99)
public class StompAuthConfig implements WebSocketMessageBrokerConfigurer {
		// ...
}
```

---

토큰 인증 처리에 대해서 살펴보았다. 다음 포스팅은 채팅방에서 클라이언트가 전송한 채팅을 MongoDB에 저장하고, 다른 사용자가 접속하면 이를 읽음 처리하는 기능을 구현해나갈 것이다.

# 참고 자료

[**Authentication**](https://docs.spring.io/spring-framework/reference/web/websocket/stomp/authentication.html)

[**Token Authentication**](https://docs.spring.io/spring-framework/reference/web/websocket/stomp/authentication-token-based.html)

[**RFC 6455**](https://datatracker.ietf.org/doc/html/rfc6455#section-10.5)

[**웹 소켓 테스트 사이트**](https://jiangxy.github.io/websocket-debug-tool/)

[**Spring WebSocket 예외 처리 - @MessageExceptionHandler, StompSubProtocolErrorHandler**](http://shout-to-my-mae.tistory.com/434#%EB%B9%84%EC%A6%88%EB%8B%88%EC%8A%A4%20%EB%A1%9C%EC%A7%81%EC%97%90%20%EB%8C%80%ED%95%9C%20%EA%B2%80%EC%A6%9D%20%EC%98%88%EC%99%B8%20%EC%B2%98%EB%A6%AC-1)