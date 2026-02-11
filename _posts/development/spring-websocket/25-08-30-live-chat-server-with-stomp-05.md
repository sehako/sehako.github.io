---
title: 실시간 채팅 개발 - 스케일 아웃

categories:
  - Spring
  - WebSocket

toc: true
toc_sticky: true
published: true

date: 2025-08-30
last_modified_at: 2025-08-30
---

# 소켓 서버 스케일아웃의 문제점

채팅 애플리케이션을 설정부터 인증, 예외 처리, 읽음 처리까지 모두 개발하였다. 이제 채팅 서버를 스케일 아웃 했을 때 발생하는 문제를 살펴보도록 하자. 애플리케이션에 다음과 같은 설정을 추가하였다.

```yaml
server:
	port: 0
```

이는 애플리케이션이 실행될 때마다 시스템이 자동으로 빈 포트를 할당하도록 하는 설정이다. 이를 통해 서버 인스턴스 간의 포트 충돌을 방지할 수 있으며, 클라이언트는 개별 서버의 포트를 알 필요가 없다. 모든 요청은 로드 밸런서를 거쳐 들어오고, 로드 밸런서는 설정된 정책에 따라 등록된 서버 인스턴스에 요청을 분배한다.

아무튼 이렇게 설정을 하고 두 개의 서버 인스턴스를 실행해보도록 하자. IntelliJ에서는 Run/Debug Configurations에서 다음 아이콘을 클릭하면 된다.

![image.png](/assets/images/development/spring-websocket/25-08-30-live-chat-server-with-stomp-05/01.png)

![image.png](/assets/images/development/spring-websocket/25-08-30-live-chat-server-with-stomp-05/02.png)

복제한 두 개의 서버 인스턴스가 랜덤 포트로 실행된 것을 확인할 수 있을 것이다.

```
# 1
Tomcat started on port 49602 (http) with context path '/'
# 2
Tomcat started on port 49632 (http) with context path '/'
```

## 테스트

이제 테스트를 해보자. 로드 벨런서에 의해서 1번 사용자는 1번 서버 인스턴스에, 2번 사용자는 2번 서버 인스턴스에 접속하여 1번 채팅방에서 서로 채팅을 나눈다고 가정하자.

**1번 사용자**

```
# ws://localhost:49602/w

>>> SEND
destination:/app/chat.1
content-length:33
{"userId": "1", "message": "Hi?"}

Received data
<<< MESSAGE
content-length:93
message-id:0571ef11-8ec7-247e-f8d8-3d562217b1fe-0
subscription:sub-0
content-type:application/json
destination:/topic/chat.1
content-length:93
headers:
content-length: 93
message-id: 0571ef11-8ec7-247e-f8d8-3d562217b1fe-0
subscription: sub-0
content-type: application/json
destination: /topic/chat.1
{"id":102,"userId":1,"message":"Hi?","unreadCount":2,"createdAt":"2025-08-29T12:01:43.67323"}
```

1번 사용자는 자신이 보낸 메시지를 받을 수 있었다. 하지만 2번 사용자는 아무런 메시지도 받지 못한다. 그렇다면 채팅 저장이 실패한 것일까? 2번 사용자로 최신 채팅 내역 20개를 불러오도록 해보자.

**2번 사용자**

```
# ws://localhost:49632/ws
>>> SUBSCRIBE
ack:auto
id:sub-3
destination:/user/queue/chat.log

>>> SEND
destination:/app/chat.1.log

Received data
<<< MESSAGE
content-length:1947
message-id:764b40c1-3ac9-4460-7f90-07f479b5cb5f-1
subscription:sub-3
content-type:application/json
destination:/user/queue/chat.log
content-length:1947
headers:
content-length: 1947
message-id: 764b40c1-3ac9-4460-7f90-07f479b5cb5f-1
subscription: sub-3
content-type: application/json
destination: /user/queue/chat.log
[
{"id":102,"userId":1,"message":"Hi?","unreadCount":2,"createdAt":"2025-08-29T12:01:43.67323"}
...
]
```

채팅 내역은 또 정상적으로 불러올 수 있다는 것을 확인하였다. 이 문제를 살펴보도록 하자.

## 인메모리 메시지 브로커

이렇게 채팅 서버를 수평적 확장하면 같은 채팅방에서 채팅을 나누어도 실시간으로 통신을 할 수 없다는 것을 확인하였다. 그 이유는 첫 번째 포스팅에서 STOMP에 대해서 언급할 때 짧게 다룬 바 있다.

> STOMP는 Simple (or Streaming) Text Oriented Message Protocol의 약자이다. 이는 메시지 지향 미들웨어(RabbitMQ, Kafka)와 함께 작동되도록 설계된 간단한 텍스트 기반의 프로토콜이다.

