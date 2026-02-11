---
title: Spring Batch - CSV 백업 및 알림 메일 처리

categories:
  - Spring Batch

toc: true
toc_sticky: true
published: true

date: 2025-12-09
last_modified_at: 2025-12-09
---

Spring Batch로 전환한 덕분에 메모리 점유율과 처리 시간을 90% 정도 개선할 수 있었다. 여기서 추가적으로 처리가 완료된 다음에 처리된 데이터를 CSV 파일로 백업한 다음에 해당 파일을 메일로 보내도록 만들어봤다.

참고로 초기에는 여러 ItemWriter를 받아서 순차적으로 처리하는 `CompositeItemWriter`를 활용해서 데이터베이스에 내역을 INSERT한 다음 파일 쓰기 작업을 진행하도록 만드려고 했다. 하지만 내역 관련 정보를 아직 저장하기 전의 상황이므로 해당 Step이 완료된 이후에 추가적인 Step을 활용하여 `history` 테이블에 있는 데이터를 읽어서 CSV 파일로 만들어야 좀 더 타당할 것 같아서 추가적인 Step을 정의하여 Job에 추가하였다.

# 백업 및 이메일 Step 구현

## CSV 파일 백업 Step

### ItemReader

`history` 테이블에서 주어진 날에 대한 데이터를 조회해야 하므로 `JdbcPagingItemReader`를 다음과 같이 설계하였다.

```java
@Configuration
public class ItemReaderConfig {
		// ...
    @Bean
    @StepScope
    public JdbcPagingItemReader<HistoryRow> historyReader(
            DataSource dataSource,
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        return new JdbcPagingItemReaderBuilder<HistoryRow>()
                .name("historyReader")
                .dataSource(dataSource)
                .pageSize(1000)
                .selectClause("SELECT *")
                .fromClause("FROM history")
                .whereClause("taking_date = :targetDate")
                .sortKeys(Map.of("id", Order.ASCENDING))
                .parameterValues(Map.of("targetDate", targetDate))
                .dataRowMapper(HistoryRow.class)
                .build();
    }
}
```

### ItemWriter

CSV 파일을 만들어야 하기 때문에 이전 작업에서 사용했던 데이터베이스 쓰기용 객체인 `JdbcBatchItemWriterBuilder` 대신에 파일 쓰기용 객체인 `FlatFileItemWriter`를 사용하였다.

```java
@Configuration
public class ItemWriterConfig {
		// ...
    @Bean
    @StepScope
    public FlatFileItemWriter<HistoryRow> historyFlatFileWriter(
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        return new FlatFileItemWriterBuilder<HistoryRow>()
                .name("historyFlatFileWriter")
                .resource(new FileSystemResource(targetDate + ".csv"))
                .delimited()
                .delimiter(",")
                .names(
                        "id",
                        "memberId",
                        "takingDate",
                        "morning",
                        "lunch",
                        "dinner",
                        "sleep",
                        "morningTaking",
                        "lunchTaking",
                        "dinnerTaking",
                        "sleepTaking"
                )
                .build();
    }
}
```

### Step 구성 및 등록

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
            Step managementResetStep,
            Step historyBackupStep
    ) {
        return new JobBuilder("writeHistoryJob", jobRepository)
                .start(historyStep)
                .next(managementResetStep)
                .next(historyBackupStep)
                .build();
    }

    // ...

    @Bean
    public Step historyBackupStep(
            JdbcPagingItemReader<HistoryRow> historyReader,
            FlatFileItemWriter<HistoryRow> historyFlatFileWriter
    ) {
        return new StepBuilder("historyBackupStep", jobRepository)
                .<HistoryRow, HistoryRow>chunk(1000, transactionManager)
                .reader(historyReader)
                .writer(historyFlatFileWriter)
                .build();
    }
}
```

## 메일 전송 Step

이제 CSV 백업 Step 이후 처리 결과를 메일로 보내는 Step을 작성해보도록 하자. 다음과 같이 만들 것이다.

- Step 성공시 History Batch Success를 보낸다.
- Step 실패 시 History Batch Fail과 함께 오류 이유를 보낸다.

### 애플리케이션 설정

메일을 보내기 위해서 Spring Mail 의존성을 불러온다.

```groovy
implementation 'org.springframework.boot:spring-boot-starter-mail'
```

그리고 application.yml에 다음 설정을 추가한다.

```yaml
spring:
  mail:
    host: smtp.gmail.com
    port: 587
    username: { EMAIL_USERNAME }
    password: { EMAIL_APP_PASSWORD }
    properties:
      mail.smtp.auth: true
      mail.smtp.starttls.enable: true
