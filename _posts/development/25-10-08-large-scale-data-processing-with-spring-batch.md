---
title: Spring Batch - 기본 개념 정리 및 도입

categories:
  - Spring
  - Spring Batch

toc: true
toc_sticky: true
published: true

date: 2025-10-08
last_modified_at: 2025-10-08
---

앞서 Spring Scheduler를 활용하여 대용량 데이터를 처리하면 어떤 치명적인 문제가 발생하는지 알아보고 해당 문제들을 해결하기 위해서 Spring Batch가 사용된다는 것을 알아보았다. 그렇다면 Spring Batch는 무엇일까?

_참고로 앞서 Spring Scheduler로 구현했던 배치 작업을 **기존 작업**으로 명칭하도록 하겠다._

# Spring Batch

공식 문서에 따르면 스프링 배치는 다음과 같다.

> 스프링 배치는 로깅/추적, 트랜잭션 관리, 작업 처리 통계, 작업 재시작, 건너뛰기, 리소스 관리 등 대용량 레코드 처리에 필수적인 재사용 가능한 기능과, 최적화 및 파티셔닝 기술을 통해 대용량 및 고성능 배치 작업을 지원하는 더욱 진보된 기술 서비스와 기능을 제공한다. 간단한 작업뿐 아니라 복잡한 대용량 배치 작업도 확장성이 뛰어난 방식으로 이 프레임워크를 활용하여 방대한 양의 정보를 처리할 수 있다.

쉽게 말해 스프링 프레임워크 기반으로 만들어진 대용량 데이터 처리 프레임워크다.

## 기본 개념

먼저 기존 작업을 Spring Batch로 전환하기 위해 필요한 기본적인 개념들을 알아둬야 한다. 하나씩 알아보도록 하자.

### Job과 Step

Spring Batch는 배치 작업을 Job과 Step으로 구성한다. Job은 말 그대로 하나의 작업이다. 기존 작업을 기준으로 하면 ‘매일 새벽 두 시에 데이터를 처리한다’는 것이 바로 Job이 된다.

그렇다면 Step은 무엇일까? Step은 Job내에서 처리해야 하는 일을 나타낸다. 이는 기존 작업으로 한다면 각각 다음 처리들이 Step이 되는것이다.

1. `management` 테이블에서 데이터를 조회한다.
2. 조회한 결과 리스트를 `History` 엔티티로 변환한다.
3. 변환된 엔티티를 `history` 테이블에 삽입한다.
4. 이후 `management` 테이블의 복약 여부 정보를 초기화 한다.

이 작업들이 바로 Step이 되는 것이다. 정리하자면 Spring Batch에서 Job을 실행시키면 Job에 정의된 Step이 순차적으로 실행되어 비즈니스 로직을 처리한다.

### Step의 종류

**태스크릿 지향 처리 (Tasklet-Oriented Processing)**

복잡하지 않고 단발성 작업을 처리할 때 주로 사용된다. (예: 특정 키워드의 파일 일괄 삭제, 단발성 데이터베이스 쿼리 등)

**청크 지향 처리 (Chunk-Oriented Processing)**

데이터 처리 작업은 결국 데이터의 조회 → 처리(변환) → 쓰기 단계로 이루어진다. 이러한 패턴을 보이는 작업을 Spring Batch에서는 청크 지향 처리라고 부른다.

청크는 일정 크기를 가진 데이터 덩어리로, 앞서 기존 작업에서 보여주었던 것 처럼 100만 건의 데이터를 한 번에 처리하는 것이 아닌 1000건의 청크로 분할하여 처리하도록 하는 것이다. 어떻게 보면 기존 작업은 청크 지향 처리를 구현한 것이다.

Spring Batch는 다음 인터페이스로 데이터의 조회, 처리, 쓰기를 처리한다. 순서대로 알아보도록 하자.

**ItemReader**

```java
public interface ItemReader<T> {
    T read() throws Exception,
        UnexpectedInputException,
        ParseException,
        NonTransientResourceException;
}
```

read() 메서드는 아이템을 하나씩 반환한다. 아이템이란 파일의 한 줄, 데이터베이스의 한 행과 같은 하나의 데이터를 의미한다. 그리고 ItemReader가 null을 반환하면 읽을 데이터가 더 이상 없다는 의미이므로 청크 지향 처리 Step의 종료 시점이 된다.

**ItemProcessor**

```java
public interface ItemProcessor<I, O> {
    O process(I item) throws Exception;
}
```

ItemReader로부터 받은 아이템(I)을 비즈니스 로직이 요구하는 데이터 형태(O)로 변환하거나, 필요하다면 필터링도 수행한다. 만약 현재 아이템이 필요가 없다면 단순히 null을 반환하면 된다. 또한 데이터 가공이 필요하지 않으면 ItemProcessor는 생략 가능하다.

**ItemWriter**

```java
public interface ItemWriter<T> {
    void write(Chunk<? extends T> chunk) throws Exception;
}
```

ItemProcessor가 가공한 데이터를 받아 최종적으로 저장 / 출력한다. 이때 ItemWriter는 앞선 객체들이 아이템을 하나씩 처리하는 것과는 다르게 데이터를 한 건씩 쓰지 않고 청크 단위로 묶어서 한 번에 데이터를 쓴다.

이렇게 3개의 객체들로 구성된 청크 지향 처리는 어떻게 보면 데이터를 읽고, 처리하고, 쓴다는 데이터 처리의 표준이기도 하다. 이 패턴 덕분에 Spring Batch에서의 청크 지향 처리는 책임 분리, 재사용성 극대화, 높은 유연성을 가진다는 장점이 있다. 또한 청크 지향 처리는 청크 단위로 트랜잭션을 나누어 처리하기 때문에 예외 발생 시 해당 청크만 롤백된다.

