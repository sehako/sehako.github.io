---
title: Spring Boot에서 Flyway로 데이터베이스 마이그레이션 관리하기

categories:
  - Spring

toc: true
toc_sticky: true
published: true

date: 2026-09-02
---

# Flyway

데이터베이스 스키마의 변경을 마이그레이션 파일로 관리하는 툴이다. 이를 사용하면 다음과 같은 장점이 존재한다.

- DB 변경 이력 관리 가능

Flyway에서는 일반적으로 테이블 생성, 컬럼 추가, 인덱스 추가, 초기 데이터 삽입 같은 DB 변경을 SQL 마이그레이션 파일로 관리한다.

```
src/main/resources/db/migration
 ├── V1__create_member_table.sql
 ├── V2__create_order_table.sql
 └── V3__add_order_status.sql
```

이것이 자연스럽게 Git 변경 사항으로 남기 때문에 데이터베이스 변경 사항도 추적할 수 있게 된다.

- 모든 환경에 같은 순서로 반영

로컬 환경, 개발 환경, 운영 환경 등 여러 환경의 DB의 현재 상태는 다를 수 있다.

```
local:   V1, V2, V3 적용됨
dev:     V1, V2 적용됨
prod:    V1 적용됨
```

이를 위해 Flyway는 DB에 현재 상태를 관리하는 테이블인 `flyway_schema_history` 테이블을 두고, 이미 적용된 마이그레이션은 건너뛰고, 아직 적용되지 않은 마이그레이션만 순서대로 실행한다.

- 마이그레이션 중복 적용 및 변경 감지

Flyway는 `flyway_schema_history`를 통해 이미 적용된 Versioned Migration을 다시 실행하지 않는다. 또한 적용 당시 파일의 체크섬을 저장하고, 이후 파일 내용이 변경되면 체크섬 불일치로 검증 오류를 발생시킨다.

| version | script | checksum | success |
| --- | --- | --- | --- |
| 1 | V1__init.sql | 123456789 | true |
| 2 | V2__create_order.sql | 987654321 | true |

>
>
>
> **파일 체크섬**
>
> 파일 내용으로부터 계산한 값이다. Flyway는 적용 당시 저장된 체크섬과 현재 마이그레이션 파일의 체크섬을 비교하여 파일 내용의 변경 여부를 검증한다.
>

따라서 이미 적용된 기존 마이그레이션 파일을 수정하는 것이 아니라, 다음 버전의 마이그레이션 파일을 추가하여 변경 사항을 적용하는 것이 기본적인 방식이다.

- 배포 자동화

Spring Boot 에서 Flyway 모듈을 classpath에 추가하면 애플리케이션 시작 시 마이그레이션을 자동 실행할 수 있다. 기본 마이그레이션 경로는 다음과 같다.

```
src/main/resources/db/migration
```

여기에 마이그레이션 정보를 남겨두면 애플리케이션 실행 시 Flyway가 SQL을 적용하고, 적용 이력을 DB에 남긴다.

Flyway를 사용하면 개발자는 DB 변경 사항을 마이그레이션으로 작성하고, Flyway가 아직 적용되지 않은 마이그레이션을 찾아 순서대로 실행하도록 관리할 수 있다.

Flyway가 마이그레이션 파일과 `flyway_schema_history`를 비교하여 아직 적용되지 않은 마이그레이션을 찾아 실행하기 때문이다.

## 운영 환경의 스키마 마이그레이션 주의 사항

운영 환경에서 기존 애플리케이션과 새로운 스키마의 호환성을 유지하면서 점진적으로 스키마를 변경해야 한다면 Expand–Contract 전략을 고려할 수 있다.

특히 기존 데이터가 존재하는 테이블의 컬럼 추가, 변경, 삭제와 같은 작업에서 유용하다.

1. 컬럼 추가: NULLABLE 컬럼으로 추가한다. 기존 데이터가 있는 운영 테이블에 바로 NOT NULL 컬럼을 추가하는 것은 피한다.
2. 애플리케이션 호환 처리: 새 컬럼이 아직 NULL일 수 있는 상태를 애플리케이션에서 처리할 수 있어야 한다.
3. 데이터 백필: 기존 데이터에 대해 정책을 정한 뒤 값을 채운다. 기본 값, 기존 데이터 기반 계산 로직, 배치 작업 등을 활용해 백필한다.
4. 제약 조건 추가: 필요한 NOT NULL, UNIQUE 등의 제약 조건과 INDEX 등을 후속 마이그레이션으로 적용한다.

