---
title: 서블릿 애플리케이션 vs WebFlux - 15,000명 동시 접속 부하 시나리오 테스트

categories:
  - Spring WebFlux
  - Reactive Programming

toc: true
toc_sticky: true
published: true

date: 2025-11-24
last_modified_at: 2025-11-24
---

Spring WebFlux의 장점인 안정성을 확인해보기 위해서 데이터베이스에 INSERT 작업을 하는 엔드포인트를 하나 만들어 K6로 부하 테스트를 진행해봤다.

# 기능 구현

우선 이를 위해서 간단하게 게시글을 작성하는 기능을 구현해보자. 요구사항은 다음과 같다.

> 사용자가 요청한 게시글 작성이 성공하면 `Location` 헤더에 작성된 게시글의 번호와 함께 201 응답을 반환한다.

## 사전 설정

애플리케이션에서 공통으로 공유하는 것들은 다음과 같다.

**데이터베이스**

```bash
docker run --name mysql-db -e MYSQL_ROOT_PASSWORD=1234 -d -p 3306:3306 mysql:latest
```

간단하게 username은 root, password는 1234로 설정된 도커 컨테이너를 하나 생성했다.

**데이터베이스 스키마**

```sql
CREATE DATABASE IF NOT EXISTS board;

USE board;

CREATE TABLE IF NOT EXISTS post
(
    `no`         INTEGER      NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `title`      VARCHAR(100) NULL,
    `content`    TEXT         NULL,
    `created_at` TIMESTAMP    NULL DEFAULT CURRENT_TIMESTAMP
);
```

**엔티티**

```java
@Table(name = "post")
@NoArgsConstructor(access = AccessLevel.PROTECTED)
public class Post {
    @Id
    public Integer no = null;

    public String title;
    public String content;

    @CreatedDate
    public LocalDateTime createdAt;

    public Post(String title, String content) {
        this.title = title;
        this.content = content;
    }
}
```

**DTO**

```java
public record PostWriteRequest(
        String title,
        String content
) {
    public Post toEntity() {
        return new Post(title, content);
    }
}
```

## 서블릿 + JPA

### 애플리케이션 설정

**데이터베이스 연결 설정**

```yaml
spring:
  datasource:
    url: jdbc:mysql://localhost:3306/stream_board?serverZoneId=Asia/Seoul
    username: root
    password: 1234
```

**JPA 설정**

JPA를 활용하기 위해서 엔티티에 다음과 같이 어노테이션을 추가하도록 하자.

```java
@Entity
@Table(name = "post")
@NoArgsConstructor(access = AccessLevel.PROTECTED)
@EntityListeners(AuditingEntityListener.class)
public class Post {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    public Integer no = null;
    // ...
}
```

그리고 `createdAt` 필드는 스키마에 따르면 데이터베이스가 기본값을 생성한다. 따라서 이런 기본값을 활용하기 위해서 JpaAuditing 기능을 활성화시키는 설정 클래스를 만든다.

```java
@Configuration
@EnableJpaAuditing
public class JpaAuditingConfiguration {}
```

### 코드 작성

**레포지토리**

```java
@Repository
public interface PostRepository extends JpaRepository<Post, Integer> {
}
```

**서비스**

```java
@Service
@Transactional
@RequiredArgsConstructor
public class PostService {

    private final PostRepository postRepository;

    public Integer createPost(PostWriteRequest request) {
        return postRepository.save(request.toEntity()).no;
    }
}
```

**컨트롤러**

```java
@RestController
@RequestMapping("/post")
@RequiredArgsConstructor
public class PostController {
    private static final String POST_RETRIEVE_URI = "/post/%s";

    private final PostService postService;

    @PostMapping
    public ResponseEntity<Void> createPost(
            @RequestBody PostWriteRequest request
    ) {
        Integer no = postService.createPost(request);

        return ResponseEntity
                .created(URI.create(String.format(POST_RETRIEVE_URI, no)))
                .build();
    }
}
```

## Spring WebFlux + R2DBC

여기서도 데이터베이스의 기본값 생성 기능을 활용하기 위해서 `R2dbcAuditing`를 활성화시키는 설정 클래스를 만든다.

### 애플리케이션 설정

**데이터베이스 연결 설정**

```yaml
spring:
  r2dbc:
    url: r2dbc:mysql://localhost:3306/stream_board?serverZoneId=Asia/Seoul
    username: root
    password: 1234
```

**R2DBC**

```java
@Configuration
@EnableR2dbcAuditing
public class R2dbcAuditingConfiguration {
}
```

### 코드 작성

**레포지토리**