### JobParameters

기존 작업에서는 조회하려는 날짜가 항상 하루 전으로 하드 코드 되어있었다. 이는 작업의 유연성을 망치는 일이 될 것이다. 당연히 Spring Batch에서는 Job 실행 시 파라미터를 전달하여 적절한 처리가 가능하고, 또한 중복되는 JobParameters로 작업을 실행하면 예외가 발생하기 때문에 중복 실행도 방지할 수 있다.

### Listener

Spring Batch는 Job, Step, Chunk, Item의 실행 주기에 따른 이벤트 처리 객체를 가지고 있다. 이를 통해 모니터링이나 실행 결과에 따른 후속 처리가 가능해진다.

### 메타 데이터

앞서 중복되는 JobParameters로 인한 재실행 방지 이외에도 Spring Batch는 재실행 시 실패된 청크부터 재시작할 수 있는 기능도 제공한다. 이는 어떤 마법적인 처리가 아니라 Spring Batch가 Job의 실행 이력을 메타데이터로 데이터베이스에 저장하기 때문에 가능한 일이다.

이때 실행 이력은 크게 두 가지 개념으로 관리된다.

- **JobInstance** : 동일한 Job 정의와 JobParameters 조합으로 실행된 하나의 배치 작업 단위
- **JobExecution** : JobInstance가 실행될 때마다 생성되는 실행 기록(시작 시간, 종료 시간, 성공/실패 여부 등)

덕분에 Spring Batch는 언제 어떤 Job이 실행되었는지, 어디서 실패했는지 추적할 수 있고, 실패한 시점에서 다시 이어서 실행하는 것도 가능하다.

# Spring Batch로 전환해보기

그러면 이제 기존 작업을 Spring Batch로 전환해보자.

## 사전 설정

배치 작업을 위해서 애플리케이션 설정을 먼저 하도록 하자.

**build.gradle**

다음 배치 관련 의존성을 불러오도록 하자.

```groovy
implementation 'org.springframework.boot:spring-boot-starter-batch'
```

**application.yml**

기존 작업의 모든 설정을 삭제하고, 다음과 같이 작성하자.

```yaml
spring:
  application:
    name: spring-batch
  sql:
    init:
      # 애플리케이션 실행 시 메타 데이터 저장소 삭제 스키마 실행
      mode: always
      schema-locations: classpath:org/springframework/batch/core/schema-drop-mysql.sql
  batch:
    jdbc:
      initialize-schema: always
  datasource:
    url: jdbc:mysql://localhost:3306/batch?rewriteBatchedStatements=true
    username: root
    password: 1234
```

`spring.sql.init.schema-locations`에 정의된 스키마 파일은 메타데이터 테이블들을 DROP하는 파일이다. 이렇게 설정을 통해 정의되면, 애플리케이션이 실행되었을 때 항상 실행되기 때문에 실제 운영 환경에서는 절대로 이렇게 하면 안된다. 지금은 매번 같은 배치 작업을 반복해서 실행해봐야 하기 때문에 설정한 것이다.

여기서 `spring.batch.jdbc.initialize-schema` 배치 관련 메타데이터 테이블을 생성하는 스키마 파일 실행 관련 설정이다. `always`는 애플리케이션 실행마다 항상 스키마 파일을 실행하려고 하기 때문에 특정 DBMS에서는 에러가 발생할 수 있다. 따라서 개발 환경에서만 사용하고, 로컬 환경에서는 지양하도록 하자. 그 외 설정은 아래 표로 확인해보도록 하자.

| 키워드     | 설명                                                    |
| ---------- | ------------------------------------------------------- |
| `embedded` | 내장 DB 사용 시 자동으로 메타데이터 테이블 생성, 기본값 |
| `never`    | 자동으로 테이블을 만들지 않음 (운영 환경에서 직접 사용) |

운영 환경에서는 Maven 기준으로 다음 경로로 찾아 들어가서 스키마 파일을 찾아 사용하려는 메타데이터 데이터베이스에 맞는 스키마를 실행시키면 된다.

```
org/springframework/batch/core/
```

`spring.datasource.url`에서 쿼리 파라미터로 `rewriteBatchedStatements=true` 옵션을 설정하였는데, 이 부분은 나중에 쓰기 작업을 처리할 때 설명하도록 하겠다.

## 배치 작업 설정

이제 사전 설정도 완료되었으니 배치 작업을 구성해보도록 하자.

### ItemReader

데이터를 읽는 역할을 하는 컴포넌트의 구현체는 `JdbcPagingItemReader`를 사용할 것이다. 이는 데이터베이스를 대상으로 읽음을 수행하는 컴포넌트 중에서는 사실상 표준으로 사용되는 구현체인데, 왜 JPA가 사용되는 컴포넌트가 아닐까? 그 이유는 `JpaPagingItemReader`가 OFFSET 방식을 사용하기 때문에 쿼리 성능이 별로 좋지 않기 때문이다.

하지만 `JdbcPagingItemReader`는 KEYSET 기반의 페이지네이션으로, 이전 페이지의 마지막 키 값을 기준으로 다음 데이터를 조회하여 쿼리 성능이 일정하게 유지된다. 따라서 `JdbcPagingItemReader`를 활용하여 `management` 테이블을 조회하는 컴포넌트를 작성해보도록 하자.

JPA의 기능을 사용하지 않기 때문에 `ManagementRow`라는 별도의 DTO를 만들도록 하자.