대용량 테이블에서는 Flyway 마이그레이션 안에서 대량 UPDATE를 한 번에 수행하기보다, 배치 작업 등을 통해 chunk 단위로 백필하고 Flyway는 스키마 변경과 최종 제약 조건 추가를 담당하게 분리하는 것이 안전하다.

>
>
>
> **데이터 백필**
>
> 새로 추가되거나 비어 있는 컬럼에 대해, 기존 데이터에도 의미 있는 값을 채워 넣는 작업
>

## 파일 이름 규칙

마이그레이션 파일 이름은 다음 규칙을 가지고 있다.

```
V<버전>__<설명>.sql
R__<설명>.sql
--- 예시 ---
V001__create_member_table.sql
V002__add_member_status_column.sql
V003__create_order_table.sql
R__refresh_member_summary_view.sql
```

- V: 각 버전에 대한 마이그레이션이 한 번 만 적용 되는 것이다. Flyway는 버전에 적힌 숫자를 오름차순으로 실행하기 때문에, 각 버전 이름은 고유해야 하며 최신 버전이 가장 큰 숫자여야 한다.
- R: 체크섬이 바뀔 때마다 다시 실행된다. 주로 뷰, 프로시저, 함수 같은 현재 정의가 중요하고 재생성 가능한 객체에 사용한다. 따라서 이 파일의 내용은 재실행 가능하게 작성해야 한다.

# Spring Boot에서 Flyway 사용

### 의존성 추가

Flyway를 사용하려면 Flyway 의존성과 사용하는 DB에 필요한 Flyway DB 전용 모듈을 추가한다. PostgreSQL, MySQL 등의 JDBC Driver는 애플리케이션의 DB 연결을 위해 별도로 필요하다.

```kotlin
dependencies {
  implementation("org.flywaydb:flyway-core")
  runtimeOnly("org.flywaydb:flyway-database-postgresql")
}
```

참고로 Spring Boot 4 이상 부터는 spring-boot-starter-flyway를 사용해야 한다.

```kotlin
implementation("org.springframework.boot:spring-boot-starter-flyway")
runtimeOnly("org.flywaydb:flyway-database-postgresql")
```

### application.yml 설정

```yaml
spring:
	# datasource: ...
	# jpa: ...
  flyway:
    enabled: true
    # src/main/resources/db/migration 경로 지정
    locations: classpath:db/migration
    validate-on-migrate: true
    clean-disabled: true
    baseline-on-migrate: false
    out-of-order: false
    validate-migration-naming: true
```

이렇게 설정하면 이제 `src/main/resources/db/migration`에 마이그레이션 파일들을 추가했을 때 Flyway가 자동으로 관리한다.

### FlywayConfigurationCustomizer

`application.yml`에서 제공하는 설정만으로 부족하거나 Flyway 설정을 프로그래밍 방식으로 구성해야 한다면 `FlywayConfigurationCustomizer`를 사용할 수 있다.

```kotlin
@Configuration
class FlywayConfig {

  @Bean
  fun flywayConfigurationCustomizer(): FlywayConfigurationCustomizer {
    return FlywayConfigurationCustomizer { configuration: FluentConfiguration ->
      configuration
		    .baselineOnMigrate(false)
		    .validateOnMigrate(true)
		    .cleanDisabled(true)
		    .outOfOrder(false)
    }
  }
}
```

또한 Flyway의 실행 전략 자체를 제어하려면 `FlywayMigrationStrategy`, Flyway 전용 DataSource가 필요하다면 `@FlywayDataSource`를 사용할 수 있다.

## Flyway + JPA / Hibernate 주의점

Flyway와 JPA / Hibernate를 같이 사용한다면 역할을 다음과 같이 나눠야 한다.

- Flyway: DB 스키마 변경 및 마이그레이션 관리
- JPA / Hibernate: 엔티티와 테이블 매핑, DB 스키마 검증

