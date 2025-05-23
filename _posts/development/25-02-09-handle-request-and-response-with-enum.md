---
title: enum 객체로 요청과 응답을 처리하는 방법

categories:
  - Spring Boot Tip

toc: true
toc_sticky: true
published: true
 
date: 2025-02-09
last_modified_at: 2025-02-09
---

# 카테고리 관리 방식

최근 프로젝트에서 한 번도 해보지 못한 인프라 부분을 맡아서 이번 주는 간단한 글을 가져왔다. 글 작성을 위해 멤버십이라는 카테고리가 존재하는 서비스를 개발한다고 가정해보자. 그리고 사용자는 free, basic, preminum 셋 중 하나의 멤버십만 가질 수 있고, 추가적인 중복은 안된다.

## 리스트 테이블 방식

이를 이상적인 ERD 구조를 설계하여 해결하려고 한다면 다음과 같을 것이다.

![ERD 예시 1](/assets/images/handle-request-and-response-with-enum_01.png)

나는 개인적으로 이 방법이 정석이라고 생각한다. 하지만 만약에 지금 처럼 멤버십이 사실상 고정되어 있고, 앞으로의 서비스에서 추가되지 않거나 추가되는 사항이 미미하다면 굳이 테이블의 연관관계를 맺어서 관리하는 것이 손해가 될 수 있다. 

그 이유는 특정 멤버십의 사용자를 조회하기 위해서는 조인을 수행해야 하기 때문에 일반적으로 테이블 하나의 컬럼에 대한 조건을 설정하는 것 보다 성능이 더 떨어지게 될 것이다. 

또한 이 두 테이블을 JPA 엔티티로 표현하고 연관관계를 맺으면 다음과 같은 구조가 완성된다. 변수 타입은 귀찮아서 명시를 안했으므로 변수명에만 집중해서 보자

```java
@Entity
@Table(name = "membership")
public class Membership {
    private Long id;
    private String name;
}
```

```java
@Entity
@Table(name = "user")
public class Category {
    private Long id;
    private String email;
    private String password;
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "membership_id")
    private Membership membership;
}
```

이때 JPA는 엔티티를 조회할 때 불필요한 쿼리(N + 1 문제)를 방지하기 위해서 연관관계를 가지는 엔티티를 지연 로딩 하는 것이 사실상의 표준이다. 

그 말은 즉 사용자 10명이 있다고 가정했을 때 사용자를 우선 10명 조회하고 각 사용자의 반복문을 돌면서 특정 멤버십의 사용자만 필터링 한다면, 10개의 추가 쿼리가 데이터베이스에 조회되는 것이다. 물론 이를 해결하기 위해서 QueryDSL의 FETCH JOIN 같은 방법이나 JPQL등을 사용하여 애초에 특정 멤버십만 해당되는 사용자만 조회하는 방법도 있다. 

## 카테고리 컬럼 방식

따라서 숫자가 절대적으로 작고, 추가 사항이 필요가 없으면 굳이 이런 식으로 ERD를 구성하여 서비스를 개발할 필요가 없다. 단지 사용자 테이블에 멤버십이라는 컬럼 하나를 두는 것이 더욱 간편하고 성능도 더 잘 나올 것이다.

![ERD 예시 2](/assets/images/handle-request-and-response-with-enum_01.png)

이렇게 ERD를 구성하면 엔티티도 하나만 필요하게 되고, 연관관계도 맺을 필요가 없다.

```java
@Entity
@Table(name = "user")
public class Category {
    private Long id;
    private String email;
    private String password;
    private String membership
}
```

이 두 방식의 장단점을 정리하면 다음과 같다.

| 방식 | 장점 | 단점 |
|------|------|------|
| **리스트 테이블 방식** | - 정규화된 데이터 모델로 확장성이 높음 <br> - 특정 멤버십을 조회할 때 명확한 쿼리 가능 | - JOIN 필요로 인해 성능 저하 가능 <br> - N+1 문제 발생 가능 |
| **카테고리 컬럼 방식** | - 단순한 테이블 구조로 성능이 우수함 <br> - 불필요한 JOIN을 피할 수 있음 | - 멤버십이 추가될 경우 코드 수정 필요 <br> - 정규화되지 않아 데이터 무결성 유지가 어려움 |

### enum을 활용한 카테고리 제한

