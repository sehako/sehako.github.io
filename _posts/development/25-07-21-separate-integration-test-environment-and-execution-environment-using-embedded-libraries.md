---
title: 인메모리 라이브러리를 활용한 통합 테스트 환경과 실행 환경의 분리

categories:
  - Spring

toc: true
toc_sticky: true
published: true
 
date: 2025-07-21
last_modified_at: 2025-07-21
---

프로젝트 진행 도중에 테스트 관련 강의를 모두 수강하였기 때문에 테스트 코드를 작성하고자 하였었다. 이때 나는 애플리케이션에서 요구하는 인프라 요소들을 실제 배포된 주소나 도커를 사용하여 통합 테스트를 진행하였다.

하지만 이런 방법을 사용하게 되면 실제 배포된 곳의 데이터베이스 자원을 사용하게 되고, 또한 데이터베이스 접근 방법이 달라지면 전체 테스트가 실패할 경우가 생기거나 개발자 개인의 로컬 환경에서는 잘 되지만 다른 환경에 애플리케이션이 사용하는 인프라가 로컬 환경으로 구축되지 않은 경우에는 테스트가 실패하게 되는 경우가 발생하였다.

이게 문제되는 이유는 CI/CD 진행 시 애플리케이션 테스트 후 빌드를 진행하는 build 명령어 대신에 bootJar를 통해 테스트를 생략하고 빌드를 진행하는 명령어가 강제되기 때문이다. 이러면 테스트가 실패해도 알 수가 없고, 결과적으로 코드의 품질이 저하되는 요소가 될 가능성이 있다.

# 테스트와 실행 환경을 분리

따라서 테스트 코드를 구동시키는 부분은 빌드를 수행하는 시스템의 환경에 구애받지 않아야 한다. 이를 위해서 스프링 진영은 다양한 방법을 제공한다. 그 중에서 가장 간단한 것은 `@MockitoBean`을 활용하여 인프라 요소를 사용하는 부분은 성공적으로 완료되었다고 가정하고 테스트를 진행하면 될 것이다.

하지만 유닛 테스트에서는 의미가 있을지 몰라도, 전체 애플리케이션의 구현을 검증하는 통합 테스트에서는 이러한 Mockito가 무의미할 수 있다. 통합 테스트는 비즈니스 로직뿐만 아니라 애플리케이션 인프라 관련 코드가 정상적으로 동작하는지도 검증하기 위한 작업이기 때문이다.

이제 환경을 분리하는 작업을 수행해보도록 하자. 참고로 개발 환경은 Java 17, SpringBoot 3.5.3, Gradle 환경에서 진행하였다.

애플리케이션 개발을 위해서 RDBMS, Redis, MongoDB, Kafka를 활용한다고 가정하고 초기 라이브러리는 다음과 같이 설정하였다.

```groovy
dependencies {
	implementation 'org.springframework.boot:spring-boot-starter-web'
	implementation 'org.springframework.boot:spring-boot-starter-data-jpa'
	compileOnly 'org.projectlombok:lombok'
	annotationProcessor 'org.projectlombok:lombok'
	testImplementation 'org.springframework.boot:spring-boot-starter-test'
	testRuntimeOnly 'org.junit.platform:junit-platform-launcher'
}

```

여기에 필요한 라이브러리를 하나하나 더해나가는 식으로 진행할 것이다. 관계형 데이터베이스는 인메모리 DB로 활용할 수 있는 유명한 H2를 사용하고, 임베디드 버전으로 지원되는 Redis, MongoDB, Kafka를 사용할 것이다. 하나씩 차례대로 테스트 환경을 구축해보도록 하겠다.

# H2

H2를 테스트 목적의 데이터베이스로 활용하여 테스트를 진행해보도록 하자. 다음과 같은 H2 라이브러리를 사용하도록 하자.

```groovy
testImplementation 'com.h2database:h2'

```

## 코드 작성

**테스트용 설정 파일 작성**

그리고 테스트를 진행할 때 test라는 프로필을 활용하여 테스트를 진행할 것이다. 따라서 application-test.yml이라는 파일을 하나 생성하고, 다음과 같이 작성하였다.