```

간단하게 gmail을 사용해서 SMTP를 구성해봤다. SMTP 관련 설정은 블로그 글을 참고했다.

### Tasklet 구성

```java
@Component
@StepScope
public class HistoryResultSendingTasklet implements Tasklet {

    private static final String FILE_NAME = "%s.csv";
    private static final String RECEIVER = "{RECEIVER_EMAIL_ADDRESS}";
    public static final String KEY_SUCCESS = "historyBatchSuccess";
    public static final String KEY_FAILED_STEP = "failedStep";
    public static final String KEY_ERROR = "historyBatchError";

    private final JavaMailSender sender;
    private final LocalDate targetDate;

    public HistoryResultSendingTasklet(
            JavaMailSender sender,
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        this.sender = sender;
        this.targetDate = targetDate;
    }

    @Override
    public RepeatStatus execute(StepContribution contribution, ChunkContext chunkContext) throws Exception {
        MimeMessage mimeMessage = sender.createMimeMessage();
        Boolean success = (Boolean) chunkContext.getStepContext()
                .getStepExecution()
                .getJobExecution()
                .getExecutionContext()
                .get(KEY_SUCCESS);

        String message = "History Batch Success";

        if (!success) {
            message = "History Batch Fail";
            message += "\n" + chunkContext
                    .getStepContext()
                    .getStepExecution()
                    .getJobExecution()
                    .getExecutionContext()
                    .getString(KEY_FAILED_STEP);

            message += "\n" + chunkContext
                    .getStepContext()
                    .getStepExecution()
                    .getJobExecution()
                    .getExecutionContext()
                    .getString(KEY_ERROR);
        }

        try {
            MimeMessageHelper helper = new MimeMessageHelper(mimeMessage, true);
            helper.setTo(RECEIVER);
            helper.setSubject("History Backup");
            if (success) {
                File file = new File(
                        new FileSystemResource(String.format(FILE_NAME, targetDate)).getFile().getAbsolutePath());

                helper.addAttachment(file.getName(), file);
            }
            helper.setText(message);
            sender.send(mimeMessage);
        } catch (MessagingException e) {
            e.printStackTrace();
        }
        return RepeatStatus.FINISHED;
    }
}
```

여기서 `ChunkContext`로 `JobExecution`에 존재하는 `ExecutionContext`에 접근하여 특정 값을 추출하는 코드를 볼 수 있다. 이 객체는 Job에 소속된 전체 Step이 공유할 수 있는 저장 공간이다.

### 리스너 설정

Tasklet에서 정보를 가져오기 위해서 각 Step이 끝난 뒤 실행되는 리스너를 하나 설정해야 한다.

```java
@Component
public class HistoryBackupStepListener implements StepExecutionListener {

    public static final String KEY_SUCCESS = "historyBatchSuccess";
    public static final String KEY_ERROR = "historyBatchError";
    public static final String KEY_FAILED_STEP = "failedStep";

