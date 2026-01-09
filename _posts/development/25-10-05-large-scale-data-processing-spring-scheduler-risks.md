---
title: 대용량 데이터 처리 - Spring Scheduler가 위험한 이유

categories:
  - Spring
  - Spring Batch

toc: true
toc_sticky: true
published: true

date: 2025-10-05
last_modified_at: 2025-10-05
---

올해 초에 약물 복용 관리 및 간병인 애플리케이션을 개발한 적이 있다. 나는 여기서 금일에 복약한 정보를 복약 내역 테이블에 기록하는 작업을 처리하는 기능을 맡았었고, 이를 웹 애플리케이션 프로젝트에 별도 패키지를 만들고, Spring Scheduler를 활용하여 매일 새벽 2시에 해당 데이터를 처리하도록 설계하였다.

당시 개발 일정과 기능의 중요도를 고려해봤을 때 어쩔 수 없는 선택이었다고 생각한다. 특히 처음 접하는 분야인 배포와 CI/CD 환경을 구축하는 업무를 도맡아 처리하였기 때문에 MVP가 아니었던 이 기능은 사실상 구색만 맞추는 정도로 개발할 수 밖에 없었다.

# 데이터베이스 설정

데이터베이스는 MySQL을 도커 환경에서 활용할 것이기 때문에 다음 도커 명령어로 MySQL 컨테이너를 실행하도록 하자.

```bash
docker run --name mysql-batch -e MYSQL_ROOT_PASSWORD=1234 -d -p 3306:3306 mysql:latest

```

스프링 배치로 리팩토링 하기 전에 테이블 구조를 조금 변경할 필요가 있다. 기존의 테이블 구조는 다음과 같았다.

```sql

DROP TABLE IF EXISTS `member`;

CREATE TABLE `member`
(
    `id`          BIGINT       NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `email`       VARCHAR(50)  NULL,
    `password`    VARCHAR(300) NULL,
    `name`        VARCHAR(30)  NULL,
    `nickname`    VARCHAR(30)  NULL,
    `role`        VARCHAR(50)  NULL,
    `gender`      VARCHAR(1)   NULL,
    `phone`       VARCHAR(30)  NULL,
    `created_at`  TIMESTAMP    NULL DEFAULT NOW(),
    `modified_at` TIMESTAMP    NULL DEFAULT NOW(),
    `deleted`     TINYINT      NULL DEFAULT false,
    `oauth`       TINYINT      NULL,
    `birthday`    VARCHAR(10)  NULL,
    `provider`    VARCHAR(50)  NULL
);

DROP TABLE IF EXISTS `information`;

CREATE TABLE `information`
(
    `id`           BIGINT       NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `reader`       BIGINT       NOT NULL,
    `writer`       BIGINT       NOT NULL,
    `hospital`     VARCHAR(100) NULL,
    `start_date`   DATE         NULL,
    `end_date`     DATE         NULL,
    `disease_name` VARCHAR(255) NULL,
    `requested`    TINYINT      NULL,
    `created_at`   TIMESTAMP    NULL DEFAULT NOW(),
    `modified_at`  TIMESTAMP    NULL DEFAULT NOW(),
    `deleted`      TINYINT      NULL DEFAULT false
);

DROP TABLE IF EXISTS `management`;

CREATE TABLE `management`
(
    `id`              BIGINT       NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `information_id`  BIGINT       NULL,
    `medication_name` VARCHAR(255) NULL,
    `period`          INT          NULL,
    `morning`         TINYINT      NULL,
    `lunch`           TINYINT      NULL,
    `dinner`          TINYINT      NULL,
    `sleep`           TINYINT      NULL,
    `morning_taking`  TINYINT      NULL,
    `lunch_taking`    TINYINT      NULL,
    `dinner_taking`   TINYINT      NULL,
    `sleep_taking`    TINYINT      NULL,
    `created_at`      TIMESTAMP    NULL DEFAULT NOW(),
    `modified_at`     TIMESTAMP    NULL DEFAULT NOW(),
    `deleted`         TINYINT      NULL DEFAULT false
);

DROP TABLE IF EXISTS `history`;

CREATE TABLE `history`
(
    `id`             BIGINT    NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `management_id`  BIGINT    NOT NULL,
    `member_id`      BIGINT    NOT NULL,
    `information_id` BIGINT    NOT NULL,
    `period`         INTEGER   NULL,
    `morning`        TINYINT   NULL,
    `lunch`          TINYINT   NULL,
    `dinner`         TINYINT   NULL,
    `sleep`          TINYINT   NULL,
    `morning_taking` TINYINT   NULL,
    `lunch_taking`   TINYINT   NULL,
    `dinner_taking`  TINYINT   NULL,
    `sleep_taking`   TINYINT   NULL,
    `taking_date`    DATE      NULL,
    `created_at`     TIMESTAMP NULL DEFAULT NOW(),
    `modified_at`    TIMESTAMP NULL DEFAULT NOW(),
    `deleted`        TINYINT   NULL DEFAULT false
);

```