하지만 그 동안의 포스팅에서 메시지 지향 미들웨어를 따로 지정한 적이 없고, 컨테이너도 PostgreSQL만 실행했다. 이렇게 메시지 지향 미들웨어를 사용하지 않는 경우에는 스프링은 자체적으로 인메모리에서 메시지 지향 미들웨어를 활용한다. 그리고 이 경우에는 스케일 아웃이 불가능하다. 이는 실제로 스프링 웹소켓 관련 문서에서도 다음과 같이 언급한다.

> The simple broker is great for getting started but supports only a subset of STOMP commands (e.g. no acks, receipts, etc.), relies on a simple message sending loop, and is not suitable for clustering. As an alternative, applications can upgrade to using a full-featured message broker.

간단하게 번역하자면 인메모리 브로커(simple broker)는 몇 가지 기능을 지원하지 않고, 스케일 아웃(clustering)에는 적합하지 않다고 한다.

실제로 인메모리 브로커가 어떻게 동작하는지 확인하기 위해 `SimpMessageTemplate.convertAndSend()` 호출 시의 디버깅 과정을 살펴보자. 디테일한 내부 동작에 관심이 없다면, 바로 외부 브로커 활용 파트로 넘어가도 된다.

### SimpMessagingTemplate

이 메서드를 호출하면 내부적으로 `AbstractMessageSendingTemplate<O>.convertAndSend()`가 호출되고 다음과 같은 코드가 동작한다.

```java
@Override
public void convertAndSend(
        D destination,
        Object payload,
        @Nullable Map<String, Object> headers,
		@Nullable MessagePostProcessor postProcessor
) throws MessagingException {
	Message<?> message = doConvert(payload, headers, postProcessor);
	send(destination, message);
}

@Override
public void send(D destination, Message<?> message) {
	doSend(destination, message);
}
```

여기서 메시지의 변환이 일어나고, 여기서 `doSend()`를 호출하면 다시 `SimpMessagingTemplate.doSend()`가 호출된다.

```java
@Override
protected void doSend(String destination, Message<?> message) {
	// ...
	sendInternal(message); // # 161
	// ...
}

private void sendInternal(Message<?> message) {
	String destination = SimpMessageHeaderAccessor.getDestination(message.getHeaders());
	Assert.notNull(destination, "Destination header required");

	long timeout = this.sendTimeout;
	boolean sent = (timeout >= 0 ? this.messageChannel.send(message, timeout) : this.messageChannel.send(message));

	if (!sent) {
		throw new MessageDeliveryException(message,
				"Failed to send message to destination '" + destination + "' within timeout: " + timeout);
	}
}
```

굉장히 긴 코드가 있지만 대충 요약하자면 헤더가 없으면 헤더를 생성하고, 메시지가 없다면 메시지를 빌드한다고 생각하면 된다. 그 다음 `snedInternal()`을 호출하면 `MessageChannel`의 구현 겸 추상 클래스인 `AbstractMessageChannel.send()`가 호출된다.

### AbstractMessageChannel

```java
@Override
public final boolean send(Message<?> message) {
	return send(message, INDEFINITE_TIMEOUT);
}

@Override
public final boolean send(Message<?> message, long timeout) {
	// ...
	sent = sendInternal(messageToUse, timeout); // # 139
	// ...
}
```

이후 MessageChannel의 또 다른 구현체인 `ExecutorSubscribableChannel.sendInternal()`이 호출된다.

```java
@Override
public boolean sendInternal(Message<?> message, long timeout) {
	for (MessageHandler handler : getSubscribers()) {
		SendTask sendTask = new SendTask(message, handler);
		if (this.executor != null) {
			try {
				this.executor.execute(sendTask);
			}
			catch (RejectedExecutionException ex) {
				sendTask.run();
			}
		}
		else {
			sendTask.run();
		}
	}
	return true;
}

private class SendTask implements MessageHandlingRunnable {

	private final Message<?> inputMessage;
	// ...

	@Override
	public void run() {
		Message<?> message = this.inputMessage;
		try {
			// ...
			this.messageHandler.handleMessage(message); // # 152
		}
		// ...
	}
}
```

여기서 스레드 풀 관련 프레임워크인 `Executor`의 유무에 따라서 호출 방법이 결정되며, 어떤 조건문을 거치던간에 최종적으로는 `Runnable` 을 구현한 내부 클래스인 `SendTask.run()`이 실행된다.

### AbstractBrokerMessageHandler

그리고 `SendTask.run()` 에서 `AbstractBrokerMessageHandler.handleMessage()`를 호출하고,

```java
@Override
public void handleMessage(Message<?> message) {
	if (!this.running) {
		if (logger.isTraceEnabled()) {
			logger.trace(this + " not running yet. Ignoring " + message);
		}
		return;
	}
	handleMessageInternal(message);
}
```