    @Override
    public ExitStatus afterStep(StepExecution stepExecution) {
        ExitStatus exitStatus = stepExecution.getExitStatus();

        boolean isSuccess = exitStatus.equals(ExitStatus.COMPLETED);
        if (!isSuccess) {
            List<Throwable> exceptions = stepExecution.getFailureExceptions();

            StringBuilder sb = new StringBuilder();
            for (Throwable e : exceptions) {
                sb.append(e.getMessage()).append("\n");
            }

            stepExecution.getJobExecution()
                    .getExecutionContext()
                    .put(KEY_ERROR, sb.toString());

            stepExecution.getJobExecution()
                    .getExecutionContext()
                    .put(KEY_FAILED_STEP, stepExecution.getStepName());
        }

        stepExecution.getJobExecution()
                .getExecutionContext()
                .put(KEY_SUCCESS, isSuccess);

        return exitStatus;
    }
}
```

`StepExecution`은 리스너가 호출된 Step이 가진 정보라고 생각하면 된다. 해당 객체를 이용하여 현재 Step이 실행이 정상적으로 완료되었는지 확인하고 `JobExecutionContext`에 접근하여 처리 결과를 저장할 수 있다.

만일 Step이 정상적으로 종료되지 않으면 예외와 정상적으로 처리되지 않은 Step의 이름을 `JobExecutionContext`에 저장한다. 그리고 이 리스너를 각 Step의 리스너로 등록해주면 된다.

```java
@Slf4j
@Configuration
@RequiredArgsConstructor
public class HistoryBatchConfig {
    private final PlatformTransactionManager transactionManager;
    private final JobRepository jobRepository;

    // ...

    @Bean
    public Step historyStep(
            JdbcPagingItemReader<ManagementRow> managementReader,
            ItemProcessor<ManagementRow, HistoryRow> historyProcessor,
            JdbcBatchItemWriter<HistoryRow> historyWriter,
            HistoryBackupStepListener historyBackupStepListener
    ) {
        return new StepBuilder("writeStep", jobRepository)
                .<ManagementRow, HistoryRow>chunk(1000, transactionManager)
                // ...
                .listener(historyBackupStepListener)
                .build();
    }

    @Bean
    public Step managementResetStep(
            ManagementResetTasklet managementResetTasklet,
            HistoryBackupStepListener historyBackupStepListener
    ) {
        return new StepBuilder("managementResetStep", jobRepository)
                .tasklet(managementResetTasklet, transactionManager)
                .listener(historyBackupStepListener)
                .build();
    }

    @Bean
    public Step historyBackupStep(
            JdbcPagingItemReader<HistoryRow> historyReader,
            FlatFileItemWriter<HistoryRow> historyFlatFileWriter,
            HistoryBackupStepListener historyBackupStepListener
    ) {
        return new StepBuilder("historyBackupStep", jobRepository)
                .<HistoryRow, HistoryRow>chunk(1000, transactionManager)
                // ...
                .listener(historyBackupStepListener)
                .build();
    }
}
```

### Step 구성 및 등록

작성한 Tasklet을 실행하는 Step을 만들고 이를 Job의 가장 마지막 순서에 배치하였다.

```java
@Slf4j
@Configuration
@RequiredArgsConstructor
public class HistoryBatchConfig {
    private final PlatformTransactionManager transactionManager;
    private final JobRepository jobRepository;

    @Bean
    public Job orderRecoveryJob(
            Step historyStep,
            Step managementResetStep,
            Step historyBackupStep,
            Step resultSendingStep
    ) {
        return new JobBuilder("writeHistoryJob", jobRepository)
                .start(historyStep)
                .next(managementResetStep)
                .next(historyBackupStep)
                .next(resultSendingStep)
                .build();
    }

		// ...