```java
public record ManagementRow(
    Long id,
    Long memberId,
    boolean morning,
    boolean lunch,
    boolean dinner,
    boolean sleep,
    boolean morningTaking,
    boolean lunchTaking,
    boolean dinnerTaking,
    boolean sleepTaking,
    LocalDate startDate,
    LocalDate endDate
) {}
```

엔티티를 그냥 가져다 쓰면 좋지 않을까 싶지만 이는 권장되지 않는 이유가 있다. 쿼리 결과 이후 데이터 레코드를 객체로 매핑하는 과정에서 두 가지 방식을 사용한다.

- BeanPropertyRowMapper: setter 기반 매팽
- DataClassRowMapper: record나 data class(kotlin)의 생성자 파라미터를 통한 매핑

즉, 엔티티를 그대로 사용하려면 모든 필드에 대한 setter를 열어둬야 한다. 하지만 이는 JPA 관점에서도 권장되지 않고, 배치 처리 과정에서 JPA 영속성 컨텍스트를 활용하지도 않는데 불필요하게 비즈니스 로직 전반에 setter를 허용하는 위험을 감수해야 한다. 따라서 배치 전용 DTO나 record를 별도로 두는 것이 더 안전하고 유지보수에도 유리하다.

```java
@Configuration
public class ItemReaderConfig {
    @Bean
    @StepScope
    public JdbcPagingItemReader<ManagementRow> managementReader(
        DataSource dataSource,
        @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        return new JdbcPagingItemReaderBuilder<ManagementRow>()
                .name("managementReader")
                .dataSource(dataSource)
                // 일반적으로 청크 사이즈와 동일하게 맞춘다.
                .pageSize(1000)
                // 쿼리 정의
                .selectClause("SELECT *")
                .fromClause("FROM management")
                .whereClause(":targetDate BETWEEN start_date AND end_date")
                .sortKeys(Map.of("id", Order.ASCENDING))
                .parameterValues(Map.of("targetDate", targetDate))
                // record 매핑을 여기서 수행한다.
                .dataRowMapper(ManagementRow.class)
                .build();
    }
}
```

여기서 `@StepScope`는 무엇일까? 이 어노테이션이 선언된 빈들은 애플리케이션 구동 시점에 프록시로 등록되고, Job 또는 Step이 실행된 이후에 프록시 객체에서 접근을 시도하면 실제 빈이 생성되도록 한다. 이를 활용하여 `ItemReader`가 `JobParameters`에 접근이 가능하게 해준다.

그리고 `sortKey()`는 정렬 이외에도 KEYSET 기반 페이지네이션 용도로도 사용되기 때문에 일반적으로 유니크한 값인 PK를 사용하면 된다.

### ItemProcessor

조회한 `ManagementRow` 객체를 변환해야 한다. 이를 위해서 `History` 엔티티가 아닌 `HistoryRow`라는 이름의 DTO를 다음과 같이 작성하였다.

```java
public record HistoryRow(
    Long managementId,
    Long memberId,
    LocalDate takingDate,
    boolean morning,
    boolean lunch,
    boolean dinner,
    boolean sleep,
    boolean morningTaking,
    boolean lunchTaking,
    boolean dinnerTaking,
    boolean sleepTaking
) {
    public static HistoryRow of(ManagementRow managementRow, LocalDate takingDate) {
      return new HistoryRow(
          managementRow.id(),
          managementRow.memberId(),
          takingDate,
          managementRow.morning(),
          managementRow.lunch(),
          managementRow.dinner(),
          managementRow.sleep(),
          managementRow.morningTaking(),
          managementRow.lunchTaking(),
          managementRow.dinnerTaking(),
          managementRow.sleepTaking()
      );
    }
}
```

그리고 `ItemProcessor`를 다음과 같이 작성하였다.

```java
@Configuration
public class ItemProcessorConfig {
    @Bean
    @StepScope
    public ItemProcessor<ManagementRow, HistoryRow> historyProcessor(
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        return managementRow -> HistoryRow.of(managementRow, targetDate);
    }
}
```

`management` 테이블에는 복용 시작일과 복용 종료일 밖에 없기 때문에 앞서 `ItemReader`와 마찬가지로 `JobParameters`에 접근하기 위해서 `@StepScope`를 명시하였다. 이렇게 단순한 변환 외에도 비즈니스 로직에 따라서 특정 데이터를 필터링 할 수도 있다.

### ItemWriter

이제 변환된 `HistoryRow`를 `history` 테이블에 INSERT 하도록 하자. `ItemWriter` 역시 JPA가 아닌 `JdbcBatchItemWriter`를 사용하여 다음과 같이 작성하였다.

```java
@Configuration
public class ItemWriterConfig {
    @Bean
    public JdbcBatchItemWriter<HistoryRow> historyWriter(DataSource dataSource) {
        return new JdbcBatchItemWriterBuilder<HistoryRow>()
                .dataSource(dataSource)
                .sql("""
                        INSERT INTO history (
                            management_id,
                            member_id,
                            taking_date,
                            morning,
                            lunch,
                            dinner,
                            sleep,
                            morning_taking,
                            lunch_taking,
                            dinner_taking,
                            sleep_taking
                        )
                        VALUES (
                            :managementId,
                            :memberId,
                            :takingDate,
                            :morning,
                            :lunch,
                            :dinner,
                            :sleep,
                            :morningTaking,
                            :lunchTaking,
                            :dinnerTaking,
                            :sleepTaking
                        )
                        """)
                .itemSqlParameterSourceProvider(new BeanPropertyItemSqlParameterSourceProvider<>())
                .assertUpdates(false)
                .build();
    }
}
```