```yaml
spring:
  application:
    name: test

  datasource:
    url: jdbc:h2:mem:testdb;DB_CLOSE_DELAY=-1;MODE=MySQL
    driver-class-name: org.h2.Driver
    username: sa
    password:

  jpa:
    database-platform: org.hibernate.dialect.H2Dialect
    hibernate:
      ddl-auto: create-drop

```

**비즈니스 로직 작성**

테스트를 위해 다음과 같은 엔티티와 레포지토리를 만들었다.

```groovy
@Entity
@Getter
@NoArgsConstructor(access = AccessLevel.PROTECTED)
public class Member {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;

    private String username;
    private String password;

    private String nickname;

    @Builder
    public Member(String username, String password, String nickname) {
        this.username = username;
        this.password = password;
        this.nickname = nickname;
    }
}

```

```groovy
@Repository
public interface MemberRepository extends JpaRepository<Member, Integer> {
}

```

## **테스트 코드 작성**

이제 테스트 코드를 작성해서 제대로 테스트가 이루어지는지 살펴보도록 하자.

```groovy
@SpringBootTest
@Transactional
@ActiveProfiles("test")
class MemberRepositoryTest {
    @Autowired
    private MemberRepository memberRepository;

    @Test
    @DisplayName("사용자가 회원가입을 요청하면 사용자 정보를 저장한다.")
    public void memberSaveTest() {
        // given
        Member newMember = createMember();
        Member savedMember = memberRepository.save(newMember);

        // when
        Member findMember = memberRepository.findById(savedMember.getId())
                .orElseThrow(() -> new AssertionError("저장된 멤버를 찾을 수 없습니다."));

        // then
        assertThat(findMember.getUsername()).isEqualTo("test");
        assertThat(findMember.getNickname()).isEqualTo("tester");

    }

    private Member createMember() {
        return Member.builder()
                .username("test")
                .password("test-password")
                .nickname("tester")
                .build();
    }

}

```

테스트를 진행하였을 때 제대로 통과되는 것을 확인하였다. 이제 RDBMS를 사용하는 코드에 대한 테스트는 H2를 활용하여 실행 환경과는 관계 없이 별도로 동작하게 만들었다. 사용하는 데이터베이스에 맞게 테스트 설정 파일에 H2의 모드만 설정해주면된다.

# Redis

레디스를 테스트하기 위해서 다음 라이브러리를 불러오도록 하자.

```groovy
implementation 'org.springframework.boot:spring-boot-starter-data-redis'
testImplementation 'com.github.codemonstur:embedded-redis:1.4.3'

```

참고로 codemonstur의 라이브러리가 임베디드 레디스 라이브러리 중에서 가장 유지보수가 자주 되고 있는 라이브러리이라고 한다. 25년도 7월 5일 기준으로 24년도 3월 24일에 최신 버전이 릴리즈된 것을 확인하였다.

나는 해당 라이브러리 깃허브 및 이 라이브러리를 활용하여 테스트 환경을 구축한 블로그를 참고하여 레디스에 대한 테스트 환경을 구축하였다.

## 코드 작성

**설정 파일에 레디스 접속 정보 추가**

application-test.yml 파일에 다음과 같은 설정 정보를 추가하였다.

```java
spring:
  data:
    redis:
      host: localhost
      port: 6379

```

이후 각 빈의 생명주기에 따라서 start()와 stop()을 선언하기만 하면 된다. 이때 앞서 작성한 포트 번호와 여기서 생성할 때 전달하는 숫자가 같아야 테스트가 성공적으로 진행된다.

지금은 테스트 환경 분리라는 목적만을 위해 가볍게 작성했지만 클러스터 구성이나 내 로컬에 설치되어 있는 레디스를 활용할 수도 있는 등, 다양한 설정을 제공한다고 하니 깃허브를 참고하는 것을 추천한다.

**비즈니스 로직 작성**

레디스를 세션으로 활용하여 사용자 이름을 키로, 사용자 닉네임을 값으로 하여 레디스에 저장한다고 가정하였다.

```java
@Component
@RequiredArgsConstructor
public class SessionManager {
    private final RedisTemplate<String, String> redisTemplate;

    public String getUser(String username) {
        return redisTemplate.opsForValue().get(username);
    }

    public void saveUser(String username, String nickname) {
        redisTemplate.opsForValue().set(username, nickname);
    }
}

```