여기서는 최종적으로 `SimpleBrokerMessageHandler.handlerMessageInternal()`을 호출한다.

### SimpleBrokerMessageHandler

```java
private SubscriptionRegistry subscriptionRegistry;
private final Map<String, SessionInfo> sessions = new ConcurrentHashMap<>();

@Override
protected void handleMessageInternal(Message<?> message) {
	MessageHeaders headers = message.getHeaders();
	String destination = SimpMessageHeaderAccessor.getDestination(headers);
	String sessionId = SimpMessageHeaderAccessor.getSessionId(headers);

	// ..

	SimpMessageType messageType = SimpMessageHeaderAccessor.getMessageType(headers);
	if (SimpMessageType.MESSAGE.equals(messageType)) {
		logMessage(message);
		sendMessageToSubscribers(destination, message); // # 316
	}
	// ...
}

protected void sendMessageToSubscribers(@Nullable String destination, Message<?> message) {
	MultiValueMap<String,String> subscriptions = this.subscriptionRegistry.findSubscriptions(message);
	// ...
	long now = System.currentTimeMillis();
	subscriptions.forEach((sessionId, subscriptionIds) -> {
		for (String subscriptionId : subscriptionIds) {
		  // ...
			SessionInfo info = this.sessions.get(sessionId);
			if (info != null) {
				try {
					info.getClientOutboundChannel().send(reply);
				}
				// ...
			}
		}
	});
}
```

여기서 각각의 클라이언트에 대한 세션 정보를 통해 각 클라이언트가 보유한 `MessageChannel`을 이용하여 최종적으로 메시지를 브로드캐스트 한다. 메시지 처리 과정을 정리하자면 다음과 같다.

`Application` → `SimpMessagingTemplate` → `AbstractMessageChannel` → `ExecutorSubscribableChannel` → `SendTask` → `AbstractBrokerMessageHandler` → `SimpleBrokerMessageHandler` → `ClientOutboundChannel` → `WebSocket Client`

기나긴 디버깅 과정을 통해서 다음 사실을 알 수 있다.

- 클라이언트의 세션과 구독 정보는 `ConcurrentHashMap`을 통해 인메모리로 관리된다.

세션 정보는 위 코드를 통해 `ConcurrentHashMap`으로 관리되는 것을 알 수 있다. 하지만 구독 정보는 아직 살펴보지 않았다. 따라서 `SubscriptionRegistry`의 기본 구현체인 `DefaultSubscriptionRegistry`를 살펴보도록 하자.

`DefaultSubscriptionRegistry`는 `AbstractSubscriptionRegistry`를 상속하며, `AbstractSubscriptionRegistry.findSubscriptions()`를 호출하면 내부적으로 `DefaultSubscriptionRegistry.findSubscriptionsInternal()`을 호출한다.

```java
@Override
protected MultiValueMap<String, String> findSubscriptionsInternal(String destination, Message<?> message) {
	MultiValueMap<String, String> allMatches = this.destinationCache.getSubscriptions(destination);
	if (!this.selectorHeaderInUse) {
		return allMatches;
	}
	MultiValueMap<String, String> result = new LinkedMultiValueMap<>(allMatches.size());
	allMatches.forEach((sessionId, subscriptionIds) -> {
		SessionInfo info = this.sessionRegistry.getSession(sessionId);
		if (info != null) {
			for (String subscriptionId : subscriptionIds) {
				Subscription subscription = info.getSubscription(subscriptionId);
				if (subscription != null && evaluateExpression(subscription.getSelector(), message)) {
					result.add(sessionId, subscription.getId());
				}
			}
		}
	});
	return result;
}
```

더 깊게 들어가면 머리가 아파질 질 것 같으니 여기서 멈추도록 하자… 아무튼 여기서는 내부 정적 불변 클래스인 `SessionRegistry`를 사용하고, 드디어 여기서 `ConcurrentHashMap` 으로 각 세션 ID가 가지는 구독 정보를 관리하는 것을 알 수 있다.

```java
private static final class SessionRegistry {

	private final ConcurrentMap<String, SessionInfo> sessions = new ConcurrentHashMap<>();

	@Nullable
	public SessionInfo getSession(String sessionId) {
		return this.sessions.get(sessionId);
	}

	public void forEachSubscription(BiConsumer<String, Subscription> consumer) {
		this.sessions.forEach((sessionId, info) ->
			info.getSubscriptions().forEach(subscription -> consumer.accept(sessionId, subscription)));
	}

	public void addSubscription(String sessionId, Subscription subscription) {
		SessionInfo info = this.sessions.computeIfAbsent(sessionId, _sessionId -> new SessionInfo());
		info.addSubscription(subscription);
	}

	@Nullable
	public SessionInfo removeSubscriptions(String sessionId) {
		return this.sessions.remove(sessionId);
	}
}
```