간단하게 설명하자면 `information` 테이블이 처방전, `management`가 처방전에 해당하는 약물, `history`가 복약 내역이다. 복약 내역은 주어진 날짜가 `information` 테이블의 `start_date`와 `end_date` 사이에 있는 레코드를 조회하고, 이를 외래키로 하여 `management` 레코드를 조회한 다음에 `hisotry` 테이블에 삽입하는 것이다.

즉, `management` 테이블을 적절하게 수정한다면 해당 테이블과 `history` 테이블만 활용하여 배치 작업을 처리할 수 있다. 따라서 `member`와 `information` 테이블을 없애고 필요한 두 개의 테이블만 사용하기 위해 컬럼을 추가 및 삭제하여 다음과 같이 테이블을 간소화 하였다.

```sql
CREATE DATABASE IF NOT EXISTS batch;

USE batch;

DROP TABLE IF EXISTS `management`;

CREATE TABLE `management`
(
    `id`              BIGINT       NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `member_id`       BIGINT       NULL,
    `medication_name` VARCHAR(255) NULL,
    `morning`         TINYINT      NULL,
    `start_date`      DATE         NULL,
    `end_date`        DATE         NULL,
    `lunch`           TINYINT      NULL,
    `dinner`          TINYINT      NULL,
    `sleep`           TINYINT      NULL,
    `morning_taking`  TINYINT      NULL,
    `lunch_taking`    TINYINT      NULL,
    `dinner_taking`   TINYINT      NULL,
    `sleep_taking`    TINYINT      NULL,
    `created_at`      TIMESTAMP    NULL DEFAULT NOW(),
    `modified_at`     TIMESTAMP    NULL DEFAULT NOW(),
    `deleted`         TINYINT      NULL DEFAULT false
);

DROP TABLE IF EXISTS `history`;

CREATE TABLE `history`
(
    `id`             BIGINT    NOT NULL AUTO_INCREMENT PRIMARY KEY,
    `management_id`  BIGINT    NOT NULL,
    `member_id`      BIGINT    NOT NULL,
    `morning`        TINYINT   NULL,
    `lunch`          TINYINT   NULL,
    `dinner`         TINYINT   NULL,
    `sleep`          TINYINT   NULL,
    `morning_taking` TINYINT   NULL,
    `lunch_taking`   TINYINT   NULL,
    `dinner_taking`  TINYINT   NULL,
    `sleep_taking`   TINYINT   NULL,
    `taking_date`    DATE      NULL,
    `created_at`     TIMESTAMP NULL DEFAULT NOW(),
    `modified_at`    TIMESTAMP NULL DEFAULT NOW(),
    `deleted`        TINYINT   NULL DEFAULT false
);

```

## management 테이블에 무작위 데이터 삽입

100만 건의 데이터를 대상으로 한 번 Spring Scheduler의 문제점을 파악해보도록 하자. 다음 쿼리를 실행하도록 하자.

```sql
CREATE TABLE IF NOT EXISTS digits (d TINYINT UNSIGNED PRIMARY KEY);

INSERT IGNORE INTO digits (d)
VALUES (0),(1),(2),(3),(4),(5),(6),(7),(8),(9);

CREATE OR REPLACE VIEW seq_million AS
SELECT
    (d6.d*100000 + d5.d*10000 + d4.d*1000 + d3.d*100 + d2.d*10 + d1.d) + 1 AS n
FROM digits d1
    CROSS JOIN digits d2
    CROSS JOIN digits d3
    CROSS JOIN digits d4
    CROSS JOIN digits d5
    CROSS JOIN digits d6;

INSERT INTO management
(member_id, medication_name, morning, lunch, dinner, sleep,
 morning_taking, lunch_taking, dinner_taking, sleep_taking,
 start_date, end_date)
SELECT
    FLOOR(1 + (RAND()*100)),
    CONCAT('약물_', FLOOR(1 + RAND()*50)),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    FLOOR(RAND()*2),
    '2025-10-01',
    '2025-10-05'
FROM seq_million
LIMIT 1000000;

```