이렇게 멤버십 같은 고정된 요소들은 테이블간 관계를 맺는 방식이 아닌 추가 컬럼을 두는 방식을 선택할 수 있다는 것을 알았다. 이때 위 방식처럼 문자열 형식으로 멤버십을 관리하면 문제가 발생할 수 있다. 

그 이유는 클라이언트에서 잘못 전송한 멤버십에 대해서 서버에서 알 길이 없기에 검증하는 로직을 추가로 작성해야 한다. 하지만 `enum` 타입을 사용하여 요청을 받으면 사용자의 멤버십에 대해서 클라이언트에게 3개의 멤버십 타입만을 강제할 수 있게 된다.

```java
public enum Membership {
    FREE, BASIC, PREMIUM
}
```

그리고 이에 맞춰서 엔티티 구조 또한 다음과 같이 변경하면 된다.

```java
@Entity
@Table(name = "user")
public class Category {
    private Long id;
    private String email;
    private String password;
    @Enumerated(value = EnumType.STRING)
    private Membership membership
}
```

이러면 JPA에서 `Membership`에 해당되는 `enum`의 이름 문자열을 INSERT 쿼리를 작성할 것이다.

그럼 클라이언트에서는 어떻게 위와 같은 `enum` 타입을 전송해줘야 할까? 그 답은 간단한데 클라이언트에서 `FREE`, `BASIC`, `PREMIUM`이라는 문자열을 전송하기만 하면된다. 

```
POST /api/categories
Host: example.com
Content-Type: application/json

{
    "email": "test@example.com",
    "password": "securepassword123",
    "membership": "BASIC"
}
```

### 특정 문자열을 이용하여 enum 처리

이 방식도 충분히 잘 설계되었지만, 여기서 조금 더 개선해보자. 만약에 클라이언트에서 기대하는 값이  `Basic`같은 앞 글자만 대문자의 형식을 띈 문자열이라면, 이를 어떻게 처리해야 할까? 

`enum` 타입에는 필드를 선언할 수 있는데, `value`라는 문자열 필드를 선언하여 문자열을 `enum` 타입에 저장할 수 있게 된다. 이때 `@JsonValue`라는 어노테이션을 `getter`에 선언하면 클라이언트에게 응답할 때 해당되는 필드를 선택하여 리턴하도록 명시할 수 있다.

```java
public enum Membership {
    FREE("Free"), 
    BASIC("Basic"), 
    PREMIUM("Premium")
    ;

    Membership(String value) {
        this.value = value;
    }

    @JsonValue
    public responseMembership() {
        return value;
    }

    private final String value;
}
```

이렇게 `enum`을 처리할 수 있게 되면 클라이언트는 응답받는 값과 똑같은 요청을 보내고 싶어할 것이다. 이 역시 스프링에서 지원해주는데, 바로 `@JsonCreator`이다. 

이 어노테이션을 활용하면 클라이언트에서 요청한 메시지 바디에 JSON 키 중에 `enum` 타입에 해당되는 키의 값을 불러와서 `enum` 타입이 가지는 특정 값이 있는 경우에 해당되는 `enum` 타입을 반환하여 요청 객체에 매핑한다.

또한 클라이언트에서 잘못된 문자열을 보냈을 경우에는 예외를 던지는 식으로 구성하여 적절한 예외 처리도 수행할 수 있다.

```java
@JsonCreator
public static Membership of(String value) {
    return Arrays.stream(Membership.values())
        .filter(m -> m.value.equals(value))
        .findFirst()
        .orElseThrow(IllegalArgumentException::new);
}
```

최종 코드는 다음과 같다.

```java
@RequiredArgsConstructor
public enum Membership {
    FREE("Free"), 
    BASIC("Basic"), 
    PREMIUM("Premium")
    ;

    Membership(String value) {
        this.value = value;
    }

    @JsonValue
    public String getValue() {
        return value;
    }

    @JsonCreator
    public static Membership of(String value) {
        for (Membership t : Membership.values()) {
            if (t.value.equals(value)) {
                return t;
            }
        }

        throw new IllegalArgumentException();
    }

    private final String value;
}
```

--- 

이렇게 카테고리의 형태를 띄는 데이터를 처리하는 방법을 알아보고, 스프링에서 이러한 데이터를 `enum`을 통해 처리할 수 있다는 것을 알아보았다. 최근에 젠킨스와 씨름 중인데 이 부분이 모두 끝나고 프론트엔드 쪽 배포도 끝나면 도커를 시작하여 인프라 시리즈로 돌아오도록 하겠다.