### ddl-auto

가장 중요한 주의점으로, `ddl-auto`를 통한 Hibernate의 자동 DDL 변경 기능은 운영 환경에서는 지양해야 한다. `ddl-auto` 옵션은 다음과 같이 존재한다.

| 옵션 | 동작 |
| --- | --- |
| `none` | Hibernate가 스키마 생성/변경/검증을 하지 않음 |
| `validate` | 엔티티와 DB 스키마가 맞는지만 검증 |
| `update` | 엔티티 기준으로 DB 스키마를 자동 변경 |
| `create` | 애플리케이션 시작 시 기존 스키마를 삭제하고 새로 생성 |
| `create-drop` | 시작 시 생성, 종료 시 삭제 |

Flyway를 사용한다면 `validate` 또는 `none`을 사용해서 엔티티 변경에 따른 데이터베이스 스키마 변경을 방지해야 한다.

그리고 엔티티의 변경이 이루어지면 반드시 마이그레이션 파일도 추가하여 데이터베이스 스키마의 불일치가 없도록 관리하는 것이 중요하다

### 엔티티 매핑과 스키마 변경의 책임 분리

**JPA 어노테이션만으로 DB가 변경된다고 생각하지 않기**

`ddl-auto=validate` 또는 `none`으로 설정했다면, `@Column`, `@Index`, `@UniqueConstraint` 등의 엔티티 매핑을 변경하는 것만으로 실제 DB 스키마가 변경되지는 않는다. DB 변경은 별도의 Flyway migration으로 작성해야 한다.

**네이밍 전략 차이 주의**

JPA의 naming strategy에 따라 엔티티 필드명과 실제 DB 컬럼명이 다르게 해석될 수 있다. 혼동을 줄이기 위해 `@Table`, `@Column`, `@JoinColumn` 등으로 테이블명과 컬럼명을 명시하는 것이 안전하다.

**기존 데이터가 있는 테이블 변경 주의**

기존 데이터가 존재하는 테이블의 변경은 앞서 설명한 Expand–Contract 전략을 고려한다.

**Enum 변경 시 데이터 마이그레이션 고려**

Enum은 `ORDINAL`보다 `STRING` 사용이 안전하다. 또한 enum 이름이나 값이 변경되면 코드만 수정하는 것이 아니라, DB에 저장된 기존 데이터도 함께 마이그레이션해야 한다.

**ID 생성 전략과 DB 설정 일치**

`@GeneratedValue`의 `IDENTITY`, `SEQUENCE` 전략은 실제 DB의 auto increment, sequence 설정과 일치해야 한다. 특히 sequence 이름, allocation size, increment 값이 어긋나면 ID 생성 문제가 발생할 수 있다.

### 엔티티 변경과 마이그레이션 적용 순서 고려

무중단 배포 전략을 가진 멀티 인스턴스 환경에서 마이그레이션으로 다음과 같은 문제가 발생할 수 있다.

1. 기존 코드: old_column 사용
2. migration: old_column 삭제
3. 아직 기존 코드가 떠 있는 인스턴스가 old_column 조회
4. 장애 발생

컬럼 추가 상황에서도 추가된 컬럼에 대한 데이터 처리 정책이나 기존 코드에 추가된 컬럼을 반영해야 한다.

따라서 마이그레이션과 엔티티 변경의 적용 순서에 대한 적절한 처리를 하도록 고려하는 것이 중요하다.

# 참고 자료

[**Flyway Migrations**](https://documentation.red-gate.com/flyway/flyway-concepts/migrations)

[**Versioned migrations**](https://documentation.red-gate.com/flyway/flyway-concepts/migrations/versioned-migrations)

[**Repeatable migrations**](https://documentation.red-gate.com/flyway/flyway-concepts/migrations/repeatable-migrations)

[**Flyway schema history table**](https://documentation.red-gate.com/flyway/flyway-concepts/migrations/flyway-schema-history-table)

[**Spring Boot - Database Initialization**](https://docs.spring.io/spring-boot/how-to/data-initialization.html)

[**Hibernate ORM User Guide**](https://docs.hibernate.org/orm/current/userguide/html_single/)