- 브로드캐스트는 마법이 아니다.

브로드캐스트는 클라이언트가 목적지를 담아서 메시지를 보내면, 앞서 살펴봤던 접속 정보와 구독 정보를 적절하게 조회한 다음 반복문을 통해서 현재 전송하려는 토픽을 구독한 모든 사용자에게 순차적으로 메시지를 전달하는 로직인 것이다.

반대로 생각해보면 `snedToUser()`도 서버에서 전달하려는 토픽을 구독한 클라이언트 중에서 전송 대상 세션 ID가 일치하는 클라이언트에게 메시지를 전송한다고 유추할 수 있다.

---

디버깅 과정이 상당히 길었다. 하지만 이를 통해서 채팅 서버가 `ConcurrentHashMap`을 통해서 각 클라이언트의 세션 정보와 구독 정보를 관리하고, 이를 활용하여 메시지를 전달하는 것을 확인할 수 있었다.

따라서 여러 서버 인스턴스가 존재하면, 각각의 서버 인스턴스에서 `ConcurrentHashMap`을 통해 클라이언트 접속 정보를 관리하고, 이를 통해 클라이언트에게 메시지를 보내기 때문에 서버 인스턴스가 다르면 실시간 통신을 처리하지 못하는 것이다.

# 외부 메시지 브로커

이를 해결하기 위해서 각 서버 인스턴스에서 자체적인 세션 정보로 메시지를 처리하는 것이 아닌, 서버 인스턴스와는 무관한 메시지 브로커를 둬서, 클라이언트에서 접속한 서버 인스턴스가 불일치해도 메시지를 주고받을 수 있도록 해야한다. 이를 위해서 활용하는 메시지 브로커는 대표적으로 3개가 존재하는데 Redis, RabbitMQ, Apache Kafka이다. 하나하나 순차적으로 알아보도록 하자.

## 사전 설정

우선 각각의 외부 브로커를 도커 컨테이너로 띄워서 필요할 때마다 동작시키도록 하겠다.

**Redis**

```bash
docker run -d --name redis-broker -p 6379:6379 redis:latest
```

**RabbitMQ**

```bash
docker run -d \
  --name rabbitmq \
  -p 5672:5672 \
  -p 15672:15672 \
  -p 61613:61613 \
  -e RABBITMQ_DEFAULT_USER=admin \
  -e RABBITMQ_DEFAULT_PASS=admin \
  rabbitmq:3-management
```

```bash
docker exec -it rabbitmq rabbitmq-plugins enable rabbitmq_stomp
```

**Kafka**

```bash
docker run -d --name=kafka -p 9092:9092 \
  -e KAFKA_PROCESS_ROLES=broker,controller \
  -e KAFKA_NODE_ID=1 \
  -e KAFKA_LISTENERS=CONTROLLER://localhost:9093,PLAINTEXT://0.0.0.0:9092 \
  -e KAFKA_ADVERTISED_LISTENERS=PLAINTEXT://localhost:9092 \
  -e KAFKA_LISTENER_SECURITY_PROTOCOL_MAP=CONTROLLER:PLAINTEXT,PLAINTEXT:PLAINTEXT \
  -e KAFKA_CONTROLLER_LISTENER_NAMES=CONTROLLER \
  -e KAFKA_CONTROLLER_QUORUM_VOTERS=1@localhost:9093 \
  -e KAFKA_INTER_BROKER_LISTENER_NAME=PLAINTEXT \
  -e KAFKA_OFFSETS_TOPIC_REPLICATION_FACTOR=1 \
  apache/kafka:latest
```

## Redis Pub/Sub

우선 구현 전에 의존성과 애플리케이션 설정을 먼저 하도록 하자.

**build.gradle**

```groovy
implementation 'org.springframework.boot:spring-boot-starter-data-redis'
```

**application.yml**

```yaml
spring:
  data:
    redis:
      host: localhost
      port: 6379
```

### Redis Config

```java
@Configuration
@RequiredArgsConstructor
public class RedisConfig {

    @Bean
    public RedisTemplate<String, Object> redisTemplate(RedisConnectionFactory redisConnectionFactory) {
        RedisTemplate<String, Object> redisTemplate = new RedisTemplate<>();
        redisTemplate.setKeySerializer(new StringRedisSerializer());
        redisTemplate.setValueSerializer(new StringRedisSerializer());
        redisTemplate.setConnectionFactory(redisConnectionFactory);
        return redisTemplate;
    }

    // RedisMessageListenerContainer 빈을 생성하는 메서드
    // Redis 메시지를 수신하고 리스너에 전달하는 컨테이너를 설정.
    @Bean
    public RedisMessageListenerContainer redisMessage(
            RedisConnectionFactory connectionFactory,
            MessageListenerAdapter listenerAdapterChatMessage,
            ChannelTopic channelTopic
    ) {
        RedisMessageListenerContainer container = new RedisMessageListenerContainer();
        container.setConnectionFactory(connectionFactory);
        container.addMessageListener(listenerAdapterChatMessage, channelTopic);
        return container;
    }

    // 정의된 리스너의 빈을 전달하면서 구독을 처리하는 메서드의 이름을 지정
    @Bean
    public MessageListenerAdapter listenerAdapterChatMessage(RedisSubscriber subscriber) {
        return new MessageListenerAdapter(subscriber, "sendMessage");
    }

    @Bean
    public ChannelTopic chatRoomTopic() {
        return new ChannelTopic("/topic/chat");
    }
}
```