## **테스트 코드 작성**

**임베디드 레디스 서버 설정**

임베디드 레디스를 사용하기 위해서는 테스트 환경에서만 컨텍스트에 로드되도록 하는 @TestConfiguration을 활용하여 다음과 같이 코드를 작성해야 한다. 여기서 중요한 것이 해당 파일의 위치는 src/main이 아닌 src/test라는 것이다. 이 점에 유의하도록 하자.

```java
@TestConfiguration
public class RedisTestConfiguration {
    private final RedisServer redisServer;

    public RedisTestConfiguration() throws IOException {
		    // 임베디드 레디스 인스턴스 생성
		    // 생성자 인자로 포트 번호를 전달할 수 있다.
        this.redisServer = new RedisServer(6379);
    }

    @PostConstruct
    public void postConstruct() throws IOException {
        redisServer.start();
    }

    @PreDestroy
    public void preDestroy() throws IOException {
        redisServer.stop();
    }
}

```

테스트 코드는 다음과 같이 작성하였고, 통과되는 것을 확인하였다.

```java
@ActiveProfiles("test")
@SpringBootTest(classes = { RedisTestConfiguration.class })
class SessionManagerTest {

    @Autowired
    private SessionManager sessionManager;

    @Test
    @DisplayName("사용자가 로그인 하면 세션에 사용자의 사용자 이름과 닉네임이 저장된다.")
    public void sessionTest() {
        // given
        Member loginMember = Member.builder()
                .username("test")
                .nickname("tester")
                .build();

        sessionManager.saveUser(loginMember.getUsername(), loginMember.getNickname());
        // when
        String sessionMember = sessionManager.getUser(loginMember.getUsername());

        // then

        Assertions.assertThat(sessionMember).isEqualTo(loginMember.getNickname());
    }
}

```

한 가지 눈여겨 볼 만한 것은 `@SpringBootTest`의 `classes` 속성에 앞서 작성했던 `RedisTestConfiguration` 클래스를 전달했다는 것이다.

이를 통해 테스트가 동작할 때 해당 설정 빈을 컨텍스트에 등록하게 되고, 결과적으로 내부에서 레디스 서버가 동작하여 다음과 같은 테스트가 진행될 수 있다. 이 설정 클래스를 생략하면 Unable to connect to Redis 오류가 발생하면서 테스트가 실패하게 된다.

# MongoDB

MongoDB 테스트 또한 간단하다. 다음 라이브러리를 활용하여 진행된다.

```groovy
testImplementation 'de.flapdoodle.embed:de.flapdoodle.embed.mongo.spring3x:4.20.0'
implementation 'org.springframework.boot:spring-boot-starter-data-mongodb'

```

## 코드 작성

**비즈니스 로직 작성**

사용자가 게시판에 3개의 글을 작성했다고 가정하고 다음과 같은 도큐먼트를 생성하였다.

```groovy
@Document
@Getter
@NoArgsConstructor(access = AccessLevel.PROTECTED)
public class Article {
    @Id
    private String id;

    private int userId;
    private String title;
    private String content;

    @Builder
    public Article(int userId, String title, String content) {
        this.userId = userId;
        this.title = title;
        this.content = content;
    }
}

```

## **테스트 코드 작성**

테스트 코드 작성을 할 때는 `@SpringBootTest`의 `properties` 속성에다 임베디드 MongoDB의 버전을 명시해줘야 한다.

```groovy
@ActiveProfiles("test")
@SpringBootTest(properties = "de.flapdoodle.mongodb.embedded.version=5.0.5")
class ArticleTest {

    @Autowired
    private MongoTemplate mongoTemplate;

    @Test
    @DisplayName("사용자가 작성한 게시글 리스트를 사용자 번호를 통해 조회 가능하다.")
    public void articleRetrieveTest() {
        // given
        List<Article> articles = List.of(
                makeArticle(1, "A", "B"),
                makeArticle(1, "C", "D"),
                makeArticle(1, "E", "F")
        );

        mongoTemplate.insertAll(articles);

        // when
        List<Article> articlesByFirstUser = mongoTemplate.query(Article.class)
                .matching(Query.query(Criteria.where("userId").is(1))).all();

        Assertions.assertThat(articlesByFirstUser).hasSize(3)
                .extracting("userId", "title", "content")
                .containsExactlyInAnyOrder(
                        Tuple.tuple(1, "A", "B"),
                        Tuple.tuple(1, "C", "D"),
                        Tuple.tuple(1, "E", "F")
                )
        ;
    }

    private Article makeArticle(int userId, String title, String content) {
        return Article.builder()
                .userId(userId)
                .title(title)
                .content(content)
                .build();
    }
}

```

