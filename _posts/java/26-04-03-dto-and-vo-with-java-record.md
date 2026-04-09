---
title: DTO와 VO

categories:
  - Java

toc: true
toc_sticky: true
published: true

date: 2026-04-03
---

# DTO

DTO(Data Transfer Object)는 계층 간 데이터 전송을 위한 객체이다. 컨트롤러에서 `@ModelAttribute`나 `@RequestBody` 어노테이션을 통해 전달받은 값들을 매핑하는 객체가 이에 해당된다. DTO 설계 시 규칙은 다음과 같다.

- 비즈니스 로직이 없어야 한다.
- 값 조회용 getter / 값 설정 용 setter만 존재해야 한다.
  - 생성자를 통한 값 설정으로 setter도 생략할 수 있다.

## 자바 레코드 활용

자바는 레코드를 활용하여 DTO를 간편하게 구현할 수 있다. 레코드는 자바 14에서 소개된 클래스 타입으로, 필드를 기반으로 자동으로 메서드가 생성되어 코드의 양을 줄일 수 있고, 기본적으로 불변성을 보장하기 때문에 데이터의 안정성을 높인다.

이를 활용하여 회원가입을 위해 사용자로부터 `username`과 `password`를 전달받는 DTO는 다음과 같다.

```java
public record RegisterRequest(
		String username,
		String password
) {
}
```

컨트롤러에서 서비스로 데이터를 넘길 때에는 이 값을 그대로 넘기면 된다.

```java
@RestController
@RequiredArgsConstructor
public class UserController {
	public final UserService service;

	@PostMapping("/register")
	public ResponseEntity<Void> register(
			@RequestBody RegisterRequest request
	) {
		service.register(request);
	}
}
```

나는 서비스에서 적절한 비즈니스 로직 처리 후 필요하다면 별도 DTO를 생성해 컨트롤러에게 넘겨주는 방식으로 개발하고 있다. 예를 들어 가입 이후 사용자 ID와 날짜를 리턴해야 한다고 가정해보자.

```java
public record RegisterResponse(
		Long userId,
		LocalDateTime createdAt
) {
}
```

이 DTO를 서비스에서 리턴하는 방식으로 개발한다.

```java
@Service
public class UserService {
	public RegisterResponse register(RegisterRequest request) {
		// 로직 수행
		return new RegisterResponse(1L, LocalDateTime.now());
	}
}
```

이렇게 하나의 요청에 대한 DTO를 Request / Response로 명시하면 클래스 파일 분류 시 어떤 역할을 하는지 명확하게 알 수 있어서 개인적으로 이 방법을 선호한다.

그리고 하나의 도메인에서 처리하는 일이 많아지다보면 DTO가 너무 많아지는데, 이럴 때에는 하나의 클래스로 묶어 관리하는 것을 고려할 수도 있다.

```java
public class UserRequest {
  public record Register(
		  String username,
		  String password
	) {}
}
```

# VO

VO(Value Object)는 값을 표현하는 객체다. VO 설계 시 규칙은 다음과 같다.

- 객체는 불변해야 한다.
- 객체는 로직을 포함할 수 있다.
- 객체의 모든 속성 값이 같다면 두 인스턴스는 같은 객체로 간주된다.

이 규칙에 맞춰서 한 번 구현해보도록 하자. 우선 비즈니스 로직을 포함하면서 객체를 불변하게 어떻게 유지할 수 있을까? 그 답은 속성의 변경이 있다면 새로운 VO를 리턴하는 방식으로 코드를 작성하는 것이다.

```java
public class Money {
	private long value;

	public Money(long value) {
		this.value = value;
	}

	public long getValue() {
		return value;
	}

	public void add(Money other) {
		return this.value + other.getValue(); // X
	}

	public Money add(long value) {
		return new Money(this.value + value); // O
	}
}
```

그리고 모든 필드를 `final`로 명시하면 불변성도 보장할 수 있다.

```java
public class Money {
  private final long value;
  // ...
}
```