`RedisMessageListenerContainer`를 통해 Redis 메시지를 수신하고 `MessageListenerAdapter` 에 전달하도록 설정한다. `RedisSubscriber`는 기본 제공 클래스가 아닌 개발자 직접 정의 클래스로, 여기에 존재하는 `sendMessage(String message)` 메서드가 전달받은 메시지를 처리하게 된다.

### RedisSubscriber

```java
@Service
@RequiredArgsConstructor
public class RedisSubscriber {

    private final ObjectMapper objectMapper;
    private final SimpMessageSendingOperations messageSendingOperations;

    public void sendMessage(String publishMessage) {
        try {
            MessageResponse response = objectMapper.readValue(publishMessage, MessageResponse.class);
            messageSendingOperations
                    .convertAndSend(
                            "/topic/chat." + response.chatroomId(),
                            response
                    );
        } catch (JsonProcessingException e) {
            throw new RuntimeException("메세지 전송에 실패하였습니다.");
        }
    }
}
```

이 코드를 작성하면서 그동안의 포스팅에서 저지른 치명적 실수를 발견하였다… 현재 구독한 레디스 토픽을 처리하는 `RedisSubscriber`에서 `SimpMessageSendingOperations`를 사용한 것을 볼 수 있다.

이는 디버깅을 찍어보면 내부적으로 `SimpMessagingTemplate`을 주입받는다. 따라서 본질적으로는 별 차이 없지만 인터페이스를 활용하여 객체를 주입받아야 결합도가 느슨해지기 때문에 `SimpMessagingTemplate`를 사용해야만 하는 다음 상황이 아니라면 `SimpMessageSendingOperations`를 활용하도록 하자.

chatGPT에게 질문하여 다음과 같은 상황에서는 `SimpMessagingTemplate`를 사용하는 상황이 나을 수 있다는 답변을 받았다. 참고하도록 하자.

- 커스텀 메시지 변환기를 설정할 때 (JSON 이외의 포맷 사용, `setMessageConverter()` )
- STOMP 헤더를 세밀하게 조작해야 할 때 (`SimpMessageHeaderAccessor`를 통한 헤더 설정)
- `MessageChannel`을 직접 제어해야 할 때 (특정 채널로 메시지를 직접 전달하는 경우)
- 시스템 이벤트나 백엔드 작업(스케줄러, 이벤트 리스너, 알림 서비스 등)에서 메시지를 전송할 때
- 테스트에서 실제 메시지 전송 동작을 검증하고 싶을 때
- 그 외 인터페이스(`SimpMessageSendingOperations`)로는 제공되지 않는 세부 기능이 필요할 때

### 서비스

서비스에서는 컨트롤러에서 전달받은 메시지를 `RedisTemplate.converAndSend()`에 문자열 형식으로 객체를 변환하여 전달하면 된다.

```java
@Service
@Transactional
@RequiredArgsConstructor
public class ChatService {
    private final ChatLogRepository chatLogRepository;
    private final ChatroomUserRepository chatroomUserRepository;
    private final RedisTemplate<String, String> redisTemplate;
    private final ChannelTopic channelTopic;
    private final ObjectMapper objectMapper;

    public void saveMessage(MessageSendingDto dto) throws JsonProcessingException {
        ChatLog savedChatLog = saveChatLog(dto);
        Long unreadCount = chatroomUserRepository.countByChatroomId(savedChatLog.getChatroomId());
        redisTemplate.convertAndSend(
                channelTopic.getTopic(),
                objectMapper.writeValueAsString(
                        MessageResponse.of(
                                savedChatLog.getId(),
                                savedChatLog.getChatroomId(),
                                savedChatLog.getUserId(),
                                savedChatLog.getMessage(),
                                unreadCount - 1,
                                savedChatLog.getCreatedAt()
                        )
                )
        );
    }

    private ChatLog saveChatLog(MessageSendingDto dto) {
        return chatLogRepository.save(dto.toChatLog());
    }

    // ...
}
```

이때 컨트롤러에서 브로드캐스트를 처리하지 않기 때문에 메시지 객체에 채팅방 ID가 존재해야 한다. 따라서 응답 객체를 다음과 같이 수정하였다.