테스트가 정상적으로 통과되었다.

# Kafka

마지막으로 카프카를 분리해보도록 하자. 다음 라이브러리들을 불러올 것이다.

```groovy
implementation 'org.springframework.kafka:spring-kafka'
testImplementation 'org.springframework.kafka:spring-kafka-test'

```

spring-kafka-test는 임베디드 카프카를 지원하는 라이브러리다. 이를 활용하여 테스트 코드를 작성해볼 것이다.

## 코드 작성

**카프카 생산자 / 소비자 설정**

비즈니스 로직을 작성하기 전에 카프카의 생산자와 소비자를 설정하는 설정 클래스를 먼저 작성하도록 하자. 사용자 번호와 메시지를 받는 간단한 DTO가 사용될 것이다.

```java
public record ChatMessage(
        int userId,
        String message
) {
}

```

위 객체를 사용하는 생산자와 소비자를 다음과 같이 설정하였다.

```java
@Configuration
public class KafkaProducerConfig {
    @Value("${spring.kafka.bootstrap-servers}")
    private List<String> bootstrapServers;

    private static final Map<String, Object> KAFKA_PROPERTIES = new HashMap<>();

    @PostConstruct
    public void initKafkaTemplateProperties() {
        KAFKA_PROPERTIES.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, bootstrapServers);
        KAFKA_PROPERTIES.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG, StringSerializer.class);
        KAFKA_PROPERTIES.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, JsonSerializer.class);
    }

    @Bean
    public ProducerFactory<String, ChatMessage> videoEditProducerFactory() {
        return new DefaultKafkaProducerFactory<>(KAFKA_PROPERTIES);
    }

    @Bean
    public KafkaTemplate<String, ChatMessage> videoEditKafkaTemplate() {
        return new KafkaTemplate<>(videoEditProducerFactory());
    }
}

```

```java
@EnableKafka
@Configuration
public class KafkaConsumerConfig {

    @Value("${spring.kafka.bootstrap-servers}")
    private List<String> bootstrapServers;

    @Bean
    public ConsumerFactory<String, ChatMessage> chatMessageConsumerFactory() {
        Map<String, Object> properties = new HashMap<>();
        properties.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, bootstrapServers);
        properties.put(ConsumerConfig.GROUP_ID_CONFIG, "chat-consumer");

        JsonDeserializer<ChatMessage> deserializer = new JsonDeserializer<>(ChatMessage.class);
        deserializer.addTrustedPackages("com.sehako.test");
        deserializer.setTypeMapper(new DefaultJackson2JavaTypeMapper());

        return new DefaultKafkaConsumerFactory<>(
                properties,
                new StringDeserializer(),
                deserializer
        );

    }

    @Bean
    public ConcurrentKafkaListenerContainerFactory<String, ChatMessage> kafkaListenerContainerFactory() {
        ConcurrentKafkaListenerContainerFactory<String, ChatMessage> factory = new ConcurrentKafkaListenerContainerFactory<>();
        factory.setConsumerFactory(chatMessageConsumerFactory());
        return factory;
    }
}

```

**비즈니스 로직 작성**

사용자가 채팅을 보낸다고 가정하고 비즈니스 로직을 작성하였다.

```java
@Component
@RequiredArgsConstructor
public class ChatProducer {
    private final KafkaTemplate<String, ChatMessage> kafkaTemplate;

    public void sendMessage(String topic, ChatMessage chatMessage) {
        kafkaTemplate.send(topic, chatMessage);
    }
}

```

## 테스트 코드 작성

테스트 코드 작성 이전에 application-test.yml파일에 다음 설정을 추가하도록 하자.

```java
kafka:
  topic: chat-test

```

**발행 테스트**