이제 객체의 모든 속성 값이 같으면 같은 객체로 간주하는 방법을 알아보자. 이를 위해 `equals()`를 사용하는데, 재정의 하지 않으면 `Object` 클래스에 정의된 `equals()`를 사용한다. 그러한 경우 객체의 메모리를 검사하는 `==`가 동작하기 때문에 속성 값이 같아도 다르다고 간주된다.

따라서 `equals()`를 재정의하여 속성이 같으면 `true`를 반환하도록 만들자. 이때 자바의 계약(contract)에 의해 `equals()`가 `true`인 두 객체에 대해서 각각의 `hashCode()`는같은 값을 반환해야 하기 때문에 이를 반영하여 다음과 같이 작성해야 한다.

```java
@Override
public boolean equals(Object obj) {
  if (obj instanceof Money m) {
    return m.value == this.value;
  }

  return false;
}

@Override
public int hashCode() {
  return Long.hashCode(value);
}
```

> **equals()와 hashCode()의 계약(contract)**
>
> 자바의 `Object` 구현 코드에 작성된 주석을 살펴보면 `hashCode()` 부분에 다음과 같은 문장이 명시되어 있다.
>
> _If two objects are equal according to the equals(Object) method, then calling the hashCode method on each of the two objects must produce the same integer result._
>
> 이 두 메서드가 하나의 세트처럼 존재하는 이유는 해시 기반 자료구조 때문이다. 해시맵에 값을 저장하는 상황은 다음과 같다. 키에 대한 `hashCode()`를 호출해 저장될 인덱스를 탐색하고, 이미 해당 인덱스에 값이 있는 경우 저장된 노드의 키값과 `equals()`를 통해 같으면 덮어씌운다.
>
> 따라서 둘 중 하나라도 구현되어 있지 않으면 해당 객체에 대해서 해시 기반 자료구조는 제대로 동작하지 않는다. 그렇기 때문에 두 메서드는 강력한 계약 관계로 묶여있다고 표현한다.

`Money` VO의 전체 구현 코드는 다음과 같다.

```java
public class Money {
  private final long value;

  public Money(long value) {
    this.value = value;
  }

  public long getValue() {
	  return value;
  }

  public Money add(Money other) {
    return new Money(this.value + other.getValue());
  }

  @Override
  public boolean equals(Object obj) {
    if (obj instanceof Money m) {
      return m.value == this.value;
    }

    return false;
	}

  @Override
  public int hashCode() {
    return Long.hashCode(value);
  }

  @Override
  public String toString() {
    return "Money [" + value + "]";
  }
}
```

자료에서는 `toString()`을 언금하지 않았는데, 나는 개인적으로 디버깅과 로깅 용도로 재정의를 필수적으로 해야 한다고 생각한다.

## 자바 레코드 활용

VO를 클래스로 구현한 코드를 봤을 때, 기본적으로 3개 메서드에 대한 보일러 플레이트가 발생하고, 매 필드마다 `final`을 명시해야 하는 번거로움이 존재한다. 이러한 문제에 대해서 완벽한 대안은 바로 레코드다. 앞서 레코드는 필드를 기반으로 자동으로 메서드를 생성한다고 했는데, 생성되는 메서드들은 다음과 같다.

- 생성자
- getter
- `equals()`
- `hashCode()`
- `toString()`

즉, 레코드는 사실 VO를 간결하게 구현하기 위한 모던 자바의 대안이다. 위 VO를 레코드로 바꾼다면 다음과 같이 구현하면 된다.

```java
public record Money(
		long value
) {
  public Money add(Money other) {
	  return new Money(this.value + other.value());
  }
}
```

코드가 굉장히 간결해진 것을 볼 수 있다.

## VO의 로직의 범위

VO에 작성하는 로직의 범위는 일반적으로 값에 대한 검증, 연산, 비교, 표현이다. `Money` VO를 통해 하나씩 살펴보도록 하자.

**검증**