이것이 가능한 이유는 바로 `itemSqlParameterSourceProvider()` 설정 덕분이다. 이를 통해 객체의 필드 이름을 활용하여 간편하게 데이터베이스에 INSERT 처리를 해줄 수 있다. 만약 외부 값을 사용해야 한다면 아래와 같이 `ItemPreparedStatementSetter()`를 사용하면 된다.

```java
@Configuration
public class ItemWriterConfig {
    @Bean
    public JdbcBatchItemWriter<HistoryRow> historyWriter(DataSource dataSource) {
        return new JdbcBatchItemWriterBuilder<HistoryRow>()
                .dataSource(dataSource)
                .sql("""
                        INSERT INTO history(
                            management_id,
                            member_id,
                            taking_date,
                            morning,
                            lunch,
                            dinner,
                            sleep,
                            morning_taking,
                            lunch_taking,
                            dinner_taking,
                            sleep_taking
                        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                        """)
                .itemPreparedStatementSetter((item, ps) -> {
                    ps.setLong(1, item.managementId());
                    ps.setLong(2, item.memberId());
                    ps.setDate(3, java.sql.Date.valueOf(item.takingDate()));
                    ps.setBoolean(4, item.morning());
                    ps.setBoolean(5, item.lunch());
                    ps.setBoolean(6, item.dinner());
                    ps.setBoolean(7, item.sleep());
                    ps.setBoolean(8, item.morningTaking());
                    ps.setBoolean(9, item.lunchTaking());
                    ps.setBoolean(10, item.dinnerTaking());
                    ps.setBoolean(11, item.sleepTaking());
                })
                .assertUpdates(false)
                .build();
    }
}
```

이제 `spring.datasource.url`의 쿼리 파라미터로 `rewriteBatchedStatements=true` 옵션을 설정한 이유를 설명할 차례다. Spring Batch의 ItemWriter는 내부적으로 `PreparedStatement`를 재사용하여 쿼리 템플릿 하나와 여러 파라미터 세트를 함께 전송한다.

이렇게 전달된 쿼리를 데이터베이스 서버에서 파싱하여 처리를 하는데, 이때 연결 정보에 다음 설정을 처리해주면 데이터베이스에서 Multi-Value-INSERT라는 하나의 INSERT로 통합된 쿼리로 실행하기 때문에 이러한 설정을 한 것이다.

### Tasklet 작성

여기서 Tasklet을 실행하는 Step이 하나 필요한데, 그 이유는 데이터 저장 처리가 끝나면 해당되는 일자에 대한 복용 여부 정보를 모두 `false`로 초기화 시켜야 하기 때문이다. 단일 쿼리는 데이터 읽기 - 처리 - 쓰기 작업인 청크 지향 처리가 아니므로 태스크릿 지향 처리로 다음과 같이 쿼리를 작성하였다.

```java
@Slf4j
@Component
@StepScope
public class ManagementResetTasklet implements Tasklet {
    private final JdbcTemplate jdbcTemplate;
    private final LocalDate targetDate;

    public ManagementResetTasklet(
            JdbcTemplate jdbcTemplate,
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        this.jdbcTemplate = jdbcTemplate;
        this.targetDate = targetDate;
    }

    @Override
    public RepeatStatus execute(StepContribution contribution, ChunkContext chunkContext) {
        log.info("management reset tasklet start");
        String sql = """
                UPDATE management
                SET morning_taking = false,
                lunch_taking = false,
                dinner_taking = false,
                sleep_taking = false
                WHERE ? BETWEEN start_date AND end_date
                """;

        int updatedRow = jdbcTemplate.update(sql, targetDate);

        log.info("updatedRow: {}", updatedRow);
        return RepeatStatus.FINISHED;
    }
}
```

### Job & Step 구성

이제 작성한 컴포넌트를 사용해서 Step들을 구성하고, 해당 Step들을 실행시키는 Job까지 구성해보도록 하자.

```java
@Slf4j
@Configuration
@RequiredArgsConstructor
public class HistoryBatchConfig {
    private final PlatformTransactionManager transactionManager;
    private final JobRepository jobRepository;
    private final WriteHistoryBatchListener historyBatchListener;

    @Bean
    public Job orderRecoveryJob(
            Step historyStep,
            Step managementResetStep
    ) {
        return new JobBuilder("writeHistoryJob", jobRepository)
                .start(historyStep)
                .next(managementResetStep)
                .build();
    }

    @Bean
    public Step historyStep(
            JdbcPagingItemReader<ManagementRow> managementReader,
            ItemProcessor<ManagementRow, HistoryRow> historyProcessor,
            JdbcBatchItemWriter<HistoryRow> historyWriter) {
        return new StepBuilder("writeStep", jobRepository)
                .<ManagementRow, HistoryRow>chunk(1000, transactionManager)
                .reader(managementReader)
                .processor(historyProcessor)
                .writer(historyWriter)
                .build();
    }

    @Bean
    public Step managementResetStep(
            ManagementResetTasklet managementResetTasklet
    ) {
        return new StepBuilder("managementResetStep", jobRepository)
                .tasklet(managementResetTasklet, transactionManager)
                .build();
    }
}
```

## 이벤트 리스너 작성

배치 작업 자체는 설정되었지만, 현재 상황이라면 아무런 로그도 출력되지 않을 것이다. 따라서 Job의 소요 시간과 메모리 점유율을 확인하기 위해, 이벤트 리스너를 만들어 기존 작업의 성능 측정에 사용되었던 `ExecutionMonitor`를 적용해보도록 하자.

### ExecutionMonitor 수정

