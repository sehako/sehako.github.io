---
title: GraphQL 알아보기 (등장 배경, 타입 시스템, 도입 판단 기준)
categories:
  - 웹 API와 파일 처리
toc: true
toc_sticky: true
published: true
date: 2026-11-26

permalink: "/web-api/graphql/"
---

GraphQL은 2012년에 페이스북에서 개발하여 사용하고, 2015년에 오픈소스로 전환된 API 쿼리 언어입니다. 이번 글에서는 GraphQL의 등장 배경과 특징, 타입 시스템을 알아보고, 기업의 도입 사례를 바탕으로 도입 판단 기준을 정리해 보겠습니다.

# 등장배경

2012년 페이스북이 모바일 네이티브 애플리케이션 개발을 시작하기 전까지, iOS와 안드로이드 앱 모두 웹뷰를 통해 자신들의 모바일 웹을 보여주는 간단한 래퍼 역할일 뿐이었습니다.

이는 "한 번 작성되면 모든 곳에서 돌아가는" 이상적인 애플리케이션으로 보였지만 페이스북의 기능이 복잡해질수록 래퍼 역할을 하는 웹뷰는 성능저하와 충돌로 인한 장애 문제가 빈번해졌습니다.

이를 해결하기 위해 네이티브 앱을 개발하던 중 기존에 HTML(서버 사이드 렌더링) 방식으로만 제공되던 뉴스피드를 API 형태의 데이터로 제공해야 할 필요가 처음으로 생겼습니다.

페이스북은 RESTful 서버 리소스와 FQL(SQL과 유사한 페이스북의 API)을 검토했지만 다음과 같은 문제가 있었습니다.

- 앱은 데이터를 객체 그래프로 다루지만, 서버는 리소스 URL이나 조인 테이블을 기준으로 데이터를 제공했습니다.
- 연관된 데이터를 가져오려면 여러 번의 요청이나 복잡한 조인이 필요했습니다.
- 서버에서 데이터를 준비하는 코드와 클라이언트에서 이를 파싱하는 코드가 많이 필요했습니다.

이에 데이터 조회 방식을 서버가 아닌 제품 개발자와 클라이언트 앱의 관점에서 다시 설계했고, 그 결과 클라이언트의 쿼리가 응답의 형태를 결정하는 GraphQL이 만들어졌습니다.

## REST API의 문제

REST API는 데이터를 리소스 중심으로 보고, 서버에서 각각의 API에서 응답 형식에 정의한 형식에 따라 고정적으로 응답하기 때문에 과다 조회 또는 과소 조회 문제가 항상 존재합니다.

팔로워 시스템이 존재하는 서비스에서 `GET /api/v1/user/{user-id}`라는 API 를 제공하고 다음 데이터를 반환한다고 가정해보겠습니다.

{% include code-header.html %}
```json
{
  "id": 1,
  "name": "사용자 A",
  "profileImage": "/images/profile.png",
  "email": "user@example.com",
  "createdAt": "2026-01-01"
}
```

이 API를 호출하는 클라이언트가 원하는 데이터가 이메일 하나라면, 이는 클라이언트가 원하는 데이터보다 더 많이 조회된 과다 조회 문제가 발생합니다.

또한 클라이언트가 이 사용자의 `id`값만 원하는 과다 조회 상황에서, 해당 사용자의 팔로워 숫자를 보고 싶다면 과소 조회 문제도 발생하게 됩니다. 클라이언트에서 사용자의 `id`값을 가지고 팔로워의 숫자를 보여주는 API에 또 다시 요청을 보내야 하기 때문입니다.

그렇다고 각 클라이언트의 종류와 화면 구성에 모두 대응하기 위해 모든 상황에 해당하는 API를 제공한다면 설계 및 유지보수 비용이 증가하는 문제가 존재합니다.

# 특징

## 쿼리를 통한 데이터 형태 정의

기존의 REST API는 응답 구조를 확인하기 위해 명세를 살펴봐야 하지만 GraphQL은 작성한 쿼리 자체가 응답 데이터의 구조를 정의합니다.

클라이언트가 다음과 같이 요청을 보냈다고 가정해보겠습니다.

{% include code-header.html %}
```text
{
  post {
    title
    author {
      name
    }
  }
}
```

그러면 서버의 응답은 다음과 같습니다.

{% include code-header.html %}
```json
{
  "data": {
    "post": {
      "title": "Hello",
      "author": {
        "name": "Sehako"
      }
    }
  }
}
```