```java
public interface PostRepository extends ReactiveCrudRepository<Post, Integer> {
}
```

**서비스**

```java
@Service
@Transactional
@RequiredArgsConstructor
public class PostService {

		private final PostRepository postRepository;

    public Mono<Integer> createPost(PostWriteRequest request) {
        return postRepository.save(request.toEntity())
                .map(Post::getNo);
    }
}
```

**컨트롤러**

```java
@Slf4j
@RestController
@RequestMapping("/post")
@RequiredArgsConstructor
public class PostController {
    private static final String POST_RETRIEVE_URI = "/post/%s";

    private final PostService postService;
    private final MessageSource messageSource;

    @PostMapping
    public Mono<ResponseEntity<JsonResponse<Void>>> createPost(
            @RequestBody PostWriteRequest request
    ) {
        return postService.createPost(request)
                .map(no -> ResponseEntity
                        .created(
                                URI.create(String.format(POST_RETRIEVE_URI, no))
                        )
                        .build());
    }
}
```

# 모니터링 설정

Spring WebFlux는 적은 스레드로도 많은 요청을 처리하는 것을 통해서 서버의 가용성을 확보하는 것이 주요 목표라고 하였다. 따라서 이를 확인하기 위해 Prometheus와 Grafana로 모니터링 환경을 구축해보도록 하자.

## docker-compose.yml

```yaml
services:
  grafana:
    container_name: grafana
    image: grafana/grafana:latest
    ports:
      - 3000:3000
    volumes:
      - grafana_data:/var/lib/grafana
    networks:
      - compose-network

  prometheus:
    container_name: prometheus
    image: ubuntu/prometheus:latest
    volumes:
      - ./settings/prometheus/prometheus.yml:/etc/prometheus/prometheus.yml
      - prometheus_data:/opt/bitnami/prometheus/data
    ports:
      - 9090:9090
    networks:
      - compose-network

volumes:
  grafana_data:
  prometheus_data:

networks:
  compose-network:
    name: compose-network
    driver: bridge
```

## prometheus.yml

마운트되는 파일은 다음과 같이 설정되어 있다.

```yaml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

alerting:
  alertmanagers:
    - static_configs:
        - targets:

rule_files:

scrape_configs:
  - job_name: "prometheus"
    static_configs:
      - targets: ["localhost:9090"]

  - job_name: "server"
    scrape_interval: 5s
    metrics_path: "/actuator/prometheus"
    static_configs:
      - targets: ["host.docker.internal:8080"]
```

host.docker.internal은 컨테이너에서 localhost에 접속하기 위한 주소이다. 따라서 저 설정은 곧 localhost:8080/actuator/prometheus라는 엔드포인트에서 매트릭 정보를 가져오게 된다. 원래는 30초 정도로 수집하지만 이번에는 좀 더 변화의 추이를 확실하게 확인하기 위해서 5초 주기로 서버로부터 매트릭 정보를 수집하도록 설정했다.

Grafana에서 대시보드는 간단하게 ID값이 4701인 대시보드를 import해서 사용하였다.

## 애플리케이션 설정

이제 두 애플리케이션에 각각 다음 의존성과 설정을 추가하였다.

**build.gradle**

```groovy
implementation 'org.springframework.boot:spring-boot-starter-actuator'
implementation 'io.micrometer:micrometer-registry-prometheus:1.16.0'
```

**application.yml**

```yaml
management:
  endpoints:
    web:
      exposure:
        include: "*"
```

# 부하 테스트 진행

부하 테스트를 위해서 스크립트를 작성해보았다.

```jsx
import http from "k6/http";
import { check } from "k6";

const vus_count = __ENV.VUS ? parseInt(__ENV.VUS) : 3000;

export let options = {
  scenarios: {
    constant_vus_test: {
      executor: "constant-vus",
      vus: vus_count,
      duration: "60s",
    },
  },
};

export default function () {
  const payload = JSON.stringify({
    title: "test-title",
    content: "test-content",
  });

  const params = {
    headers: {
      "Content-Type": "application/json",
    },
  };

  // 1000명의 사용자가 각자 최대한 빠르게 반복해서 요청을 보냅니다.
  let response = http.post("http://localhost:8080/post", payload, params);

  check(response, {
    "status is 201": (r) => r.status === 201,
  });
}
```

부하 테스트는 N명의 동시 접속자가 60초 동안 0초의 간격으로 지속적인 쓰기(POST) 요청을 보내는 시나리오라고 가정했다. 각각 3000명, 5000명, 10000명, 15000명의 동시 접속자를 가정하고 테스트를 진행해봤다. 테스트는 두 애플리케이션 모두 다음 환경에서 진행되었다.