2025년 10월 1일부터 10월 5일까지의 무작위 약물 데이터를 `management` 테이블에 100만 건 삽입하였다. 여담이지만 AI에 의존하느라 점점 쿼리 실력이 떨어지는 것 같다…

# 애플리케이션 사전 설정

배치 작업을 처리하기 위해서 다음과 같은 설정과 클래스를 활용하였다.

## 의존성 및 설정 정보

**build.gradle**

```groovy
dependencies {
    implementation 'org.springframework.boot:spring-boot-starter-web'
    implementation 'org.springframework.boot:spring-boot-starter-data-jpa'
    compileOnly 'org.projectlombok:lombok'
    runtimeOnly 'com.mysql:mysql-connector-j'
    annotationProcessor 'org.projectlombok:lombok'
    testImplementation 'org.springframework.boot:spring-boot-starter-test'
    testRuntimeOnly 'org.junit.platform:junit-platform-launcher'
}

```

**application.yml**

```yaml
spring:
  application:
    name: spring-batch

  datasource:
    url: jdbc:mysql://localhost:3306/batch
    username: root
    password: 1234
```

## Spring Scheduler 활성화

기존 기능인 Spring Scheduler를 통한 데이터 처리 작업을 먼저 알아보기 위해서 메인 클래스에 `@EnableScheduling`이라는 어노테이션을 명시하였다.

```java
@EnableScheduling
@SpringBootApplication
public class SpringBatchApplication {

    public static void main(String[] args) {
        SpringApplication.run(SpringBatchApplication.class, args);
    }
}
```

## 엔티티 작성

```java
@Entity
@Getter
@Table(name = "history")
@NoArgsConstructor(access = AccessLevel.PROTECTED)
public class History extends BaseEntity {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "management_id")
    private Management management;
    @JoinColumn(name = "member_id")
    private Long memberId;
    @Column(name = "taking_date")
    private LocalDate takingDate;
    @ColumnDefault(value = "false")
    private boolean morning = false;
    @ColumnDefault(value = "false")
    private boolean lunch = false;
    @ColumnDefault(value = "false")
    private boolean dinner = false;
    @ColumnDefault(value = "false")
    private boolean sleep = false;
    @Column(name = "morning_taking")
    @ColumnDefault(value = "false")
    private boolean morningTaking = false;
    @Column(name = "lunch_taking")
    @ColumnDefault(value = "false")
    private boolean lunchTaking = false;
    @Column(name = "dinner_taking")
    @ColumnDefault(value = "false")
    private boolean dinnerTaking = false;
    @Column(name = "sleep_taking")
    @ColumnDefault(value = "false")
    private boolean sleepTaking = false;

    @Builder
    public History(Management management, Long memberId, LocalDate takingDate, boolean morning, boolean lunch,
                   boolean dinner, boolean sleep, boolean morningTaking, boolean lunchTaking, boolean dinnerTaking,
                   boolean sleepTaking) {
        this.management = management;
        this.memberId = memberId;
        this.takingDate = takingDate;
        this.morning = morning;
        this.lunch = lunch;
        this.dinner = dinner;
        this.sleep = sleep;
        this.morningTaking = morningTaking;
        this.lunchTaking = lunchTaking;
        this.dinnerTaking = dinnerTaking;
        this.sleepTaking = sleepTaking;
    }
}
```

