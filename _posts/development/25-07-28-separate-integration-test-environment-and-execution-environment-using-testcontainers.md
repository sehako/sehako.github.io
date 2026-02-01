---
title: TestContainers를 활용한 통합 테스트 환경과 실행 환경의 분리

categories:
  - Spring
  - Test

toc: true
toc_sticky: true
published: true

date: 2025-07-28
last_modified_at: 2025-07-28
---

# 임베디드 라이브러리를 활용한 통합 테스트의 문제점

앞서 통합 테스트 진행을 위해서 인프라들을 인메모리로 동작하도록 구현한 임베디드 라이브러리들을 활용하였다. 하지만 나는 이렇게 이런 식으로 라이브러리들을 구성하여 통합 테스트를 통과하는 것이 과연 실제 프로덕션 코드를 반영하는 건가에 대한 생각이 들었고, 웹 서핑을 하던 중 TestContainers라는 라이브러리를 알게 되었다.

해당 라이브러리의 공식 페이지에서는 이러한 임베디드 라이브러리들을 사용하는 데에 있어서 다음과 같은 문제점들이 있다고 다룬다.

- 임베디드 라이브러리에서는 프로덕션 코드에서 사용하는 기능이 없을 수 있다.

예) Oracle에서 지원되는 최신 기능을 H2에서는 지원하지 않을 수 있다.

- 임베디드 라이브러리는 피드백 사이클을 지연시킨다.

예) H2를 사용하여 조회하는 기능에 대한 테스트를 통과했는데, 막상 배포해보니 Oracle에서는 해당 쿼리가 예외를 발생시킬 수 있다.

즉, 임베디드 라이브러리를 사용하면 테스트 환경과 운영 환경 간 불일치로 인해 실제 배포 시 예기치 못한 오류가 발생할 위험이 있다.

# TestContainers

그렇다면 TestContainers가 어떠한 기능을 지원하기에 이러한 문제들을 해결할 수 있다고 하는 것일까? 그 이유는 테스트 컨테이너는 통합 테스트를 위해 필요한 인프라들을 도커 컨테이너로 제공하는 라이브러리이기 때문이다.

이 라이브러리를 사용하면 테스트 시작 전에 필요한 컨테이너들을 띄우고, 이러한 컨테이너들을 통해 테스트를 진행하고, 테스트가 종료된 이후에는 컨테이너를 삭제시킨다. 공식문서에 따르면 TestContainers를 활용하여 다음과 같은 이점을 얻을 수 있다.

- 통합 테스트를 위한 별도의 인프라를 프로비저닝 할 필요가 없다.
- CI/CD가 병렬적으로 진행되어도 각각의 파이프라인은 독립적인 서비스로 취급되기에 데이터 충돌 위험이 없다.
- 단위 테스트를 수행하듯 통합 테스트를 수행할 수 있다.
- 모든 테스트가 종료되면 컨테이너들은 자동으로 삭제된다.

참고로 도커 컨테이너를 활용하므로 개발 환경 및 CI/CD 환경에는 모두 도커가 설치되어 있어야 한다. 개발 환경에 Docker Desktop이 설치되어 있다면 이 부분은 염려하지 않아도 된다.

# SpringBoot에서 TestContainers 활용하기

임베디드 라이브러리를 사용했던 포스팅에서는 H2를 활용하여 MySQL에 대한 테스트를 진행하였었다. 이를 변경하기 위해서 MySQL 컨테이너를 활용하여 테스트를 진행하도록 하겠다. 우선 다음 의존성이 필요하다.

```groovy
ext {
    set('testcontainers.version', "1.21.3")
}

dependencies {
		// ...
    // TestContainers
    testImplementation 'org.springframework.boot:spring-boot-testcontainers:3.5.3'
    testImplementation "org.testcontainers:junit-jupiter"
	  // MySQL 컨테이너 객체 활용을 위한 의존성
    testImplementation 'org.testcontainers:mysql'
}
```

여기서 `org.testcontainers:mysql`은 MySQL에 특화된 설정을 할 수 있는 컨테이너 객체를 사용하기 위해서 불러오는 것이다.

이것이 없으면 `GenericContainer<?>`라는 객체로 컨테이너를 만들어서 직접 설정해야 하는 것 같았다. 왠만한 것들은 모두 지원 되니 필요한 것은 Maven Repository에서 찾아보도록 하자.

코드는 임베디드 라이브러리 때 작성했던 비즈니스 코드와 테스트 코드를 그대로 사용할 것이다.

```java
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

```java
@Repository
public interface MemberRepository extends JpaRepository<Member, Integer> {
}
```

## 컨테이너 설정

스프링 부트에서는 인터페이스를 활용하여 필요한 컨테이너들을 정적 불변 객체로 생성하여 이를 `@ImportTestcontainers`로 로드하여 사용할 수 있다. 따라서 인터페이스에 MySQL을 정의하도록 하자.

```java
public interface ContainersForIntegrationTest {
    @Container
    @ServiceConnection
    MySQLContainer<?> mysqlContainer = new MySQLContainer<>("mysql:8.4.5")
            .withUsername("root")
            .withPassword("1234")
            .withDatabaseName("testdb");
}
```

> 컨테이너 설정 메서드에 `withReuse(boolean)` 이 존재하는데, 해당 메서드에 `true`를 전달하게 되면 다음 테스트 실행시에도 컨테이너가 재사용되어 컨테이너 삭제 및 생성에 소모되는 비용을 줄일 수 있다.
>
> 모든 컨테이너가 재사용되도록 만들고자 한다면 **src/test/resources/testcontainers.properties**에 다음과 같이 설정하면 된다.
>
> ```
> testcontainers.reuse.enable=true
> ```
>
> 공식 문서에 따르면 이는 실험적인 기능이므로 이런 게 있다 정도의 참고만 하도록 하자.

`@Container` 어노테이션은 테스트의 시작과 끝에 선언하는 `start()`와 `stop()` 메서드를 자동으로 처리해주는 어노테이션이다.

해당 어노테이션이 선언되어 있지 않다면 테스트를 진행하는 클래스에 `@BeforeAll` 과 `@AfterAll` 을 활용하여 다음과 같이 컨테이너의 시작과 끝을 호출해야 한다.

물론 이를 위해서 선언하는 클래스를 인터페이스가 아니라 테스트를 진행하는 클래스에 컨테이너 객체를 선언해야 할 것이다.

```java
class Example {
    static MySQLContainer<?> mysqlContainer = new MySQLContainer<>("mysql:8.4.5")
            .withUsername("root")
            .withPassword("1234")
            .withDatabaseName("testdb");