- 맥북 에어 m4
- 크롬 (Notion, Gemini) 실행 중
- IntelliJ
- 10개의 DB 커넥션 풀

부하 테스트 결과는 Gemini를 이용해서 요약해봤다.

## 서블릿 + JPA

**3000명**

| 지표 (Metrics)     | 값 (Value) |
| ------------------ | ---------- |
| 총 요청 수         | 207,500건  |
| RPS (초당 요청 수) | 3,413.1    |
| Error Rate         | 0.00%      |
| Avg Latency        | 871.38ms   |
| P95 Latency        | 1,037.48ms |

여기서 P95 Latency는 전체 요청 중 95%가 해당 시간 이하로 응답된다는 의미이다. 대다수 사용자의 실제 경험을 평가하는데 자주 활용된다고 한다.

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/01.png)

성능 지표는 어떨까? 동시 요청이 몰리자마자 현재 스레드 풀의 최대 개수인 200개까지 포화 상태가 되었다. 그리고 스레드 상태를 보면 작업중인 스레드가 데이터베이스 쓰기 요청인 I/O 작업을 기다리는 timed-waiting 상태에 머물러 있다.

이를 바로 Thread Pool Hell 현상이라고 한다. 하나의 요청에 하나의 스레드가 대응되기 때문에 I/O 요청에 대한 블로킹으로 스레드 풀 크기를 넘어서는 요청 부터는 대기 큐에서 스레드가 할당될 때 까지 기다려야 하는 것이다. 이 현상이 심각해지면 다음과 같은 시스템 붕괴를 일으킨다.

- 처리 시간 증가
- 문맥 교환(Context Switching) 오버헤드 과부화
- 메모리 부족
- 사용자의 지속적인 재시도로 발생하는 악순환 (서비스가 응답이 없으면 새로고침을 하는 것)

실제로도 그런지 점점 부하 강도를 높여가면서 확인해봤다.

**5000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 212,562건  |
| RPS            | 3,465.2    |
| Error Rate     | 0.00%      |
| Avg Latency    | 1,423.38ms |
| P95 Latency    | 1,639.08ms |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/02.png)

3000명 보다는 느려졌지만 1,639.08ms의 P95를 가지기 때문에 여전히 안정적인 RPS와 처리 시간을 보여주는 것으로 볼 수 있을 것 같다.

**10000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 215,850건  |
| RPS            | 3,445.2    |
| Error Rate     | 6.38%      |
| Avg Latency    | 2,334.44ms |
| P95 Latency    | 2,783.33ms |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/03.png)

여기서부터 본격적으로 에러가 발생하면서 가용성이 훼손되기 시작했다. P95가 길어진 것은 물론이고 전체 215,850건의 요청 중에서 약 13,771 건의 요청이 실패하게 된 것이다. 이 때부터 본격적인 Thread Pool Hell에 빠지게 되었다고 볼 수 있다.

**15000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 212,659건  |
| RPS            | 3,342.6    |
| Error Rate     | 25.29%     |
| Avg Latency    | 2,365.36ms |
| P95 Latency    | 3,920.44ms |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/04.png)

여기부터는 전체 212,659의 요청 수에서 약 53781건의 요청이 실패하게 되었다. 이 때부터는 이제 시스템이 제대로 운영되지 않는다고 볼 수 있다.

---

서블릿 애플리케이션과 JPA를 활용한 테스트에서는 10,000건의 부하부터 시스템의 가용성이 훼손되기 시작하는 것을 볼 수 있다. 이는 아파치 웹 서버의 한계인 C10K 문제와 일맥상통한 것으로 애플리케이션의 매 요청마다 하나의 스레드가 할당되고, I/O 작업이 시작하면 스레드가 블로킹된다. 이 과정이 반복되면서 더 이상 요청에 대해 할당할 스레드가 없으면 대기하다가 타임아웃이 발생하게 되는 것이다.

## Spring WebFlux + R2DBC

**3,000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 209,625건  |
| RPS            | 3,448.4    |
| Error Rate     | 0.00%      |
| Avg Latency    | 862.54ms   |
| P95 Latency    | 968.46ms   |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/05.png)

성능 지표를 통해 확인할 수 있는 가장 큰 차이점은 바로 스레드의 개수이다. 3,000명의 동시 접속 상황에서도 최대 35개의 스레드만 활성화된 것을 볼 수 있다.

**5,000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 207,172건  |
| RPS            | 3,378.6    |
| Error Rate     | 0.00%      |
| Avg Latency    | 1,460.55ms |
| P95 Latency    | 1,612.56ms |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/06.png)