그 전에 이전 작업에서 매 쓰기 작업마다 메모리 사용량 관련 로그를 보는 게 생각보다 번거로워서 `logMemory()`를 호출하면 내부적으로 연산을 거친 다음에 `end()` 호출 시 전체 메모리 사용량의 평균과 최대 메모리 사용량을 출력하도록 수정하였다.

```java
@Slf4j
public class ExecutionMonitor {
    private static long startTime;
    private static long count;
    private static long totalMemory;
    private static long maximumMemoryUsage;

    // 실행 시작 시 호출
    public static void start(String taskName) {
        startTime = System.currentTimeMillis();
        log.info("[{}]", taskName);
    }

    // 실행 종료 시 호출
    public static void end(String taskName) {
        Runtime runtime = Runtime.getRuntime();
        long endTime = System.currentTimeMillis();
        long elapsed = endTime - startTime;

        long max = runtime.maxMemory() / (1024 * 1024);
        double ratio = Math.round(((double) (totalMemory / count) / max) * 1000) / 1000.0;
        log.info("[{}] Elapsed Time: {} ms ({} sec)",
                taskName, elapsed, elapsed / 1000.0);

        log.info("Average Memory Usage: {} MB / {} MB ({} %)",
                totalMemory / count, max, String.format("%.3f", ratio * 100));

        ratio = Math.round(((double) maximumMemoryUsage / count) / max * 1000) / 1000.0;
        log.info("Maximum Memory Usage: {} MB / {} MB ({} %)",
                maximumMemoryUsage, max, String.format("%.3f", ratio * 100));
    }

    // 메모리 사용량 로그
    public static void logMemory() {
        Runtime runtime = Runtime.getRuntime();
        long used = (runtime.totalMemory() - runtime.freeMemory()) / (1024 * 1024);
        totalMemory += used;
        maximumMemoryUsage = Math.max(maximumMemoryUsage, used);
        count++;
    }
}
```

### 이벤트 리스너 작성 및 설정

그리고 다음과 같이 이벤트 리스너를 작성한다.

```java
@Slf4j
@Component
public class WriteHistoryBatchListener {
    @BeforeJob
    public void beforeJob(JobExecution jobExecution) {
        ExecutionMonitor.start("writeHistoryJob 실행");
    }

    @AfterJob
    public void afterJob(JobExecution jobExecution) {
        ExecutionMonitor.end("writerHistoryJob 종료");
    }

    @BeforeChunk
    public void beforeChunk(ChunkContext context) {
        ExecutionMonitor.logMemory();
    }
}
```

필요한 이벤트 리스너가 정의된 인터페이스를 구현하는 것이 기본이지만 이렇게 어노테이션으로 하면 깔끔하게 코드를 작성할 수 있다. 이제 이렇게 작성한 것을 Job과 Step에 다음과 같이 등록하면 된다.

```java
@Slf4j
@Configuration
@RequiredArgsConstructor
public class HistoryBatchConfig {
    private final PlatformTransactionManager transactionManager;
    private final JobRepository jobRepository;
    private final WriteHistoryBatchListener historyBatchListener;

    @Bean
    public Job orderRecoveryJob(
            Step historyStep,
            Step managementResetStep
    ) {
        return new JobBuilder("writeHistoryJob", jobRepository)
                .start(historyStep)
                .next(managementResetStep)
                // 리스너 등록
                .listener(historyBatchListener)
                .build();
    }

    @Bean
    public Step historyStep(
            JdbcPagingItemReader<ManagementRow> managementReader,
            ItemProcessor<ManagementRow, HistoryRow> historyProcessor,
            JdbcBatchItemWriter<HistoryRow> historyWriter) {
        return new StepBuilder("writeStep", jobRepository)
                .<ManagementRow, HistoryRow>chunk(1000, transactionManager)
                .reader(managementReader)
                .processor(historyProcessor)
                .writer(historyWriter)
                // 리스너 등록
                .listener(historyBatchListener)
                .build();
    }

    @Bean
    public Step managementResetStep(
            ManagementResetTasklet managementResetTasklet
    ) {
        return new StepBuilder("managementResetStep", jobRepository)
                .tasklet(managementResetTasklet, transactionManager)
                .build();
    }
}

```

이렇게 등록하면 Job과 Step의 이벤트에 해당하는 리스너를 실행할 수 있다.

## 배치 작업 실행

배치 작업을 실행 하는 방법은 몇 가지 존재하지만, 여기서는 가장 무난하게 IDE의 gradlew 명령어로 작업을 실행하여 결과를 확인해보도록 하겠다. 그 전에 `@EnableScheduling`을 비활성화 하여 기존 작업의 실행을 방지하자.

```java
// @EnableScheduling
@SpringBootApplication
public class SpringBatchApplication {

    public static void main(String[] args) {
        SpringApplication.run(SpringBatchApplication.class, args);
    }

}
```

그리고 다음 명령어를 입력하면 배치 작업이 실행된다.

```bash
./gradlew bootRun --args='--spring.batch.job.name=writeHistoryJob targetDate=2025-10-02,java.time.LocalDate'
```

이제 실행 결과를 살펴보자.

```
[writeHistoryJob 실행]
Executing step: [writeStep]
Step: [writeStep] executed in 2m32s715ms
Executing step: [managementResetStep]
management reset tasklet start
updatedRow: 1000000
Step: [managementResetStep] executed in 5s167ms
[writerHistoryJob 종료] Elapsed Time: 157948 ms (157.948 sec)
Average Memory Usage: 79 MB / 4022 MB (2.000 %)
Maximum Memory Usage: 133 MB / 4022 MB (3.300 %)
Job: [SimpleJob: [name=writeHistoryJob]] completed with
the following parameters: [{'targetDate':'{value=2025-10-02,
type=class java.time.LocalDate, identifying=true}'}] and
the following status: [COMPLETED] in 2m37s962ms
```