즉, 클라이언트에서 정의한 GraphQL 쿼리 구조가 서버에 그대로 반영되어 응답됩니다.

## 계층 구조

객체 간의 관계에 따라서 계층 구조를 이루어 필요한 데이터를 조회할 수 있습니다.

어떤 게시글 상세 데이터 조회 시, 작성자의 이름과 댓글 목록을 GraphQL로 조회하는 예시입니다.

{% include code-header.html %}
```graphql
{
  post {
    author {
      name
    }
    comments {
      content
      author {
        name
      }
    }
  }
}
```

이를 통해 각 클라이언트 컴포넌트가 필요로 하는 데이터의 구조를 GraphQL로 자연스럽게 표현할 수 있습니다.

## 강타입 시스템

쿼리의 각 계층은 특정 타입에 대응하며, 각 타입은 사용 가능한 필드의 집합을 정의합니다.

이는 SQL의 데이터베이스 테이블과 유사하며, 동일하게 쿼리 실행 전 구체적인 오류 메시지를 제공할 수 있게 합니다.

GraphQL을 제공하는 서버에서는 제공이 가능한 데이터와 타입을 스키마로 명시해야 합니다.

{% include code-header.html %}
```graphql
type Query {
    user(id: ID!): User
}

type User {
    id: ID!
    name: String!
    age: Int
}
```

이러한 상황에서 클라이언트가 `User` 타입의 `address` 라는 필드를 요청한다면 해당 쿼리가 유효하지 않다고 판단하여 오류를 응답합니다.

{% include code-header.html %}
```json
{
  "errors": [
    {
      "message": "Cannot query field \"address\" on type \"User\"."
    }
  ]
}
```

## 데이터 제공 규약 프로토콜

GraphQL은 표현 계층에 속한 데이터 제공 및 조회에 대한 프로토콜입니다. 따라서 GraphQL의 도입을 위해 기존 애플리케이션의 변경은 필요하지 않습니다.

어떤 프레임워크, 데이터베이스 구현체를 사용하든, 내부적으로 복잡한 비즈니스 로직이 존재하든 GraphQL을 도입하는데 아무런 문제가 없습니다.

## 자가 탐색 기능 제공 (Introspective)

GraphQL을 제공하는 서버에게 지원하는 타입이 무엇인지에 대한 쿼리를 제공합니다. 예를 들어 `User` 타입에 대한 스키마를 다음과 같이 조회할 수 있습니다.

{% include code-header.html %}
```graphql
{
    __type(name: "User") {
        name
        fields {
            name
        }
    }
}
```

해당 타입이 `id`, `name`, `age` 필드를 가졌다면 다음과 같이 응답을 합니다.

{% include code-header.html %}
```JSON
{
  "data": {
    "__type": {
      "name": "User",
      "fields": [
        { "name": "id" },
        { "name": "name" },
        { "name": "age" }
      ]
    }
  }
}
```

Spring으로 개발하는 REST API 역시 RestDocs나 Swagger UI 처럼 API 명세를 제공하는 외부 라이브러리가 존재하지만, 의존성 및 애플리케이션 설정이 요구됩니다.

GraphQL은 이러한 메타데이터 조회 기능이 자체적으로 포함되어 있으므로 이를 활용해 클라이언트에서 다음과 같은 기능을 구현할 수도 있습니다.

- API 스키마 자동 탐색
- API 문서 자동 생성
- 쿼리 작성 시 자동 완성
- 클라이언트 타입 및 코드 자동 생성
- 쿼리 작성 시 유효성 검증

## 불필요한 API 버전 관리

`/api/v1`, `/api/v2` 처럼 일반적인 API 관리 시 적용하는 버전 관리를 적용할 필요가 없습니다.

서버에서 최대한 단순하고 범용적인 형태로 데이터를 제공하고, 클라이언트는 필요한 타입과 필드에 대한 쿼리를 요청하기 때문입니다.

새로운 기능 추가 시 새로운 타입과 필드를 추가해도 기존 클라이언트는 아무런 영향이 없으며, 기존 기능을 deprecated로 지정하면서도 동작하도록 유지할 수 있습니다.

실제로 페이스북은 2012년에 GraphQL 개발 및 활용 이후 2015년인 3년 동안 동일한 버전의 GraphQL API를 지원하고 있다고 밝혔습니다.

# 스키마와 타입 시스템

GraphQL에서 스키마는 서버가 제공하는 데이터와 기능을 정의하는 계약이고, 이러한 스키마를 타입 시스템을 활용하여 정의 및 검증합니다.

