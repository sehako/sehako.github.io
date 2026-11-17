---
title: MySQL과 PostgreSQL (왜 요즘 PostgreSQL이 많이 선택되는가)

categories:
  - 개발 노트

toc: true
toc_sticky: true
published: true

date: 2026-11-16

permalink: "/dev-notes/mysql-and-postgresql/"
---

대학과 SSAFY 모두 데이터베이스 수업과 프로젝트에서 MySQL을 주로 사용했습니다.

그런데 최근에는 AI 에이전트로 개발을 하면 PostgreSQL을 주로 추천합니다. 또한 2025년 스택 오버플로우 개발자 설문에서 지난 1년간 개발에 사용한 데이터베이스로 PostgreSQL을 선택한 응답자는 전체의 55.6%(직업 개발자는 58.2%)로, MySQL(각각 40.5%, 39.6%)을 앞섰습니다. 이 글에서는 왜 그동안 MySQL이 선택되었고, 요즘은 왜 PostgreSQL이 점차 사용되는지 정리해 보겠습니다.

# 왜 MySQL을 많이 사용했는가

MySQL은 1995년에 출시된 RDBMS이며, 2000년에 GPL 라이선스를 채택하며 오픈소스로 전환했습니다.

이 시기에는 Linux + Apache + PHP를 통해 웹 서비스를 구축하는 것이 일반적이었으며, 이러한 상황에서 유료 라이선스에 주력하는 Oracle을 선택하는 것은 소규모 팀에는 부담이었습니다.

>**GPL이란?**
>
>GPL은 GNU General Public License의 약자로, 소프트웨어를 자유롭게 사용, 수정, 공유할 수 있도록 허용하는 오픈소스 라이선스입니다.

>**Oracle Database 10g Express Edition**
>
>오라클도 2005년에 무료 버전을 공개했지만, 이 버전은 당시 1코어 CPU, 1GB 메모리, 4GB의 저장공간 등 제약이 많았습니다.
>
>반면 MySQL은 이러한 제한이 없는 완전한 오픈소스였으며, 이를 활용해서 상용 서비스를 만들어도 유료 라이선스를 구매하지 않아도 된다는 이점이 있었습니다.

이러한 상황이었기 때문에 소규모 팀에서 도입에 부담이 없는 MySQL을 채택하였으며, 시간이 지나면서 Linux, Apache, MySQL, PHP의 LAMP 스택이 웹 개발에서 널리 쓰이는 구성이 되었습니다.

이렇게 웹 개발 분야에서 주로 MySQL이 채택되면서 다음과 같은 순환이 이루어졌습니다.

{% include code-header.html %}
```text
MySQL 사용 확대
-> 책, 강의, 예제 코드 및 개발 도구 증가
-> 대학 교육 및 웹 개발 입문 프로젝트에서 MySQL 채택
-> 새로운 개발자들이 MySQL을 선택
```

## 대학에서 MySQL을 채택하는 이유

대학교 수업은 특정 RDBMS 제품을 학습하는 것이 아닌 관계형 모델, SQL, 정규화, 인덱스, 트랜잭션, 동시성 제어 등 RDBMS에서 공통적으로 적용되는 원리를 가르치는 데 목적이 있습니다.

이러한 상황에서 MySQL을 사용하면 다음과 같은 이점이 있다고 정리할 수 있습니다.

- **무료 사용 가능**: 오픈소스라 학교 서버나 학생 개인 PC에 라이선스 비용 없이 설치할 수 있습니다. 수업 목적이 제품 학습이 아니므로, 비용 부담 없이 SQL과 트랜잭션 같은 공통 원리를 실습할 수 있으면 충분합니다.
- **교육 환경 표준화**: 설치가 간단하고 자료가 풍부해서 수강생 전원이 같은 환경에서 실습하기 쉽습니다. 교재, 강의 자료, 예제가 MySQL 기준으로 많이 만들어져 있어 교수와 학생 모두 환경 문제로 시간을 덜 쓰게 됩니다.
- **프로젝트 확장**: 수업에서 배운 MySQL을 졸업 작품이나 팀 프로젝트의 웹 서비스에 그대로 쓸 수 있습니다. 앞서 본 LAMP 스택 이후의 자료와 개발 도구가 많아, 학습한 환경을 별도 전환 없이 프로젝트까지 이어갈 수 있습니다.