    @Bean
    public Step resultSendingStep(
            HistoryResultSendingTasklet historyResultSendingTasklet
    ) {
        return new StepBuilder("historySendingStep", jobRepository)
                .tasklet(historyResultSendingTasklet, transactionManager)
                .build();
    }
}
```

## 실행

이제 작업 성공의 유무에 따라서 메일로 CSV 파일이 보내지거나 작업 실패에 대한 원인을 담은 이메일이 보내질 것이다. 다음 명령어로 CSV 파일이 보내지는지 확인해보도록 하자.

```bash
./gradlew bootRun --args='--spring.batch.job.name=writeHistoryJob targetDate=2025-10-02,java.time.LocalDate'
```

![image.png](/assets/images/development/spring-batch/25-12-09-large-scale-data-processing-with-spring-batch-with-flow/01.png)

정상적으로 CSV 파일이 보내진 것을 확인할 수 있다. 그렇다면 오류 상황은 어떨까? ItemProcessor에 임의의 예외를 던져서 확인해보았다.

```java
@Configuration
public class ItemProcessorConfig {
    @Bean
    @StepScope
    public ItemProcessor<ManagementRow, HistoryRow> historyProcessor(
            @Value("#{jobParameters['targetDate']}") LocalDate targetDate
    ) {
        throw new IllegalArgumentException("임의의 예외 발생!");
//        return managementRow -> HistoryRow.of(managementRow, targetDate);
    }
}
```

다음과 같이 오류가 발생하면서 작업 자체가 종료되었다.

```
Factory method 'historyProcessor' threw exception with message: 임의의 예외 발생!
...
```

그 이유는 기본적으로 Spring Batch는 각 Step 처리 중 예외가 발생하면 Job 자체가 종료되기 때문이다. 그러면 만약에 Step이 실패하면 바로 이메일 관련 Step으로 이동하도록 만들 수 있을까? 이를 위해 Spring Batch Flow를 사용할 수 있다.

# Spring Batch Flow

Spring Batch Flow는 조건에 따라 작업 흐름을 분기하고, 복잡한 실행 경로를 설계하는 방법이다. 이를 통해 시스템 조건에 따라 다른 처리 경로를 선택하거나, 실패 시 대체 작업을 수행하여 작업 간 복잡한 의존성을 관리할 수 있다.

## Flow 핵심 요소

Spring Batch Flow는 3개의 핵심 요소가 존재한다.

**상태 (State)**

상태는 Flow 내에서 현재 실행이 머무르거나 도달할 수 있는 모든 논리적 지점을 의미한다. Flow를 구성하는 상태는 그게 두 가지 주요 유형으로 분류할 수 있다.

- 상태: 특정 작업을 수행하는 지점을 나타낸다.
- 종료 상태: Flow 실행의 최종 도착점을 나타내는 상태로, Flow가 해당 상태에 도달하면 더 이상 진행되지 않고 실행이 종료된다. Job의 최종 결과는 Flow가 어떤 EndState로 끝났는지에 따라 결정된다.

**전이 조건 (ExitCode)**

분기 기준으로, Spring Batch Flow에서는 `ExitStatus`의 `exitCode` 필드가 다음 전이를 결정하는 핵심 조건이 된다. 기본으로 제공되는 `COMPLETED`, `FAILED` 외에도 직접 정의한 커스텀 `ExitStatus`를 정의해 세밀한 분기가 가능하다.

```java
ExitStatus{
    public static final ExitStatus COMPLETED = new ExitStatus("COMPLETED");
    public static final ExitStatus FAILED = new ExitStatus("FAILED");

    // ...

    // Flow의 전이 분기를 결정하는 핵심 필드다.
    private final String exitCode;

    public ExitStatus(String exitCode) {
        this(exitCode, "");
    }

    // ...
}
```

**전이 규칙 (Transition)**

`ExitCode` 조건에 따라 다음 상태로의 이동을 정의한다. 예를 들어, `ExitCode`가 `COMPLETED`면 A Step으로 이동하라 같은 조건부 이동 규칙을 정의한다.

## Flow 적용

이제 작업에 대해서 Flow를 적용해서 각 Step에서 작업 실패 시 바로 `resultSendingStep`으로 전이하도록 만들어보자. 기본적으로 `ExitCode`는 `COMPLETED`와 `FAILED`가 존재한다. 따라서 `COMPLETED`가 아니면 바로 `resultSendingStep`으로 전이하도록 다음과 같이 Flow를 구성해봤다.

```java
@Slf4j
@Configuration
@RequiredArgsConstructor
public class HistoryBatchConfig {
    private final PlatformTransactionManager transactionManager;
    private final JobRepository jobRepository;