테스트를 위한 컨슈머 설정을 위해서 카프카 테스트를 다룬 공식문서와 GPT의 도움을 받아서 작성하였다.

```java
@ActiveProfiles("test")
@SpringBootTest(properties = "de.flapdoodle.mongodb.embedded.version=5.0.5")
@DirtiesContext
@EmbeddedKafka(topics = "${kafka.topic}", partitions = 1)
class ChatProducerTest {

		@Value("${kafka.topic}")
		private String topic;

    @Autowired
    private ChatProducer chatProducer;

    @Autowired
    private EmbeddedKafkaBroker embeddedKafkaBroker;

    private Consumer<Integer, ChatMessage> consumer;

    @BeforeEach
    void setUp() {
        // 1) 컨슈머 프로퍼티 생성
        Map<String, Object> props = KafkaTestUtils.consumerProps(
                "testGroup",   // 그룹 아이디
                "true",        // autoCommit
                embeddedKafkaBroker
        );
        // 2) JSON 역직렬화를 위해 trusted 패키지 설정
        props.put(JsonDeserializer.TRUSTED_PACKAGES, "*");

        // 3) 컨슈머 팩토리로부터 컨슈머 생성
        DefaultKafkaConsumerFactory<Integer, ChatMessage> cf =
                new DefaultKafkaConsumerFactory<>(
                        props,
                        new IntegerDeserializer(),
                        new JsonDeserializer<>(ChatMessage.class, false)
                );
        consumer = cf.createConsumer();

        // 4) Embedded 토픽에 컨슈머 할당
        consumer.subscribe(Collections.singletonList(topic));
    }

    @AfterEach
    void tearDown() {
        consumer.close();
    }

    @Test
    @DisplayName("사용자가 채팅을 전송하면 다른 사용자들은 해당 채팅을 볼 수 있다.")
    void sendChatTest() {
        // given
        ChatMessage msg = new ChatMessage(1, "Hello, World");

        // when
        chatProducer.sendMessage(topic, msg);

        // then
        // 토픽에서 한 건 읽어오기 (최대 5초 대기)
        ConsumerRecord<Integer, ChatMessage> record =
                KafkaTestUtils.getSingleRecord(consumer, topic);

        assertThat(record.value().userId()).isEqualTo(1);
        assertThat(record.value().message()).isEqualTo("Hello, World");
    }
}

```

`@SpringBootTest`의 속성값은 무시하도록 하자. 이 테스트를 통해 토픽 발행이 제대로 진행되는 것을 확인할 수 있다. 참고로 IntelliJ에서 테스트를 진행할 경우 `EmbeddedKafkaBroker`라는 빈이 없다는 오류가 발생하는데, 그냥 무시해도 상관 없다.

**컨슈머 검증**

만약 토픽에 메시지를 발행하는 것 뿐 아니라 메시지가 컨슈머에 의해 제대로 소비되었는 지 확인하고자 한다면 AOP와 `CountDownLatch` 를 활용하여 다음과 같은 테스트 코드를 작성하면 된다. 다음 비즈니스 로직을 검증한다고 가정할 것이다.

```java
@Slf4j
@Component
public class ChatConsumer {
    @KafkaListener(
            topics = {"${kafka.topic}"},
            groupId = "chat-consumer"
    )
    public void listen(ChatMessage chatMessage) {
        log.info("Receive message: {}", chatMessage);
    }
}

```

해당 컴포넌트에 대한 AOP 설정 코드는 다음과 같다.

```java
@Aspect
public class ListenerInvocationAspect {
    private final CountDownLatch latch = new CountDownLatch(1);

    // ChatConsumer.listen(...) 메소드 실행 직후 advice
    @After("execution(* com.sehako.test.kafka.ChatConsumer.listen(..))")
    public void afterListen(JoinPoint jp) {
        latch.countDown();
    }

    // 테스트에서 호출 여부를 대기하기 위한 헬퍼 메소드
    public boolean await(long timeout, TimeUnit unit) throws InterruptedException {
        return latch.await(timeout, unit);
    }
}

```

```java
@TestConfiguration
@EnableAspectJAutoProxy
public class ChatConsumerTestConfig {
    @Bean
    public ListenerInvocationAspect listenerInvocationAspect() {
        return new ListenerInvocationAspect();
    }
}

```