```java
@Entity
@Getter
@Table(name = "management")
@NoArgsConstructor(access = AccessLevel.PROTECTED)
public class Management extends BaseEntity {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    @Column(name = "member_id")
    private Long memberId;
    @ColumnDefault(value = "false")
    private boolean morning = false;
    @ColumnDefault(value = "false")
    private boolean lunch = false;
    @ColumnDefault(value = "false")
    private boolean dinner = false;
    @ColumnDefault(value = "false")
    private boolean sleep = false;
    @Column(name = "morning_taking")
    @ColumnDefault(value = "false")
    private boolean morningTaking = false;
    @Column(name = "lunch_taking")
    @ColumnDefault(value = "false")
    private boolean lunchTaking = false;
    @Column(name = "dinner_taking")
    @ColumnDefault(value = "false")
    private boolean dinnerTaking = false;
    @Column(name = "sleep_taking")
    @ColumnDefault(value = "false")
    private boolean sleepTaking = false;
    @Column(name = "start_date")
    private LocalDate startDate;
    @Column(name = "end_date")
    private LocalDate endDate;

    public void resetTakingInformation() {
        this.morningTaking = false;
        this.lunchTaking = false;
        this.dinnerTaking = false;
        this.sleepTaking = false;
    }
}
```

## 레포지토리 작성

```java
@Repository
public interface HistoryRepository extends JpaRepository<History, Long> {
}
```

```java
@Repository
public interface ManagementRepository extends JpaRepository<Management, Long> {
    @Query("SELECT m FROM Management m "
            + "WHERE :date BETWEEN m.startDate AND m.endDate")
    List<Management> findYesterdayManagements(LocalDate date);
}
```

## 성능 모니터링

메모리 사용량과 실행 시간을 간단하게 확인하기 위해서 다음과 같은 클래스를 작성하였다.

```java
import lombok.extern.slf4j.Slf4j;

@Slf4j
public class ExecutionMonitor {

    private static long startTime;

    // 실행 시작 시 호출
    public static void start(String taskName) {
        startTime = System.currentTimeMillis();
        logMemory(taskName);
    }

    // 실행 종료 시 호출
    public static void end(String taskName) {
        long endTime = System.currentTimeMillis();
        long elapsed = endTime - startTime;

        logMemory(taskName);
        log.info("[{}] Elapsed Time: {} ms ({} sec)",
                taskName, elapsed, elapsed / 1000.0);
    }

    // 메모리 사용량 로그
    public static void logMemory(String point) {
        Runtime runtime = Runtime.getRuntime();
        long used = (runtime.totalMemory() - runtime.freeMemory()) / (1024 * 1024);
        long max = runtime.maxMemory() / (1024 * 1024);
        double usage = Math.round(((double) used / max) * 1000) / 1000.0;

        log.info("[{}] Memory Used: {} MB / Max: {} MB ({} %)",
                point, used, max, String.format("%.3f", usage));
    }
}
```

# Spring Scheduler로 배치를 설정하면 안되는 이유

이제 내가 구현했던 과거의 기능이 어떤 치명적인 문제를 가지고 있는 지 살펴보도록 하자. 이를 위해서 `@Scheduled`의 실행 시간을 매 1분마다 실행되도록 설정하고, 내가 구현했던 기능과 유사하게 다음 데이터 처리 코드를 작성하였다.

```java
@Component
@RequiredArgsConstructor
@Transactional
public class HistoryScheduler {
    private final HistoryRepository historyRepository;
    private final ManagementRepository managementService;

    @Scheduled(cron = "0 * * * * *")
    public void creatHistory() {
        ExecutionMonitor.start("스케줄러 시작");
        LocalDate yesterday = LocalDate.now().minusDays(1);
        List<Management> validManagements = managementService.findYesterdayManagements(yesterday);

        List<History> histories = validManagements.stream()
                .filter(management -> !management.isDeleted())
                .map(management -> {
                    History history = History.builder()
                            .management(management)
                            .memberId(management.getMemberId())
                            .morning(management.isMorning())
                            .lunch(management.isLunch())
                            .dinner(management.isDinner())
                            .sleep(management.isSleep())
                            .morningTaking(management.isMorningTaking())
                            .lunchTaking(management.isLunchTaking())
                            .dinnerTaking(management.isDinnerTaking())
                            .sleepTaking(management.isSleepTaking())
                            .takingDate(yesterday)
                            .build();

                    management.resetTakingInformation();
                    return history;
                })
                .toList();

        ExecutionMonitor.logMemory("현재 메모리 사용량");
        historyRepository.saveAll(histories);

        ExecutionMonitor.end("스케줄러 종료");
    }
}
```

우선 코드를 살펴봤을 때 가장 먼저 예측되는 문제는 바로 메모리 비효율이다. 전체 데이터베이스 조회 결과를 하나의 리스트로 조회하여 `History` 엔티티로 변환해야 하기 때문에 굉장히 많은 메모리가 요구되며, 최악의 경우 OOM 문제가 발생할 것이다.