    @Bean
    public Job orderRecoveryJob(
            Step historyStep,
            Step managementResetStep,
            Step historyBackupStep,
            Step resultSendingStep
    ) {

        return new JobBuilder("writeHistoryJob", jobRepository)
                .start(historyStep)
                .on("FAILED").to(resultSendingStep)
                .on("*").to(managementResetStep)

                .from(managementResetStep)
                .on("FAILED").to(resultSendingStep)
                .on("*").to(historyBackupStep)

                .from(historyBackupStep)
                .on("*").to(resultSendingStep)

                .end()
                .build();
    }
    // ...
}
```

이를 시각화하면 다음과 같다.

```
historyStep
 ├─ FAILED → resultSendingStep
 └─ COMPLETED → managementResetStep
        ├─ FAILED → resultSendingStep
        └─ COMPLETED → historyBackupStep
                └─ (SUCCESS/FAIL) → resultSendingStep
```

## 실행

Flow를 적용한 다음 실패 시 메일이 제대로 오는지 확인해봤다.

![image.png](/assets/images/development/spring-batch/25-12-09-large-scale-data-processing-with-spring-batch-with-flow/02.png)

실패한 Step과 실패 원인이 작성되어 전송된 것을 볼 수 있다.

---

Spring Batch 시리즈로 Spring Batch의 기본 개념과 대용량 데이터 처리 구성, 그리고 Flow를 이용한 분기 제어까지 알아보았다. 이 외에도 내결함성 기능을 활용해서 재시작이나 건너뛰기 처리 흐름을 구성하고 싶었지만 딱히 활용할만한 부분이 없어서 넘겼다.

Spring Batch를 활용하면서 굉장히 간단한 설정으로 복잡한 데이터 처리 로직을 빠르게 구성할 수 있다고 느꼈다. 참고로 일반적으로 배치 작업은 jar 파일을 만들어서 해당 파일을 실행시키는 방식으로 많이 처리한다고 한다. 이런 경우에는 메인 메서드의 코드를 다음과 같이 수정해야 한다.

```java
@SpringBootApplication
public class SpringBatchApplication {

    public static void main(String[] args) {
        System.exit(SpringApplication.exit(SpringApplication.run(SpringBatchApplication.class, args)));
    }

}
```

이렇게 수정 해야 배치 작업의 처리 도중 오류가 발생하면 0이 아닌 다른 종료 코드가 반환되기 때문에 외부 스케줄러로 실행할 때 문제가 생기면 적절한 프로세스를 거치도록 설계할 수 있다고 한다.

그리고 현재는 간단하게 이메일 첨부 파일로 보내도록 만들었지만, 용량이 많아지면 이 부분이 문제가 된다. 실제로 100만 건 데이터가 약 100MB정도 나와서 이메일로 보낼 때 오류가 발생하여 간단하게 100건의 데이터로 급격하게 줄였다. 따라서 해당 글은 이런 처리가 가능하다 정도로만 보는 게 좋을 것 같다.

이후에 여유가 된다면 Spring Batch에 관한 테스트 코드 작성법을 배워서 활용해보겠지만, 최근에는 Spring Security와 Kotlin에 눈이 가는지라 언제가 될 지는 잘 모르겠다.

# 참고자료

[**SMTP를 활용한 이메일 인증**](https://velog.io/@gkrdh99/email-verification-with-smtp)

[**죽음의 Spring Batch: 새벽 3시의 처절한 공포는 이제 끝이다.**](https://www.inflearn.com/course/%EC%A3%BD%EC%9D%8C%EC%9D%98-spring-batch/dashboard)