테스트 코드는 다음과 같다.

```java
@ActiveProfiles("test")
@SpringBootTest(properties = "de.flapdoodle.mongodb.embedded.version=5.0.5")
@DirtiesContext
@EmbeddedKafka(topics = "${kafka.topic}", partitions = 1)
@Import(ChatConsumerTestConfig.class)
class ChatProducerTest {
    @Value("${kafka.topic}")
    private String topic;

    @Autowired
    private ChatProducer chatProducer;

    @Autowired
    private ListenerInvocationAspect aspect;

    @Test
    @DisplayName("사용자가 채팅을 전송하면 다른 사용자들은 해당 채팅을 볼 수 있다.")
    void sendChatTest() throws InterruptedException {
        // given
        ChatMessage msg = new ChatMessage(1, "Hello, World");

        // when
        chatProducer.sendMessage(topic, msg);

        // then
        assertTrue(
                aspect.await(5, TimeUnit.SECONDS),
                "ChatConsumer.listen(...) 메소드가 호출되지 않았습니다."
        );
    }
}

```

AOP를 사용한 이유는 내가 참고한 블로그에서는 컨슈머 코드에 CountDownLatch를 삽입했기 때문이다. 이런 방식은 비즈니스 코드에 테스트가 침범한다고 생각되어서 AOP를 통해서 분리하였다.

# 통합 테스트 환경 통일

카프카 테스트 코드에서 보았겠지만, 임베디드 라이브러리들이 하나 둘 늘어갈 수록 해당 라이브러리 설정이 존재하지 않아서 테스트가 실패하는 경우가 발생하였다.

이를 위해 차라리 통합 테스트 환경을 하나의 추상 클래스로 통일하여 테스트를 진행하여 컨텍스트 초기화 주기를 줄이고, 다른 요소 때문에 테스트가 실패하는 상황을 없애는 것이 낫다고 생각하게 되었다. 따라서 다음과 같은 추상 클래스를 정의하였다.

```java
@ActiveProfiles("test")
@SpringBootTest(
		classes = {TestApplication.class, RedisTestConfiguration.class},
		properties = "de.flapdoodle.mongodb.embedded.version=5.0.5"
)
@EmbeddedKafka(topics = "${kafka.topic}", partitions = 1)
@Import(ChatConsumerTestConfig.class)
@Transactional
public abstract class IntegrationTestEnvironment {
}

```

참고로 `TestApplication.class` 는 현재 이 블로그를 진행하기 위해서 만든 프로젝트의 메인 클래스이다.

```java
@SpringBootApplication
public class TestApplication {

	public static void main(String[] args) {
		SpringApplication.run(TestApplication.class, args);
	}

}

```

이렇게 통합한 환경을 상속받고, 세부 클래스에서 각각 별도로 필요한 설정을 하는 식으로 하면 어떨까 생각한다.

```java
@DirtiesContext
class ChatProducerTest extends IntegrationTestEnvironment {...}

```

---

테스트 환경과 실행 환경을 분리하는 것에 성공하였다. 이제 테스트는 Mockito 및 임베디드 라이브러리를 통해 진행하고, 실제 실행은 로컬이나 서버에 구축된 인프라를 통해 실행되어 성공적으로 애플리케이션 빌드 및 실행을 위한 환경을 마련하였다.

# 참고자료

[**H2 Database 란? 그리고 사용법?**](https://yjkim-dev.tistory.com/3)

[**codemonstur/embedded-redis**](https://github.com/codemonstur/embedded-redis)

[**Embedded Redis Server with Spring Boot Test**](https://www.baeldung.com/spring-embedded-redis)

[**Flapdoodle OSS/de.flapdoodle.embed.mongo.spring**](https://github.com/flapdoodle-oss/de.flapdoodle.embed.mongo.spring/tree/spring-3.x.x?tab=readme-ov-file)

[**Querying Documents**](https://docs.spring.io/spring-data/mongodb/reference/mongodb/template-query-operations.html)

[**Testing Applications**](https://docs.spring.io/spring-kafka/reference/testing.html)

[**Testing Kafka and Spring Boot**](https://www.baeldung.com/spring-boot-kafka-testing)