예를 들어 사용자와 게시글에 관한 스키마를 정의한다면 다음과 같이 정의할 수 있습니다.

{% include code-header.html %}
```graphql
type User {
    id: ID!
    name: String!
    email: String!
    posts: [Post!]!
}

type Post {
    id: ID!
    title: String!
    content: String
    author: User!
}
```

`User`와 `Post`에 관한 타입을 정의하고, 해당 타입 내에 필드를 정의합니다. 이를 통해 각 타입이 가지는 필드와 해당 필드에 대한 null 허용 여부까지 정의할 수 있는 것을 볼 수 있습니다.

그리고 클라이언트가 요청을 보낼 수 있는 쿼리 또한 스키마에 정의해야 합니다.

{% include code-header.html %}
```graphql
type Query {
    user(id: ID!): User
    posts: [Post!]!
}
```

`posts` 에 정의된 `[]`는 리스트 형식을 의미합니다. 이렇게 정의한 타입과 쿼리를 기반으로 클라이언트는 다음과 같은 요청을 보내는 것입니다.

{% include code-header.html %}
```text
query {
    user(id: "1") {
        name
        posts {
            title
        }
    }
}
```

사용자의 이름과 사용자가 작성한 글에 대한 제목 리스트가 다음과 같이 응답됩니다.

{% include code-header.html %}
```json
{
    "data": {
        "user": {
            "name": "Sehako",
            "posts": [
                {
                    "title": "GraphQL 학습"
                },
                {
                    "title": "Spring Boot 개발"
                }
            ]
        }
    }
}
```

## 타입 시스템

API에서 사용할 수 있는 타입의 종류와 각 타입이 준수해야 하는 규칙을 정의합니다. GraphQL에는 6개의 Named Type과 2개의 Wrapping Type이 존재합니다.

## Root Operation Type

클라이언트가 어떤 형식으로 데이터를 제어하는지를 정의하는 타입입니다.

{% include code-header.html %}
```graphql
schema {
    query: Query
    mutation: Mutation
    subscription: Subscription
}

type Query {
    user(id: ID!): User
    posts: [Post!]!
}

type Mutation {
    createUser(input: CreateUserInput!): User!
    deleteUser(id: ID!): Boolean!
}

type Subscription {
    postCreated: Post!
}
```

3개의 Root Operation Type은 각각 다음을 의미합니다.

| 타입           | 역할               | 필수 여부 |
| ------------ | ---------------- | ----- |
| Query        | 데이터 조회의 진입점      | 필수    |
| Mutation     | 데이터 변경 등의 작업 진입점 | 선택    |
| Subscription | 이벤트 구독의 진입점      | 선택    |

GraphQL 연산의 시작점이라는 특별한 역할이 부여된 Object Type으로 보면 됩니다. 위 3개의 기본 이름을 스키마에서 그대로 사용하는 경우에는 `schema` 블록을 정의하지 않아도 됩니다.

## Named Type

### Scalar

더 이상 하위 필드로 탐색할 수 없는 단일 값을 표현하는 타입입니다. 기본적으로 5가지의 Scalar 타입을 제공합니다.

| 타입        | 설명            | 값의 예시     |
| --------- | ------------- | --------- |
| `Int`     | 부호 있는 32비트 정수 | `100`     |
| `Float`   | 배정밀도 부동소수점    | `3.14`    |
| `String`  | 문자열           | `"hello"` |
| `Boolean` | 논리값           | `true`    |
| `ID`      | 식별자           | `"123"`   |

`ID`는 식별자로 응답에서는 문자열로 치환됩니다. GraphQL을 구현하는 프레임워크나 라이브러리에서 Custom Scalar를 정의한 경우에는 이를 사용할 수도 있습니다.

{% include code-header.html %}
```graphql
scalar DateTime
scalar BigDecimal
scalar Long

type Product {
    id: ID!
    price: BigDecimal!
    createdAt: DateTime!
}
```

### Object

여러 필드로 구성된 객체를 표현하는 타입입니다. 앞서 살펴봤던 `User` 타입, `Post` 타입이 이에 해당합니다.

{% include code-header.html %}
```graphql
type User {
    id: ID!
    name: String!
    posts: [Post!]!
}

type Post {
    id: ID!
    title: String!
    author: User!
}
```

Object Type은 다른 Object Type을 참조할 수 있으므로 클라이언트는 이 관계를 따라 데이터를 탐색할 수 있습니다. 또한 Object Type의 필드는 인자를 받을 수 있습니다.