```java
public record MessageResponse(
        Long id,
        Long chatroomId,
        Long userId,
        String message,
        Long unreadCount,
        LocalDateTime createdAt
) {
    public static MessageResponse of(Long id, Long chatroomId, Long userId, String message, Long unreadCount,
                                     LocalDateTime time) {
        return new MessageResponse(id, chatroomId, userId, message, unreadCount, time);
    }
}
```

### 컨트롤러

컨트롤러에서는 단순히 요청받은 객체를 DTO로 변환하여 서비스에 전달하면 된다.

```java
@Slf4j
@Controller
@RequiredArgsConstructor
public class StompController {
    private final ChatService chatService;
    private final SimpMessagingTemplate messagingTemplate;

    @MessageMapping("/chat.{chatroom-id}")
    public void sendMessage(
            @DestinationVariable("chatroom-id") Long chatroomId,
            MessageRequest message
    ) throws JsonProcessingException {
        chatService.saveMessage(message.toMessageSendingDto(chatroomId));
    }
    // ...
}
```

## RabbitMQ

RabbitMQ를 사용하기 위해서는 다음 의존성과 설정이 필요하다.

**build.gradle**

```groovy
implementation 'org.springframework.boot:spring-boot-starter-amqp'
implementation 'io.projectreactor.netty:reactor-netty'
```

**application.yml**

```yaml
spring:
  rabbitmq:
    host: localhost
    stomp-port: 61613
    port: 5672
    username: admin
    password: admin
```

### StompConfig

```java
@Configuration
@EnableWebSocketMessageBroker
public class StompConfig implements WebSocketMessageBrokerConfigurer {
    @Value("${spring.rabbitmq.host}")
    private String RELAY_HOST;
    @Value("${spring.rabbitmq.stomp-port}")
    private Integer RELAY_PORT;
    @Value("${spring.rabbitmq.username}")
    private String RELAY_USERNAME;
    @Value("${spring.rabbitmq.password}")
    private String RELAY_PASSWORD;

    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOrigins("*");

        registry.setErrorHandler(new StompErrorHandler());
    }

    @Override
    public void configureMessageBroker(MessageBrokerRegistry registry) {
        // 외부의 RabbitMQ 브로커를 사용하기 위한 설정
        // TOPIC만 사용하도록 허용
        registry.enableStompBrokerRelay("/topic", "/queue")
                .setRelayHost(RELAY_HOST)
                .setRelayPort(RELAY_PORT)
                .setClientLogin(RELAY_USERNAME)
                .setClientPasscode(RELAY_PASSWORD)
                .setSystemLogin(RELAY_USERNAME)
                .setSystemPasscode(RELAY_PASSWORD)
                .setVirtualHost("/")
                .setSystemHeartbeatSendInterval(10000)
                .setSystemHeartbeatReceiveInterval(10000);
        // 어플리케이션에 요청을 보낼 때 작성해야 하는 접두사
        registry.setApplicationDestinationPrefixes("/app");

        registry.setUserDestinationPrefix("/user");
    }
}
```

이렇게 RabbitMQ를 외부 브로커로 설정하면 `SimpleBrokerMessageHandler` 대신에 `StompBrokerRelayMessageHandler`가 메시지를 처리한다.

### 서비스

이렇게 설정만 하면 애플리케이션에서는 `SimpMessageSendingOperations`나 `SimpMessagingTemplate`를 활용해서 메시지를 처리하면 된다.

```java
@Service
@Transactional
@RequiredArgsConstructor
public class ChatService {
    private final ChatLogRepository chatLogRepository;
    private final ChatroomUserRepository chatroomUserRepository;
    private final SimpMessageSendingOperations operations;

    public void saveMessage(MessageSendingDto dto) {
        ChatLog savedChatLog = saveChatLog(dto);
        Long unreadCount = chatroomUserRepository.countByChatroomId(savedChatLog.getChatroomId());
        operations.convertAndSend(
                "/topic/chat." + savedChatLog.getChatroomId(),
                MessageResponse.of(
                        savedChatLog.getId(),
                        savedChatLog.getChatroomId(),
                        savedChatLog.getUserId(),
                        savedChatLog.getMessage(),
                        unreadCount - 1,
                        savedChatLog.getCreatedAt()
                )
        );
    }
    // ...
}
```

## Kafka

Kafka는 RabbitMQ와는 다르게 STOMP 설정에서 브로커를 선택하지 않고, 자체적으로 Kafka를 통해서 메시지를 발행 및 구독하여 처리한다. Kafka를 사용하기 위해서는 다음 의존성과 설정이 필요하다.

**build.gradle**

```groovy
implementation 'org.springframework.kafka:spring-kafka:3.3.9'
```

**application.yml**

```yaml
spring:
  kafka:
    bootstrap-servers: localhost:9092
```