3,000명과 5,000명은 비슷한 요청 수와 RPS를 보여준다. 그렇다면 기존의 블로킹 애플리케이션의 한계인 10,000명 규모의 부하 테스트는 어떨까?

**10,000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 204,568건  |
| RPS            | 3,250.6    |
| Error Rate     | 0.00%      |
| Avg Latency    | 2,988.99ms |
| P95 Latency    | 3,321.47ms |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/07.png)

여기서 서블릿 애플리케이션과의 차이점이 확연하게 드러난다. 204,568 건의 요청을 모두 성공적으로 처리한 것을 볼 수 있다. 그리고 요청이 아무리 증가해도 최대 스레드는 35개 이하로 유지되는 것을 볼 수 있다.

**15,000명**

| 지표 (Metrics) | 값 (Value) |
| -------------- | ---------- |
| 총 요청 수     | 174,030건  |
| RPS            | 2,658.5    |
| Error Rate     | 0.00%      |
| Avg Latency    | 5,327.36ms |
| P95 Latency    | 6,060.17ms |

![image.png](/assets/images/development/spring/25-11-24-load-test-servlet-vs-spring-webflux/08.png)

15,000명 까지도 0퍼센트의 오류율을 기록했다! 물론 P95가 6,060.17ms이므로 상당히 느린 편이지만 핵심은 블로킹 애플리케이션이 요청 자체가 실패한다면 WebFlux는 모든 요청을 오류 없이 처리할 수 있다는 것이다.

참고로 MVC의 총 요청 수가 더 많아 보이는 것은 실패가 빠르게 반환되면서 테스트 도구가 더 많은 재요청을 발생시킨 영향으로, 실제 성공 요청 기준으로는 WebFlux가 더 많은 요청을 안정적으로 수용했다는 것에 초점을 맞추면 될 것 같다.

## 정리

부하 테스트를 통해 서블릿 애플리케이션과 Spring WebFlux의 차이를 확인할 수 있었다.

서블릿 애플리케이션은 약 5,000명까지는 안정적인 RPS를 유지했지만, 동시 요청이 10,000명을 넘어가자 스레드 풀 포화로 인한 Thread Pool Hell 현상이 본격적으로 나타났다. 그 결과 처리 시간이 급격히 증가하고 타임아웃이 발생하면서 요청 실패가 눈에 띄게 증가했다.

반면 Spring WebFlux는 부하가 커질수록 평균 응답 시간과 P95는 증가했지만, 단 35개의 스레드만으로도 15,000명 규모의 트래픽을 Error Rate 0%로 처리했다. 스레드 고갈이 발생하지 않는 논블로킹 구조 덕분에 높은 동시성 환경에서도 안정적으로 시스템을 유지할 수 있었다.

---

테스트의 결과를 통해 WebFlux가 서블릿 애플리케이션 대비 서버의 안정성을 확보할 수 있다는 것을 확인하였다. 하지만 무조건 WebFlux로 전환하는 것이 답이라고는 생각하지 않는다. 우선 현재 테스트는 N명의 사용자가 0초의 어떠한 기다림도 없이 꾸준히 쓰기 요청을 보낸다고 가정한 시나리오다.

즉, 사용자 개개인의 입력 처리나 클릭 시간 등의 생각 시간(Think Time)을 0초로 설정했기 때문에 같은 사용자 규모라도 K6를 통한 테스트가 좀 더 가혹한 환경이라고 생각한다. 또한 두 애플리케이션 모두 스레드 풀 개수나 데이터베이스 커넥션풀의 개수를 조정하지 않은 기본 설정 상태의 테스트이다. 따라서 이를 적절하게 설정한다면 기존의 서블릿 애플리케이션으로도 더 많은 가용성을 확보할 수 있다고 생각한다.

물론 WebFlux도 마찬가지로 튜닝을 통해 더 많은 부하를 견딜 수 있을 것이다. 하지만 WebFlux의 디버깅의 난해함과 러닝 커브를 고려했을 때 마냥 전체 서비스를 WebFlux로 전환한다는 것은 현실성이 떨어진다고 생각한다. 개인적으로 한 순간의 트래픽 폭주가 예상되는 선착순 시스템이나 스트리밍, 웹 게임 등의 도메인에서는 WebFlux를 도입하고, 나머지 API는 기존의 서블릿 애플리케이션으로 개발한 다음 문제가 생기면 적절한 성능 튜닝을 하는 게 괜찮을 것 같다.

# 참고 자료

[**“Thread Pool Hell” 발생원인**](https://k0zdevel.tistory.com/128)

[**Nginx**](https://velog.io/@gkrdh99/nginx)