    @BeforeAll
    static void beforeAll() {
        mysqlContainer.start();
    }

    @AfterAll
    static void afterAll() {
        mysqlContainer.stop();
    }
}
```

`@ServiceConnection` 어노테이션은 컨테이너의 사전 설정과 연결에 대한 설정을 자동화 해준다. 해당 어노테이션을 명시하지 않는다면 `@DynamicPropertySource` 어노테이션으로 다음과 같은 설정을 개발자가 직접 작성해야 한다.

```java
class Example {
    @Container
    static MySQLContainer<?> mysqlContainer = new MySQLContainer<>("mysql:8.4.5")
            .withUsername("root")
            .withPassword("1234")
            .withDatabaseName("testdb");

    @DynamicPropertySource
    static void configureProperties(DynamicPropertyRegistry registry) {
        registry.add("spring.datasource.url",      mysqlContainer::getJdbcUrl);
        registry.add("spring.datasource.username", mysqlContainer::getUsername);
        registry.add("spring.datasource.password", mysqlContainer::getPassword);
    }
}
```

또는 테스트용으로 사용하는 yml파일에 접속 정보를 설정해야 한다.

```yaml
spring:
  datasource:
    url: jdbc:tc:mysql:8.4.5:///testdb
    username: root
    password: 1234
```

## application-test.yml 설정

접속 정보는 코드를 통해서 설정이 끝났기 때문에 application-test.yml에는 JPA 관련 설정만 해줄 것이다.

```yaml
spring:
  jpa:
    database: mysql
    database-platform: org.hibernate.dialect.MySQL8Dialect
    hibernate:
      ddl-auto: create
    properties:
      hibernate:
        format_sql: true
        dialect: org.hibernate.dialect.MySQL8Dialect
    defer-datasource-initialization: true
    show-sql: true
```

## 컨테이너 로드 및 테스트 진행

인터페이스를 불러오고 간단한 저장 기능을 검증하는 테스트 코드를 작성하였다.

```java
@DataJpaTest
@Transactional
@ActiveProfiles("test")
// 컨테이너 설정 인터페이스 로드
@ImportTestcontainers(ContainersForIntegrationTest.class)
class TestContainersTest {
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

이 테스트를 실행하기 전에는 Docker Desktop이 실행되어 있어야 한다. 테스트를 실행시키고 Docker Desktop을 유심히 살펴보면 testcontainers/ryuk 이미지와 내가 설정했던 MySQL 이미지가 자동으로 로드되어 실행까지 되고, 테스트 진행 후 자동으로 컨테이너가 삭제되는 것을 볼 수 있을 것이다.

## 추상 클래스로 컨테이너 설정하기

Redis, Kafka등 여러 인프라들이 함께 사용되기 시작하면 마찬가지로 테스트에 필요한 시간이 많이 필요해질 것이다. 이를 따로 따로 설정하여 환경이 미묘하게 달라지면 컨텍스트 초기화 횟수는 늘어나게 될 것이고 그에 따라서 테스트를 위한 컨테이너의 생성 및 삭제가 빈번하게 일어나 결과적으로 테스트를 수행하는 데 상당한 시간이 소요될 것이다.

따라서 이를 추상 클래스 하나로 만들어 통합 테스트에 필요한 모든 컨테이너를 로드하고 공통된 컨텍스트에서 사용하도록 유도하면 어떨까 생각하였다.

```java
@DataJpaTest
@ImportTestcontainers(ContainersForIntegrationTest.class)
@ActiveProfiles("testcontainer")
public abstract class IntegrationTestContainers {
}
```

이 추상 클래스가 필요한 테스트 클래스에 `extends` 하였고, MySQL 컨테이너가 정상적으로 실행 및 테스트가 통과되는 것을 확인할 수 있었다.

```java
@Transactional
class TestContainersTest extends IntegrationTestContainers {...}
```

---

TestContainers를 활용하는 방법을 알아보았다. 사실 저번처럼 모든 인프라들을 하나하나 해보면 어떨까 싶었지만 도커 컨테이너라서 몇몇 설정만 잘해주면 실제 테스트 코드는 이전 포스팅과 차이가 없거나 오히려 더 간단하다고 생각하기 때문에 MySQL만 예시로 테스트해보았다.

# 참고자료

[**What is Testcontainers, and why should you use it?**](https://testcontainers.com/guides/introducing-testcontainers/)

[**Getting started with Testcontainers for Java**](https://testcontainers.com/guides/getting-started-with-testcontainers-for-java/)

[**Reusable Containers (Experimental)**](https://java.testcontainers.org/features/reuse/)

[**JDBC support**](https://java.testcontainers.org/modules/databases/jdbc/)

[**Testcontainers**](https://docs.spring.io/spring-boot/reference/testing/testcontainers.html)