작업 실행 시간은 IDE의 기본적인 로그 출력으로 알 수 있어서 해당 로그를 기준으로 계산해보도록 하자. 우선 100만 건의 데이터를 대상으로 기존 작업이 15분 ~ 20분 정도 걸려서 처리하던 것을 Spring Batch 전환 이후 2분 ~ 3분으로 줄였는데, 이는 최대 시간 기준 약 85%로 시간을 단축시킨 것이다,

또한 최대 1.3GB를 점유하던 메모리 사용률을 최대 133MB를 점유하도록 줄였는데, 이는 기존 작업의 메모리 점유율 대비 약 90% 정도 개선한 것이다. 정리하자면 메모리 사용률과 처리 시간 모두 기존 작업에 비해서 획기적으로 단축시켰다!

## 추가 테스트

그렇다면 추가적으로 테스트를 한 번 진행해보자. 기존 작업은 100만 건 이상의 데이터를 처리하려고 하면 OOM이 발생하거나 시간이 굉장히 오래 걸릴 것이 예상되어서 데이터 크기를 늘려 테스트 하기 조금 꺼려졌다.

하지만 Spring Batch를 활용했을 때 소요 시간과 메모리 점유를 획기적으로 개선했으므로, 과감하게 현재 데이터 레코드의 10배인 1000만 건의 데이터를 대상으로 배치 작업을 처리해보도록 하자.

### 사전 설정

**데이터베이스**

`management` 테이블에 1000만 건의 무작위 데이터 생성을 위해서 다음 쿼리를 실행시켰다.

```sql
CREATE TABLE IF NOT EXISTS digits (d TINYINT UNSIGNED PRIMARY KEY);

INSERT IGNORE INTO digits (d)
VALUES (0),(1),(2),(3),(4),(5),(6),(7),(8),(9);

CREATE OR REPLACE VIEW seq_ten_million AS
SELECT
    (d7.d*1000000 + d6.d*100000 + d5.d*10000 + d4.d*1000 + d3.d*100 + d2.d*10 + d1.d) + 1 AS n
FROM digits d1
         CROSS JOIN digits d2
         CROSS JOIN digits d3
         CROSS JOIN digits d4
         CROSS JOIN digits d5
         CROSS JOIN digits d6
         CROSS JOIN digits d7;

DROP TABLE IF EXISTS `management`;
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
FROM seq_ten_million
LIMIT 10000000;
```

**청크 크기 변경**

그리고 처리해야 할 데이터가 많기 때문에 청크 크기와 한 번에 조회하는 레코드의 개수 크기도 좀 더 늘려서 5000으로 조정하였다.

```java
@Slf4j
@Configuration
@RequiredArgsConstructor
public class HistoryBatchConfig {
    private final PlatformTransactionManager transactionManager;
    private final JobRepository jobRepository;
    private final WriteHistoryBatchListener historyBatchListener;

    // ...

    @Bean
    public Step historyStep(
            JdbcPagingItemReader<ManagementRow> managementReader,
            ItemProcessor<ManagementRow, HistoryRow> historyProcessor,
            JdbcBatchItemWriter<HistoryRow> historyWriter) {
        return new StepBuilder("writeStep", jobRepository)
                .<ManagementRow, HistoryRow>chunk(5000, transactionManager)
                // ...
    }
    // ...
}

```

```java
@Configuration
public class ItemReaderConfig {
    @Bean
    @StepScope
    public JdbcPagingItemReader<ManagementRow> managementReader(
            DataSource dataSource,
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        return new JdbcPagingItemReaderBuilder<ManagementRow>()
                .name("managementReader")
                .dataSource(dataSource)
                .pageSize(5000)
                // ...
    }
}
```

### 배치 작업 실행 및 결과 확인

이제 1000만 건의 데이터를 대상으로 배치 작업을 처리해보자. 명령어는 앞서 실행했던 명령어와 동일하다.

```
[writeHistoryJob 실행]
Executing step: [writeStep]
Step: [writeStep] executed in 22m5s695ms
Executing step: [managementResetStep]
management reset tasklet start
updatedRow: 10000000
Step: [managementResetStep] executed in 1m5s74ms
[writerHistoryJob 종료] Elapsed Time: 1390840 ms (1390.84 sec)
Average Memory Usage: 122 MB / 4022 MB (3.000 %)
Maximum Memory Usage: 196 MB / 4022 MB (4.900 %)
Job: [SimpleJob: [name=writeHistoryJob]] completed with
the following parameters: [{'targetDate':'{value=2025-10-02,
type=class java.time.LocalDate, identifying=true}'}] and
the following status: [COMPLETED] in 23m10s857ms
```

생각보다 많은 시간이 소요된 것을 확인할 수 있다. 왜 그럴까?

### 문제 해결

문제 해결을 위해서 AI에게 물어보며 여러 설정을 해봤다.

**application.yml**

애플리케이션의 데이터베이스 관련 설정이 대용량 데이터를 처리하기에는 미비하다는 판단을 하였다. 따라서 연결 정보에 추가적인 파라미터를 전달하는 것이 어떻겠냐는 조언을 하여 다음과 같이 설정하였다.