`Money` VO는 음수로부터 만들어질 수 없다. 따라서 VO 생성 시 생성자에서 이를 검사 해줘야 한다. 이때 레코드는 컴팩트 생성자를 지원하기 때문에 별도 파라미터 명시 없이 다음과 같이 간결하게 작성할 수 있다.

```java
public Money {
	if (value < 0) {
	  throw new IllegalArgumentException("돈은 음수가 될 수 없습니다.");
	}
}
```

**연산**

연산은 VO를 기반으로 새로운 값을 가지는 VO를 반환하는 것이다. 앞서 작성했던 `add()`가 이에 해당한다.

```java
public Money add(long value) {
  return new Money(this.value + value);
}
```

연산 메서드 작성 시 주의해야 할 점은 반환하는 타입이 VO의 타입과 다르면 안된다는 것이다.

```java
public record Won(
	long value
) {
	public Dollar exchange(double rate) {
		return new Dollar(value * rate);
	}
}
```

이는 환전을 하는 잘못된 VO 로직 예시다. 이 경우에는 해당 VO에 적합하지 않은 로직이거나 VO의 범위를 잘못 설정한 것은 아닌지 고민해봐야 한다. 적절한 예시는 다음과 같다.

```java
public record Money(
	long value,
	String country
) {
	public Money exchange(double rate, String newCountry) {
		return new Money(value * rate, newCountry);
	}
}
```

**비교**

비교는 VO와 어떤 값을 비교하는 것이다. `m1` 보다 `m2`가 더 돈이 많은지 검사하려면 다음과 같은 비교 로직을 작성하면 된다.

```java
public record Money(
	long value
) {
	public boolean isGreaterThan(Money other) {
		return value > other.value()
	}
}

Money m1 = new Money(1000);
Money m2 = new Money(500);

m1.isGreaterThan(m2); // true
```

이를 통해 의미를 확실하게 만들어 코드의 가독성을 올릴 수 있다.

**표현**

표현은 VO의 현재 상태나 포매팅 등을 수행한다. 다음 메서드를 정의하여 파산한 상태를 표현하거나 출력을 화폐단위 형식으로 포매팅할 수도 있다.

```java
public boolean isBankrupted() {
	return value == 0;
}

public String valueAsMoneyFormat() {
	return String.format("%,d", value);
}
```

# DTO와 VO 정리 표

지금까지 다뤘던 내용을 종합하여 표로 정리하면 다음과 같다.

| 구분        | DTO (Data Transfer Object)                 | VO (Value Object)                            |
| ----------- | ------------------------------------------ | -------------------------------------------- |
| 목적        | 계층 간 데이터 전송 (택배 상자)            | 도메인의 특정 값 표현 (값 그 자체)           |
| 가변성      | 가변(Mutable) 또는 불변(Immutable)         | 무조건 불변 (Immutable)                      |
| 로직        | 비즈니스 로직 없음 (Getter/Setter만 존재)  | 자가 검증, 연산, 비교, 표현 로직 포함        |
| 동등성      | 주소값이 다르면 다른 객체 (중요치 않음)    | 값이 같으면 같은 객체 (equals/hashCode 필수) |
| 식별자      | 없음 (데이터의 묶음일 뿐)                  | 없음 (값 자체가 정체성)                      |
| 생성자      | 주로 기본 생성자 + Setter 또는 전체 생성자 | 컴팩트 생성자를 통한 유효성 검증 필수        |
| 사용 위치   | Controller ↔ Service, Service ↔ 외부 API   | Service ↔ Domain, Entity 내부 필드           |
| Java Record | 단순 데이터 전달용으로 매우 적합           | VO의 속성(불변, 동등성)과 완벽히 일치        |

---

# 참고 자료

[**[JAVA] DTO와 VO의 차이**](https://maenco.tistory.com/entry/Java-DTO%EC%99%80-VO%EC%9D%98-%EC%B0%A8%EC%9D%B4)

[**[Java] Record란?**](https://m.blog.naver.com/seek316/223341255150)