{% include code-header.html %}
```graphql
type User {
    id: ID!
    name: String!
    posts(limit: Int = 10): [Post!]!
}
```

### Enum

자바의 enum과 유사하게 허용 가능한 값의 범위를 제한합니다.

{% include code-header.html %}
```graphql
enum OrderStatus {
    PENDING
    PAID
    SHIPPED
    CANCELLED
}

type Order {
    id: ID!
    status: OrderStatus!
}

type Query {
    orders(status: OrderStatus): [Order!]!
}
```

### Interface

여러 Object Type이 공통으로 제공해야 하는 필드를 정의합니다. 정의된 인터페이스는 Object Type에서 `implements`로 구현합니다.

{% include code-header.html %}
```graphql
interface Payment {
    id: ID!
    amount: Int!
}

type CardPayment implements Payment {
    id: ID!
    amount: Int!
    cardCompany: String!
}

type BankPayment implements Payment {
    id: ID!
    amount: Int!
    bankName: String!
}
```

또한 이렇게 정의한 인터페이스를 쿼리에서도 사용할 수 있습니다.

{% include code-header.html %}
```graphql
type Query {
    payments: [Payment!]!
}
```

#### 필드 구체화

클라이언트에서 Inline Fragment로 구상 필드에 접근할 수 있습니다.

{% include code-header.html %}
```graphql
query {
    payments {
        id
        amount

        ... on CardPayment {
            cardCompany
        }

        ... on BankPayment {
            bankName
        }
    }
}
```

### Union

여러 Object Type 중 하나가 반환될 수 있음을 표현하는 추상 타입입니다. 어떤 검색 기능에서 사용자 또는 게시글이 검색 결과로 반환되는 경우에 Union Type을 사용하여 다음과 같이 표현할 수 있습니다.

{% include code-header.html %}
```graphql
union SearchResult = User | Post

type Query {
    search(keyword: String!): [SearchResult!]!
}
```

클라이언트에서는 `__typename` 메타 필드를 통해서 Union Type이 실제 반환한 타입 이름을 확인할 수 있습니다.

{% include code-header.html %}
```graphql
query {
    search(keyword: "GraphQL") {
        __typename

        ... on User {
            id
            name
        }

        ... on Post {
            id
            title
        }
    }
}
```

### Input Object

클라이언트가 서버에 쿼리를 전달할 때 입력해야 하는 복합적인 데이터의 구조를 정의합니다.

{% include code-header.html %}
```graphql
input CreateUserInput {
    name: String!
    email: String!
    password: String!
}

type User {
    id: ID!
    name: String!
    email: String!
}

type Mutation {
    createUser(input: CreateUserInput!): User!
}
```

이 경우 클라이언트는 정의된 Input Object Type에 따라서 쿼리를 전달해야 합니다.

{% include code-header.html %}
```graphql
mutation {
    createUser(input: {
        name: "Kim"
        email: "kim@example.com"
        password: "example123"
    }) {
        id
        name
    }
}
```

GraphQL은 기본 Object Type을 Output Object Type으로 간주하여 입력에만 사용되는 Input Object Type을 활용합니다.

## Wrapping Type

`List`와 `Non-Null`이라는 두 가지 Wrapping Type을 제공합니다.

### Non-Null

GraphQL의 타입은 기본적으로 Nullable입니다. 이때 `!` Non-null Type을 이용하면 각 타입에 대해서 Non-null을 설정할 수 있습니다.

{% include code-header.html %}
```graphql
type User {
    id: ID!
    name: String!
    nickname: String
}
```

Non-Null이 설정된 타입에 대해서 실제로 null이 반환되면 GraphQL 실행 오류가 발생하고, 오류가 가장 가까운 Nullable 상위 필드까지 전파되는 Null Propagation(Null Bubbling)이 발생합니다.

### List

특정 타입의 값이 여러 개 존재할 수 있음을 나타냅니다.

{% include code-header.html %}
```graphql
type User {
    tags: [String]
    posts: [Post!]!
}
```

Non-null Type과 List Type은 서로 조합해서 사용할 수 있습니다.

|타입|리스트 null 허용|요소 null 허용|
|---|---|---|
|`[String]`|O|O|
|`[String!]`|O|X|
|`[String]!`|X|O|
|`[String!]!`|X|X|

참고로 `[String!]!` 이라고 정의해도 null이 아닌 빈 리스트는 허용됩니다.