```yaml
spring:
  application:
    name: spring-batch
  sql:
    init:
      # 애플리케이션 실행 시 메타 데이터 저장소 삭제 스키마 실행
      mode: always
      schema-locations: classpath:org/springframework/batch/core/schema-drop-mysql.sql
  batch:
    jdbc:
      initialize-schema: always
  datasource:
    url: jdbc:mysql://localhost:3306/batch
      ?rewriteBatchedStatements=true
      # 클라이언트의 PreparedStatement 사용
      &useServerPrepStmts=false
      # PreparedStatement 캐시 활성화 (반복 SQL 파싱 방지)
      &cachePrepStmts=true
      # 캐시할 PreparedStatement 최대 개수
      &prepStmtCacheSize=250
      # 캐시 가능한 SQL 길이 제한 (기본 256 → 확장)
      &prepStmtCacheSqlLimit=2048
    username: root
    password: 1234
    hikari:
		  # 기본 autocommit 비활성화
		  # 트랜잭션을 Spring Batch의 chunk 단위로 제어 (필수)
      auto-commit: false
      # 대량 INSERT 시 커밋 횟수 ↓, 디스크 flush ↓
      maximum-pool-size: 10
```

하지만 이렇게 설정하고 배치 작업을 재실행 하여도 비슷한 시간이 소요되었다.

**데이터베이스 관련 설정**

데이터베이스 자체에도 몇몇 설정을 해줘야 한다고 한다. 따라서 다음 명령어들로 MySQL 컨테이너에 접속한 다음에 파일을 하나 생성하였다.

```bash
docker exec -it mysql-batch bash
```

```bash
cd /etc/mysql/conf.d
```

```bash
echo '[mysqld]
innodb_flush_log_at_trx_commit=2
innodb_buffer_pool_size=4G
innodb_redo_log_capacity=8G
innodb_write_io_threads=8
innodb_read_io_threads=8
' > custom.cnf
```

각 설정에 대한 설명은 아래 표로 정리하였다.

| 설정 항목                        | 권장값 | 역할 / 설명                                                                                                                                    | 효과                                                                                    |
| -------------------------------- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| `innodb_flush_log_at_trx_commit` | `2`    | 트랜잭션 커밋 시 로그를 디스크로 즉시 flush하지 않고 1초 단위로 동기화하도록 설정한다.                                                         | 디스크 I/O 감소로 성능이 2~3배 향상되며, 전원 장애 시 1초 내 데이터 손실 가능성이 있다. |
| `innodb_buffer_pool_size`        | `4G`   | InnoDB 캐시 메모리 크기를 지정한다. 데이터와 인덱스를 메모리에 캐싱하여 읽기/쓰기 성능을 향상시킨다. (전용 DB 서버라면 RAM의 60~70% 수준 권장) | 읽기·쓰기 성능이 향상되고 디스크 접근이 최소화된다.                                     |
| `innodb_redo_log_capacity`       | `8G`   | MySQL 8.0.30+ 이후 도입된 설정으로, 전체 Redo Log(트랜잭션 로그)의 용량을 지정한다. 과거 `innodb_log_file_size` 설정을 대체한다.               | 로그 flush 빈도가 감소하여 대용량 배치 처리 시 쓰기 지연이 완화된다.                    |
| `innodb_write_io_threads`        | `8`    | InnoDB의 쓰기 I/O를 처리하는 백그라운드 스레드 수를 지정한다. 멀티코어 CPU 환경에서 병렬 디스크 쓰기를 처리한다.                               | 병렬 처리 성능이 향상된다.                                                              |
| `innodb_read_io_threads`         | `8`    | InnoDB의 읽기 I/O를 처리하는 백그라운드 스레드 수를 지정한다.                                                                                  | 읽기 요청이 많은 경우 성능이 향상되지만, 쓰기 설정보다 영향은 다소 적다.                |

해당 설정을 한 다음 컨테이너 재시작 후 배치 작업을 테스트해봤는데 시간 개선이 전혀 이루어지지 않았다. 그 외에도 청크 크기도 조정해보았지만 유의미한 변화는 없었다.

**Docker I/O 문제**

Docker는 기본적으로 overlay2(Copy-on-Write) 파일 시스템을 사용하는데, 이는 파일을 덮어쓸 때마다 새로운 레이어를 만들어서 merge하는 방식이라고 한다. 따라서 랜덤 쓰기가 많은 작업에서 성능 저하 이슈가 있다고 한다.

따라서 호스트 PC에 직접 데이터베이스를 설치하거나 아니면 WSL 환경에서 아래 명령어로 데이터베이스 컨테이너의 데이터들을 WSL 환경에 마운트하고, 배치 작업 또한 WSL 환경에서 실행하여 이러한 I/O 병목을 줄이는 방법이 있다고 한다. 기존 컨테이너를 삭제하고 WSL에 접속하여 다음 명령어로 새로운 MySQL 컨테이너를 실행하였다.

```bash
docker run -d \
  --name mysql-batch \
  -e MYSQL_ROOT_PASSWORD=1234 \
  -e MYSQL_DATABASE=batch \
  -p 3306:3306 \
  -v /home/{WSL_USERNAME}/mysql-data:/var/lib/mysql \
  --restart always \
  mysql:9.4
```

그 다음 jar를 빌드하여 WSL 내부에서 배치 작업을 실행시킬 수 있도록 WSL에서 다음 명령어로 jar 파일을 WSL 홈 디렉터리로 복사하였다.

```bash
cp /mnt/c/{JAR_FILE_PATH} ~/
```

테이블과 데이터 설정 이후, 앞서 처리한 모든 애플리케이션과 데이터베이스 관련 설정을 한 다음에 테스트를 진행해봤다. jar 파일일 때의 배치 실행 명령어는 다음과 같다.

```bash
java -jar {JAR_FILE_NAME}.jar \
  --spring.batch.job.name=writeHistoryJob \
  targetDate=2025-10-02,java.time.LocalDate
```

실행 결과는 다음과 같았다.