그리고 앞서 RabbitMQ에서 진행했던 외부 브로커 설정 코드를 원복하도록 하자.

### StompConfig

```groovy
@Configuration
@EnableWebSocketMessageBroker
public class StompConfig implements WebSocketMessageBrokerConfigurer {
    @Override
    public void registerStompEndpoints(StompEndpointRegistry registry) {
        registry.addEndpoint("/ws").setAllowedOrigins("*");
        registry.setErrorHandler(new StompErrorHandler());
    }

    @Override
    public void configureMessageBroker(MessageBrokerRegistry registry) {
        // 토픽 구독을 위한 접두사
        registry.enableSimpleBroker("/topic", "/queue");
        // 어플리케이션에 요청을 보낼 때 작성해야 하는 접두사
        registry.setApplicationDestinationPrefixes("/app");

        registry.setUserDestinationPrefix("/user");
    }
}
```

### KafkaProducerConfig

Kafka의 특정 토픽에 메시지를 발행하기 위해서 사용하는 `KafkaTemplate`를 설정하는 코드를 작성하자.

```java
@Configuration
public class KafkaProducerConfig {

    @Value("${spring.kafka.bootstrap-servers}")
    private List<String> bootstrapServers;

    @Bean
    public ProducerFactory<String, MessageResponse> producerFactory() {
        Map<String, Object> properties = new HashMap<>();
        properties.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, bootstrapServers);
        properties.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG, StringSerializer.class);
        properties.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, JsonSerializer.class);

        return new DefaultKafkaProducerFactory<>(properties);
    }

    @Bean
    public KafkaTemplate<String, MessageResponse> kafkaProducerTemplate() {
        return new KafkaTemplate<>(producerFactory());
    }
}
```

### KafkaConsumerConfig

```java
@EnableKafka
@Configuration
public class KafkaConsumerConfig {

    @Value("${spring.kafka.bootstrap-servers}")
    private List<String> bootstrapServers;
    @Value("${kafka.consumer.group-id}")
    private String groupId;

    @Bean
    public ConsumerFactory<String, MessageResponse> consumerFactory() {
        Map<String, Object> properties = new HashMap<>();
        properties.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, bootstrapServers);
        properties.put(ConsumerConfig.GROUP_ID_CONFIG, groupId);
        // 값 문자열 역직렬화
        JsonDeserializer<MessageResponse> deserializer = new JsonDeserializer<>(MessageResponse.class);
        deserializer.addTrustedPackages("com.example.stomp");
        deserializer.setTypeMapper(new DefaultJackson2JavaTypeMapper());

        return new DefaultKafkaConsumerFactory<>(
                properties,
                new StringDeserializer(),
                deserializer
        );
    }

    @Bean
    public ConcurrentKafkaListenerContainerFactory<String, MessageResponse> chatListenerContainerFactory() {
        ConcurrentKafkaListenerContainerFactory<String, MessageResponse> factory =
                new ConcurrentKafkaListenerContainerFactory<>();
        factory.setConsumerFactory(consumerFactory());

        // 에러 핸들러 설정
        DefaultErrorHandler errorHandler = new DefaultErrorHandler(
                (record, exception) -> {
		                // 예외 처리
                },
                new FixedBackOff(1000L, 3L) // 3회 재시도
        );
        factory.setCommonErrorHandler(errorHandler);

        return factory;
    }
}
```

`kafka.consumer.group-id`는 구독한 토픽에 발행된 메시지를 가져갈 때 활용하는데, 서버 인스턴스가 모두 같은 그룹 ID를 사용하면 그 중 하나의 서버 인스턴스에서만 메시지를 처리하면 다른 서버 인스턴스에서는 메시지를 처리하지 않기 때문에 환경 변수로 각기 다른 그룹 ID를 가지도록 하였다.

### 서비스

```java
@Service
@Transactional
@RequiredArgsConstructor
public class ChatService {
    private final ChatLogRepository chatLogRepository;
    private final ChatroomUserRepository chatroomUserRepository;
    private final SimpMessageSendingOperations operations;
    private final KafkaTemplate<String, MessageResponse> kafkaTemplate;

    public void saveMessage(MessageSendingDto dto) {
        ChatLog savedChatLog = saveChatLog(dto);
        Long unreadCount = chatroomUserRepository.countByChatroomId(savedChatLog.getChatroomId());
        kafkaTemplate.send(
                "chat",
                MessageResponse.of(
                        savedChatLog.getId(),
                        savedChatLog.getChatroomId(),
                        savedChatLog.getUserId(),
                        savedChatLog.getMessage(),
                        unreadCount - 1,
                        savedChatLog.getCreatedAt()
                )
        );
    }

    @KafkaListener(
            topics = "chat",
            containerFactory = "chatListenerContainerFactory")
    public void listen(MessageResponse message) {
        operations.convertAndSend(
                "/topic/chat." + message.chatroomId(),
                message
        );
    }
    // ...
}
```