# 도입 사례 및 판단 기준

GraphQL은 클라이언트가 원하는 데이터를 한 번의 네트워크 요청으로 조회할 수 있는 API 쿼리 언어이므로 REST API에 비하면 많은 이점을 제공해주는 것 같이 보입니다.

하지만 무조건적인 도입은 오히려 복잡도를 과하게 늘릴 수 있습니다. 따라서 여러 기업의 도입 사례를 간략히 요약 정리하며 판단 기준을 세워보겠습니다.

## 도입 사례

- 깃허브

REST API가 가지는 문제와 함께, 엔드포인트에 대한 메타 정보 수집과 한 번의 요청으로 클라이언트가 원하는 데이터를 조회할 수 있기 때문에 도입을 결정했다고 밝혔습니다.

- 카카오페이지

iOS, Android, Web을 지원하며, 그중 Web에만 BFF(Backend For Frontend)를 적용하고 GraphQL을 사용했습니다.

여러 API 호출, 프론트엔드에 쌓이는 복잡한 계산 로직, CORS, 화면에 필요한 데이터만 받기 어려운 문제 등을 BFF에서 처리하고자 했습니다.

다만 Apollo Client의 id 기반 정규화 캐시가 조회 조건이 다른 결과를 덮어쓰는 문제를 겪어, 문서 기반 캐시를 사용하는 urql로 교체했습니다.

- 미도입 사례: 카카오페이

MSA 환경에서 여러 API를 조합하는 BFF를 WebFlux와 코루틴으로 구현했지만, GraphQL은 도입하지 않았습니다.

해당 서비스를 비롯한 많은 서비스에 모바일 화면만 존재하여 GraphQL을 도입할 이유가 없다고 판단했습니다.

- 넷플릭스

넷플릭스는 BFF 패턴을 활용했으나, 클라이언트별 BFF에서 데이터 조회 및 통합 로직이 중복되는 문제를 해결하기 위해 GraphQL을 도입하였습니다.

또한 중앙 GraphQL 서버에 Schema와 Resolver의 관리 책임이 집중되는 것을 방지하기 위해 도메인별 GraphQL API를 DGS(Domain Graph Service)로 구성하고, 이를 Federated Gateway에서 하나의 Schema로 통합하는 GraphQL Federation 구조를 구축하였습니다.

이에 따라 클라이언트와 Gateway 사이뿐만 아니라 Gateway와 DGS 사이에서도 GraphQL을 사용하는 구조가 형성되었습니다.

## 판단 기준

- 클라이언트나 화면마다 필요한 데이터가 다르면 도입을 검토합니다.
- 요청 형태가 다양하지 않다면 REST API(필요하면 REST BFF)를 우선합니다.
- 클라이언트별 BFF에 조회와 통합 로직이 중복되면 통합 GraphQL 계층을 검토합니다.
- 캐시와 상태 관리의 복잡도를 감당할 수 있는지 확인합니다.

일반적으로 여러 클라이언트를 고려하는 상황에서 GraphQL을 도입합니다. 하지만 클라이언트가 하나라도 도입할 수 있다고 생각합니다.

예를 들어 어떤 화면에서 10개의 API를 조회한다면, 이를 묶은 통합 API를 제공해 해결할 수 있습니다.

하지만 여러 화면에서 이 중 서로 다른 조합이 계속 필요하다면 조합마다 통합 API를 만들어야 합니다. 이때는 GraphQL로 필요한 조합을 요청해 API 증가 없이 네트워크 왕복을 줄일 수 있습니다. 다만 서버 내부의 조회 비용은 별도로 관리해야 합니다.

# 참고자료

[**GraphQL: A data query language**](https://engineering.fb.com/2015/09/14/core-infra/graphql-a-data-query-language)

[**Schemas and Types**](https://graphql.org/learn/schema)

[**GraphQL**](https://spec.graphql.org/September2026)

[**GitHub GraphQL API**](https://github.blog/developer-skills/github/the-github-graphql-api/)

[**카카오페이지는 BFF(Backend For Frontend)를 어떻게 적용했을까?**](https://kakaoentertainment-tech.tistory.com/92?category=524888)

[**WebFlux와 코루틴으로 BFF(Backend For Frontend) 구현하기**](https://tech.kakaopay.com/post/bff_webflux_coroutine/)

[**How Netflix Scales its API with GraphQL Federation (Part 1)**](https://netflixtechblog.com/how-netflix-scales-its-api-with-graphql-federation-part-1-ae3557c187e2)