**청크 크기가 3000인 경우**

```
Executing step: [writeStep]
Step: [writeStep] executed in 11m21s544ms
Executing step: [managementResetStep]
management reset tasklet start
updatedRow: 10000000
Step: [managementResetStep] executed in 1m10s545ms
[writerHistoryJob 종료] Elapsed Time: 752363 ms (752.363 sec)
Average Memory Usage: 84 MB / 1950 MB (4.300 %)
Maximum Memory Usage: 152 MB / 1950 MB (7.800 %)
Job: [SimpleJob: [name=writeHistoryJob]] completed with
the following parameters: [{'targetDate':'{value=2025-10-02,
type=class java.time.LocalDate, identifying=true}'}] and
the following status: [COMPLETED] in 12m32s437ms
```

**청크 크기가 5000인 경우**

```
Executing step: [writeStep]
Step: [writeStep] executed in 11m39s501ms
Executing step: [managementResetStep]
management reset tasklet start
updatedRow: 10000000
Step: [managementResetStep] executed in 1m17s203ms
[writerHistoryJob 종료] Elapsed Time: 776767 ms (776.767 sec)
Average Memory Usage: 118 MB / 1950 MB (6.100 %)
Maximum Memory Usage: 197 MB / 1950 MB (10.100 %)
Job: [SimpleJob: [name=writeHistoryJob]] completed with
the following parameters: [{'targetDate':'{value=2025-10-02,
type=class java.time.LocalDate, identifying=true}'}] and
the following status: [COMPLETED] in 12m56s781ms
```

여담으로 100만 건의 데이터도 WSL에서 재실행 해봤다.

**청크 크기가 1000인 경우**

```
 Executing step: [writeStep]
 Step: [writeStep] executed in 1m24s243ms
 Executing step: [managementResetStep]
 management reset tasklet start
 updatedRow: 1000000
 Step: [managementResetStep] executed in 6s538ms
 [writerHistoryJob 종료] Elapsed Time: 90846 ms (90.846 sec)
 Average Memory Usage: 76 MB / 1950 MB (3.900 %)
 Maximum Memory Usage: 130 MB / 1950 MB (6.700 %)
 Job: [SimpleJob: [name=writeHistoryJob]] completed with
 the following parameters: [{'targetDate':'{value=2025-10-02,
 type=class java.time.LocalDate, identifying=true}'}] and
 the following status: [COMPLETED] in 1m30s861ms
```

**청크 크기가 5000인 경우**

```
Executing step: [writeStep]
Step: [writeStep] executed in 1m14s970ms
Executing step: [managementResetStep]
management reset tasklet start
updatedRow: 1000000
Step: [managementResetStep] executed in 7s60ms
[writerHistoryJob 종료] Elapsed Time: 82119 ms (82.119 sec)
Average Memory Usage: 117 MB / 1950 MB (6.000 %)
Maximum Memory Usage: 172 MB / 1950 MB (8.800 %)
Job: [SimpleJob: [name=writeHistoryJob]] completed with
the following parameters: [{'targetDate':'{value=2025-10-02,
type=class java.time.LocalDate, identifying=true}'}] and
the following status: [COMPLETED] in 1m22s141ms
```

여기서 한 가지 알 수 있는 건 청크 크기가 크다고 배치 작업이 눈에 띄게 빨라지거나 하지는 않는다는 것이다. 100만 건의 데이터 대상으로는 청크 크기가 5000인 경우가 8초 더 빠르지만, 개선 폭에 비해서 매모리는 2%p 더 차지하는 경우가 나타났다.

심지어 1000만 건의 데이터는 청크 크기가 5000인 경우가 메모리 점유와 소요 시간이 더 요구되었다. 따라서 각 배치 작업에 따라서 여러 테스트를 진행하여 적절한 청크 크기를 설정해야 한다.

---

이렇게 Spring Scheduler를 활용한 어설픈 배치 작업에서 Spring Batch를 활용한 배치 작업으로 성공적으로 전환하였다. 결과적으로 소모 시간은 기존 작업 대비 85% 이상, 메모리 점유율은 기존 작업 대비 90% 이상 개선할 수 있었다.

생각보다 리팩토링 결과를 작성하는 글이 많이 늦어졌다. AI를 활용하면 내가 아는 것 이상의 지식을 활용할 수 있기 때문에 구현 자체는 이보다 훨씬 빨랐겠지만, 배치 작업은 실제 고객의 금전적인 부분을 처리하는 작업인 경우도 많기 때문에 그런 식으로 배우고 ‘배치를 할 줄 안다’고 스스로 착각하면 위험할 것 같아서 강의를 통해 자세히 공부해야 한다고 생각하였고, 이 선택은 틀리지 않았다고 생각한다. 해당 포스팅을 작성하는데 도움이 된 강의를 참고자료에 공유하도록 하겠다.

다음 포스팅은 배치 작업의 재시도 기능과, Spring Batch Flow를 통해 Step의 성공 유무에 따라서 각기 다른 Step으로 전이하도록 처리해보는 작업을 처리할 것이다. 이를 통해 데이터 처리 성공 시 `history`에서 `targetDate`에 해당되는 데이터들을 조회한 다음에 이를 CSV로 변환하여 이메일로 전송하고, 실패시에는 실패했다는 이메일을 전송하는 것을 해볼 것이다.

# 참고자료

[**Spring Batch**](https://spring.io/projects/spring-batch#overview)

[**죽음의 Spring Batch: 새벽 3시의 처절한 공포는 이제 끝이다.**](https://www.inflearn.com/course/%EC%A3%BD%EC%9D%8C%EC%9D%98-spring-batch/dashboard)