정리하자면, MySQL이 대학에서 채택된 것은 다른 RDBMS보다 기술적으로 뛰어나서라기보다, 무료로 쓸 수 있고 도입이 쉬워 LAMP 스택과 함께 웹 개발에서 널리 쓰이게 된 데서 출발한 결과입니다. 이후 늘어난 교재와 예제를 따라 대학 교육과 입문 프로젝트도 MySQL을 사용하게 되었습니다.

# AI 에이전트 개발 시대에 점점 선택되는 PostgreSQL

PostgreSQL은 1986년 UC Berkeley의 연구 프로젝트(POSTGRES)로 시작했습니다. 1994년에 SQL을 지원하는 Postgres95로 이어졌고, 1996년에 PostgreSQL이라는 이름으로 변경되었습니다.

최근에는 AI 에이전트가 개발 환경에 널리 사용되면서, 시스템의 구축 자체는 AI가 수행하고, 사람은 시스템이 가져야 하는 정책(스펙)과 각 시스템 간의 연계 방식에 집중하는 방식으로 개발 문화가 변하고 있습니다.

이러한 상황에서 AI 에이전트로 초기 프로젝트를 만들면 제 경험상 에이전트는 데이터베이스로 PostgreSQL을 추천하는 경우가 많았습니다. DB 운영 자동화 업체 NeverBlink의 [글](https://neverblink.ai/blog/postgresql-everyone-uses-postgres-but-why)도 에이전트가 DB를 지정받지 않으면 Postgres를 고른다고 쓰고 있습니다. 그 이유를 살펴보겠습니다.

## ORDBMS (Object-Relational Database Management System)

ORDBMS는 전통적인 관계형 데이터 모델을 기반으로 객체지향적인 개념을 일부 도입한 DBMS를 의미합니다.

PostgreSQL은 이러한 ORDBMS로, MySQL과 비교해 다음과 같은 확장 기능을 제공합니다.

- **사용자 정의 타입(User-Defined Types)**: `CREATE TYPE`으로 ENUM, 복합 타입, 범위 타입 등을 이름 있는 독립된 타입으로 정의해 여러 테이블에서 재사용하고, `CREATE DOMAIN`으로 기존 타입에 제약 조건을 부여할 수 있습니다. 또한 확장 인터페이스를 통해 새로운 기본 타입을 등록할 수 있으며, PostGIS의 `geometry`, pgvector의 `vector`가 대표적인 사례입니다.
- **복합 타입(Composite Types)**: 여러 필드를 묶어 하나의 데이터 타입으로 정의할 수 있습니다. PostgreSQL은 복합 타입 외에도 배열과 `JSONB` 등 구조화된 데이터를 저장하는 다양한 타입을 제공합니다.
- **테이블 상속(Table Inheritance)**: `INHERITS`로 부모 테이블의 컬럼을 자식 테이블이 상속받을 수 있습니다. 다만 기본 키, 고유 제약 조건, 외래 키 등이 자동으로 상속되지 않는 제약이 있습니다. 과거에는 파티셔닝에도 활용되었지만, 현재 일반적인 파티셔닝에는 선언적 파티셔닝을 사용하는 것이 권장됩니다.
- **사용자 정의 함수 및 연산자**: `CREATE FUNCTION`, `CREATE OPERATOR`로 함수와 연산자를 추가할 수 있습니다. pgvector의 거리 연산자 `<->`가 대표적인 사례입니다. MySQL도 사용자 정의 함수는 지원합니다.

즉, PostgreSQL의 특징은 단순히 다양한 데이터 타입을 제공하는 것이 아니라, 새로운 타입과 이를 처리하는 함수, 연산자, 인덱스 접근 방식까지 확장할 수 있는 구조를 갖췄다는 점입니다. ITWorld 기사도 PostgreSQL이 SQL 표준 준수, 고급 기능, 확장성으로 MySQL과의 격차를 좁혔다고 평가합니다.

## pgvector

pgvector는 PostgreSQL에 벡터 저장과 유사도 검색 기능을 추가하는 오픈소스 확장입니다. 앞서 본 확장 구조가 실제로 쓰이는 대표적인 사례입니다.

AI 애플리케이션은 텍스트나 이미지를 임베딩(숫자 벡터)으로 바꿔 저장하고, 질문과 가까운 벡터를 찾는 방식(예: RAG)을 자주 사용합니다. 보통은 이를 위해 별도의 벡터 DB를 두지만, pgvector를 쓰면 기존 PostgreSQL 안에서 처리할 수 있습니다. The New Stack의 [글](https://daily.dev/posts/why-developers-are-betting-on-postgres-for-ai-pefuo5sjs)도 이를 AI 애플리케이션에서 PostgreSQL이 선택되는 이유로 듭니다.

- **`vector` 타입**: 사용자 정의 타입으로 등록된 벡터 컬럼입니다. `vector(1536)`처럼 차원 수를 지정합니다.
- **거리 연산자**: 사용자 정의 연산자로 추가된 `<->`(L2 거리), `<=>`(코사인 거리), `<#>`(내적) 등을 `ORDER BY`에 사용해 가까운 순으로 조회합니다.
- **벡터 인덱스**: HNSW, IVFFlat 인덱스로 근사 최근접 이웃 검색 속도를 높입니다.

{% include code-header.html %}
```sql
CREATE EXTENSION vector;

CREATE TABLE document (
    id        BIGSERIAL PRIMARY KEY,
    content   TEXT,
    embedding vector(1536)
);

SELECT id, content
FROM document
ORDER BY embedding <=> '[0.1, 0.2, ...]'
LIMIT 5;
```

이 방식의 장점은 벡터와 일반 데이터를 같은 DB에서 함께 다룰 수 있다는 것입니다. 예를 들어 `WHERE user_id = 1` 같은 조건과 벡터 검색을 하나의 SQL로 결합할 수 있고, 트랜잭션과 백업 체계도 그대로 사용합니다.

다만 pgvector는 원본 데이터가 바뀌어도 벡터를 자동으로 갱신하지 않으므로, 임베딩을 다시 만드는 별도 처리가 필요합니다.

## 클라우드와 개발 플랫폼 생태계

앞서 살펴본 확장성은 PostgreSQL을 선택할 기술적 이유가 되지만, 실제 프로젝트에서 DBMS를 선택할 때는 개발과 운영의 편의성도 중요합니다. 이와 관련해 PostgreSQL을 핵심 기술로 채택하여 개발과 운영 기능을 통합한 플랫폼들이 발전했습니다.

- **Supabase**: PostgreSQL을 중심으로 인증(Auth), 스토리지(Storage), 실시간 처리(Realtime), 자동 생성 API 등을 통합한 백엔드 개발 플랫폼입니다([문서](https://supabase.com/docs/guides/getting-started/architecture)). PostgreSQL을 활용하면서 별도의 백엔드 인프라를 구축하는 부담을 줄일 수 있습니다.
- **Neon**: PostgreSQL 기반 서버리스 데이터베이스 플랫폼([저장소](https://github.com/neondatabase/neon))으로, 컴퓨팅과 스토리지를 분리하여 자동 확장, 유휴 시 컴퓨팅 중지(Scale to Zero), 데이터베이스 브랜칭 등을 제공합니다. 특히 브랜칭은 개발 및 테스트 환경을 독립적으로 구성하는 데 활용할 수 있습니다.

이러한 플랫폼을 활용하면 소규모 팀에서도 PostgreSQL의 다양한 기능을 활용하면서 개발과 운영에 필요한 부담을 줄일 수 있습니다.

이처럼 PostgreSQL의 기술적 확장성과 개발 플랫폼 생태계가 결합하면서 도입 장벽이 낮아졌고, 그 과정에서 축적된 플랫폼과 기술 문서, 코드 예제 등이 AI 코딩 에이전트의 PostgreSQL 추천에도 영향을 미치지 않을까 개인적으로 추측합니다.

# 정리

MySQL 대신에 PostgreSQL이 주로 채택되는 이유는 크게 세 가지로 볼 수 있습니다.

- **도입 부담이 적다**: Oracle과 달리 무료 오픈소스라 소규모 팀도 비용 부담 없이 도입할 수 있습니다. 다만 MySQL도 같은 조건이므로, 선택지에 들어오기 위한 전제 조건에 가깝습니다.
- **여러 요구사항을 충족할 수 있다**: ORDBMS의 확장 구조 덕분에 JSONB, 벡터 검색(pgvector) 같은 새로운 요구를 DBMS 하나로 처리할 수 있습니다. 별도 시스템을 추가하지 않아도 되므로 인프라 부담이 줄어듭니다. MySQL도 JSON 등을 지원하지만, 새로운 타입과 연산자까지 확장할 수 있는 폭에서 차이가 있습니다. ITWorld 기사도 MySQL은 벡터 검색 같은 최신 기능 대응이 늦다고 지적합니다.
- **운영 부담이 낮고 플랫폼 생태계가 발달했다**: Supabase, Neon 같은 PostgreSQL을 핵심으로 삼은 플랫폼 덕분에 소규모 팀도 적은 운영 부담으로 도입할 수 있습니다.

이러한 요인들은 MySQL 때와 같은 순환을 만듭니다.

{% include code-header.html %}
```text
PostgreSQL 사용 확대
-> 자료, 예제, AI 에이전트의 추천 증가
-> 새로운 프로젝트에서 PostgreSQL 선택
```

결국 MySQL은 무료이고 도입이 쉬워 웹 개발의 표준이 되었고, PostgreSQL은 같은 전제 위에서 확장성을 제공하여 여러 요구사항을 PostgreSQL로 커버할 수 있는 폭을 넓혀 AI 시대의 기본 선택지가 되고 있습니다.

그래도 단순한 트랜잭션 위주의 웹 서비스라면 MySQL도 여전히 충분한 선택입니다. AWS의 RDS 문서도 JSON 처리, 복잡한 쿼리, 사용자 정의 확장이 필요하면 PostgreSQL을, 기존 도구와의 폭넓은 호환성이나 단순한 트랜잭션 위주라면 MySQL을 선택하도록 안내합니다. 또한 이미 운영 중인 시스템이라면 전환 비용도 고려해서 PostgreSQL 전환을 검토해야 합니다.

# 참고 자료

[**Everyone Uses PostgreSQL... But Why?**](https://neverblink.ai/blog/postgresql-everyone-uses-postgres-but-why)

[**Why developers are betting on Postgres for AI**](https://daily.dev/posts/why-developers-are-betting-on-postgres-for-ai-pefuo5sjs)

[**Supabase Docs: Architecture**](https://supabase.com/docs/guides/getting-started/architecture)

[**Neon: Serverless Postgres**](https://github.com/neondatabase/neon)

[**Amazon RDS: Choosing your database engine**](https://docs.aws.amazon.com/AmazonRDS/latest/gettingstartedguide/choosing-engine.html)

[**2025 Stack Overflow Developer Survey: Technology**](https://survey.stackoverflow.co/2025/technology)

[**MySQL Retrospective - The Early Years**](https://blogs.oracle.com/mysql/post/mysql-retrospective-the-early-years)

[**Oracle Database Express Edition Licensing Information, 10g Release 2 (10.2)**](https://docs.oracle.com/cd/B25329_01/doc/license.102/b25456.pdf)

[**PostgreSQL Documentation: A Brief History of PostgreSQL**](https://www.postgresql.org/docs/16/history.html)

[**마이SQL 30주년, 왕관은 내려놨지만 여전한 존재감**](https://www.itworld.co.kr/article/3984622/마이sql-30주년-왕관은-내려놨지만-여전한-존재감.html)

[**pgvector로 PostgreSQL에 벡터 검색 더하기**](https://daleseo.com/pgvector/)