또한 새벽 두 시에 이 작업을 실행시키면 항상 하루 전 날인 것이 강제된다. 이는 작업이 유연하지 못하게 하드코딩된 상태라고 볼 수 있다.

마지막으로 테이블에서 전체 데이터를 조회하고 한 번에 저장하기 때문에 많은 시간이 소요될 것으로 보인다. 그렇다면 실제로 이러한 단점이 존재하는 지 확인해보도록 하자. 다음은 해당 스케줄러 실행 결과다.

```
[스케줄러 시작] Memory Used: 50 MB / Max: 4022 MB (1.243 %)
[현재 메모리 사용량] Memory Used: 1290 MB / Max: 4022 MB (32.073 %)
[스케줄러 종료] Memory Used: 1210 MB / Max: 4022 MB (30.084 %)
[스케줄러 종료] Elapsed Time: 1165251 ms (1165.251 sec)
```

스케줄러 작업 실행 로그를 살펴보면 `management` 테이블에서 전체 엔티티를 조회해서 리스트로 적재했을 때 약 1.3GB 정도의 메모리를 점유하는 것을 확인할 수 있다. 그리고 전체 작업은 약 20분 정도 소요되었다.

## 리팩토링 시도

그렇다면 현재 구조에서 이러한 비효율을 개선해볼 수 있지 않을까? 가장 문제되는 것은 전체 테이블을 조회하는 쿼리다. 그렇다면 단순하게 이 부분을 최대 1000건의 커서 기반 페이지네이션으로 변경해보자.

```java
@Repository
public interface ManagementRepository extends JpaRepository<Management, Long> {
    @Query(value = "SELECT * FROM management m " +
            "WHERE :date BETWEEN m.start_date AND m.end_date " +
            "AND m.id > :id " +
            "ORDER BY m.id " +
            "LIMIT 1000", nativeQuery = true)
    List<Management> findYesterdayManagements(LocalDate date, Long id);
}
```

여기에 맞춰서 데이터 처리 작업도 `while`로 데이터가 빌 때 까지 반복하도록 처리하는 것이다. 그러면 적어도 메모리 최적화는 가능할 것으로 보인다. 이 과정에서 `EntityManager`로 `history` 테이블에 INSERT를 할 때마다 영속성 컨텍스트를 비워 메모리 누적을 방지하도록 해봤다.

```java
@Component
@RequiredArgsConstructor
@Transactional
public class HistoryScheduler {
    private final HistoryRepository historyRepository;
    private final ManagementRepository managementService;
    private final EntityManager entityManager;

    @Scheduled(cron = "0 * * * * *")
    public void creatHistory() {
        ExecutionMonitor.start("스케줄러 시작");
        LocalDate yesterday = LocalDate.now().minusDays(1);

        Long maxId = 0L;
        int count = 0;
        while (true) {
            List<Management> validManagements = managementService.findYesterdayManagements(yesterday, maxId);
            if (validManagements.isEmpty()) {
                break;
            }
            maxId = validManagements.stream()
                    .max(Comparator.comparing(Management::getId))
                    .orElseThrow().getId();

            // ...

            ExecutionMonitor.logMemory("현재 메모리 사용량 [id: " + maxId + ", count: " + count++ + "]");
            historyRepository.saveAll(histories);

            entityManager.flush();
            entityManager.clear();
        }

        ExecutionMonitor.end("스케줄러 종료");
    }
}
```

그리고 **application.yml**에서 다음과 같이 JPA 관련 설정을 작성하자.

```yaml
spring:
  jpa:
    properties:
      hibernate:
        jdbc:
          batch_size: 1000
        order_inserts: true
        order_updates: true
```

이제 애플리케이션을 실행시켜 실제로 메모리 점유를 개선할 수 있는지 확인해보자.

```
[스케줄러 시작] Memory Used: 49 MB / Max: 4022 MB (1.218 %)
[현재 메모리 사용량 [id: 1000, count: 0]] Memory Used: 73 MB / Max: 4022 MB (1.815 %)
...
[현재 메모리 사용량 [id: 501000, count: 500]] Memory Used: 53 MB / Max: 4022 MB (1.318 %)
...
[스케줄러 종료] Memory Used: 52 MB / Max: 4022 MB (1.293 %)
[스케줄러 종료] Elapsed Time: 954783 ms (954.783 sec)
[스케줄러 시작] Memory Used: 52 MB / Max: 4022 MB (1.293 %)
```