서비스에서는 `KafkaTemplate`로 토픽에 메시지를 발행하고, `@KafkaListener`를 통해 구독한 토픽으로부터 메시지를 받아서 메시지를 브로드캐스트하면 된다.

## 테스트

3개의 외부 브로커를 활용하는 방법을 각각 알아봤다. 이제 테스트를 진행해보자.

```
# ws://localhost:54698/ws

>>> SEND
destination:/app/chat.1
content-length:36
{"userId": "2", "message": "Hello?"}
```

```
# ws://localhost:54697/ws
>>> SUBSCRIBE
ack:auto
id:sub-0
destination:/topic/chat.1

Received data
<<< MESSAGE
content-length:112
message-id:708a2237-9ec1-05b2-3d77-ffc670c03bed-0
subscription:sub-0
content-type:application/json
destination:/topic/chat.1
content-length:112
{"id":152,"chatroomId":1,"userId":2,"message":"Hello?","unreadCount":2,"createdAt":"2025-08-29T22:35:11.163141"}
```

이렇게 다른 서버 인스턴스에서도 정상적으로 메시지를 수신하는 것을 확인할 수 있다.

## 각 외부 브로커 비교

Redis, RabbitMQ, 그리고 Kafka를 사용했을 때의 장단점을 간단하게 표로 알아보도록 하자.

| 기술          | 장점                                                                                     | 단점/주의점                                                                                                               |
| ------------- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------- |
| Redis Pub/Sub | 아주 단순하고 지연이 매우 낮고 쉬운 운영                                                 | 영속성 없고 구독 순간에만 전달(오프라인 소비자에게 유실), 재처리/리플레이 불가, 스케일아웃은 샤딩·패턴 분리의 난해함 존재 |
| RabbitMQ      | 다양한 라우팅과 패턴, DLQ/TTL/우선순위 큐, at-least-once                                 | 클러스터링/고가용성 튜닝 필요, Kafka보다 일반적으로 낮은 처리량                                                           |
| Kafka         | 매우 높은 처리량·내구성·리플레이, 파티션 기반 병렬처리, 대규모 로그 수집/스트리밍에 최적 | 운영 복잡도·러닝 커브 높음, 스키마/토픽/파티션 설계 필요, Redis에 비하면 높은 지연 시간                                   |

겉보기에는 Kafka를 몇 줄 코드로 적용하는 게 무척 쉬워 보인다. 하지만 실제 서비스 수준에서 안정적으로 운영하려면 토픽 파티션, 리밸런싱, 메시지 내구성 같은 구조적 요소들을 설계해야 하고, 이 과정에서 생각보다 큰 러닝 커브가 존재한다고 생각한다. 나도 토픽으로 발행 및 구독을 처리하는 부분밖에 모르기에 Kafka에 대해서 공부를 진행해야 한다.

표를 기반으로 정리하자면 작고 간단한 서비스라면 Redis, 라우팅이 복잡하면 다양한 라우팅 패턴을 지원하는 RabbitMQ, 대형 트래픽과 로그성 메시지에는 Kafka를 활용하면 될 것 같다.

---

Spring STOMP를 활용하여 채팅 애플리케이션 개발을 해보았다. 메시지 브로커를 다룰 때 RabbitMQ와 Kafka의 차이점을 자세하게 다루고 싶었는데, 포스팅의 길이가 너무 길어질 것 같기도 하고 결정적으로 둘다 활용만 어느정도 할 줄 알지 내부 아키텍처나 원리 등은 나도 잘 모르기 때문에 제외했다. 둘 모두 공부하여 언젠가 한 번 다루고자 한다.

또한 6번째 포스팅을 고민중인데, 바로 소켓의 구체적인 원리와 순수 자바 코드로 채팅 애플리케이션을 간단하게 구현하는 방법을 다룰까 생각중이다. 다만 이 부분도 학습이 필요하기 때문에 바로 포스팅을 할 수 있을지는 모르겠다.

# **참고 자료**

[**26. WebSocket Support - External Broker**](https://docs.spring.io/spring-framework/docs/4.3.x/spring-framework-reference/html/websocket.html?utm_source=chatgpt.com#websocket-stomp-handle-broker-relay)

[**[Spring] 실시간 채팅 서버 구현 (STOMP, Redis Pub/Sub), 상세 시나리오**](https://seo92js.tistory.com/41)

[**[Spring Boot] WebSocket & Stomp & Redis pub/sub & FCM으로 개발하는 채팅기능**](https://modutaxi-tech.tistory.com/6)

[**Spring boot에서 RabbitMQ를 사용하는 방법 (STOMP, AMQP)**](https://wooing1084.tistory.com/40)