100만 건의 데이터를 대상으로 메모리가 100MB 이하로 개선되었고, 시간도 16분 정도로 개선 이전에 비해서 5분 가량 더 빨라진 것을 확인할 수 있다. 하지만 이렇게 하면 과연 끝일까?

만약에 데이터가 1000만 건이라면 어떨까? 이 경우 소요 시간은 1시간 이상 걸릴 수도 있다. 만약에 데이터 처리 도중 예외가 발생한다면? 트랜잭션 정책에 따라서 전체 데이터 처리가 롤백된다. 매 저장마다 커밋 처리를 하더라도, 이렇게 자동화된 작업에서 우리는 해당 작업을 재시작할 때 어떤 데이터부터 처리해야 하는지 알 수 없고, 알려면 일일이 `history` 테이블을 살펴봐야 한다.

만약에 서버 인스턴스를 동시에 3개 실행시키는 스케일 아웃 상황이라면? 각 인스턴스가 현재 작업을 처리중인지 알 수 없기 때문에 300만 건의 데이터가 `history` 테이블에 들어갈 것이다. 만약에… 만약에…

## 단점 정리

정리하자면 Spring Scheduler에서 배치 작업을 처리하려면 비즈니스 로직 뿐 아니라 여러 예외 상황에 따른 처리를 해줘야 한다. 성능 이외의 단점을 정리해보면 다음과 같다.

1. **트랜잭션 범위 문제**

- 클래스 전체에 `@Transactional`이 붙어 있어서 모든 데이터가 한 트랜잭션에 묶임
- 데이터가 수천, 수만 건일 경우 롤백 시 부담이 크고, DB 락 시간이 길어져 다른 트랜잭션에 영향
- 부분 커밋 불가 → 전체 실패 가능성 ↑

1. **장애 추적 어려움**

- `saveAll()` 호출로 한 번에 INSERT → 내부적으로 batch insert가 안 되면 row 단위 실패 원인 추적 어려움 (예: 특정 row에서 `DataIntegrityViolationException` 발생 → 전체 실패)

1. **모니터링/로깅 한계**

- 진행 상황(몇 건 완료/실패)을 알 수 없기 때문에 운영 상황에서 장애 대응이 난해함

1. **스케줄러 중복 실행 위험**

- `@Scheduled`는 락이 없으므로, 이전 실행이 끝나기 전에 다음 실행이 겹칠 수 있음
- 특히 대용량 처리 시 겹치면 동일 데이터 중복 처리, DB Deadlock 발생 위험

1. **확장성 부족**

- 단일 인스턴스에서만 동작 가능 → 여러 서버에서 띄우면 중복 실행 발생
- 분산 환경/클러스터 환경에서는 동기화, 리더 선출, 분산락(ZooKeeper, Redis 등) 필요

1. **에러/재처리 전략 부재**

- 실패 시 재처리할 건만 따로 처리 불가
- 중간 저장이나 skip/retry 정책 없음 → 낮은 안정성

1. **테스트/운영 환경 분리 어려움**

- `@Scheduled` 붙은 메서드는 테스트 자동 실행 문제.
- 운영 배포 시점에 우발적으로 실행될 수 있음 → 데이터 꼬임 가능성 존재

---

이렇게 100만 건의 데이터를 활용하여 Spring Scheduler를 활용한 배치 작업의 치명적인 단점들을 알아보았다. 이러한 단점들로 미루어 봤을 때, Scheduler 기반 배치 작업은 간단한 주기성 작업에서는 쓸 수 있지만, 대용량 데이터 처리나 안정적인 운영에는 치명적인 한계가 있다는 걸 알 수 있다.

그렇다면 이런 문제들을 어떻게 해결할 수 있을까? 위 단점을 한 번에 해결할 수 있을까? 이에 대한 정답은 바로 Spring Batch다. 다음 포스팅 부터는 위에서 발견한 문제들이 Spring Batch의 Job과 Step, 청크 지향 처리, 재시도와 건너뛰기 정책으로 어떻게 해결되는지 단계별로 살펴보도록 하겠다.

# 참고 자료

[**American-Startup/Pillme**](https://github.com/American-Startup/Pillme)
