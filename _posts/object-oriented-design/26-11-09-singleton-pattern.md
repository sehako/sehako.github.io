---
title: 싱글턴 패턴 (Java 구현 방법과 Spring 싱글톤 빈과의 차이)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-09

permalink: "/object-oriented-design/singleton-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 생성 패턴 편입니다. 가장 널리 알려진 싱글턴을 다룹니다. 구현 방법과 단점을 살펴본 뒤, Spring의 싱글톤 빈과 무엇이 다른지까지 정리하겠습니다.

# 싱글턴 패턴이란

싱글턴은 애플리케이션에 인스턴스가 하나만 있도록 하면서 전역 접근 지점을 제공하는 생성 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 단일 인스턴스를 모든 클라이언트가 공유해야 하는 경우

대표적인 예시가 데이터베이스 커넥션 풀을 관리하는 객체입니다. 데이터베이스 연결을 요청마다 새로 생성하면 연결 수립에 따른 비용이 발생합니다. 그래서 커넥션 풀을 관리하는 객체를 하나만 만들고 여러 클라이언트가 공유하도록 구성합니다.

이때 클라이언트는 동일한 풀 관리 객체를 사용하지만, 실제 데이터베이스 커넥션은 풀에서 각각 할당받아 사용합니다.

- 전역 변수를 엄격하게 제어해야 하는 경우

자바에서 전역 변수에 가장 가까운 것은 `public static` 필드입니다. 커넥션 풀을 이 방식으로 공유한다고 가정해 보겠습니다.

{% include code-header.html %}
```java
public class ConnectionPool {
    // 전역 변수처럼 사용하는 public static 필드
    public static ConnectionPool pool = new ConnectionPool();
}

// 다른 코드에서 공유 인스턴스를 다른 객체로 교체할 수 있습니다.
ConnectionPool.pool = new ConnectionPool();

// new 연산자로 별개의 인스턴스를 얼마든지 만들 수 있습니다.
ConnectionPool another = new ConnectionPool();
```

공유 인스턴스가 교체되면 그 전에 `pool`을 참조하던 코드와 이후에 참조하는 코드가 서로 다른 풀을 사용하게 됩니다. `new`로 만든 인스턴스도 커넥션을 따로 관리하므로 풀을 하나로 제한하려던 의도가 깨집니다.

싱글턴은 생성자를 비공개하고 접근 경로를 `getInstance()` 하나로 제한해서 이 두 가지를 막습니다.

{% include code-header.html %}
```java
public class ConnectionPool {
    private static final ConnectionPool INSTANCE = new ConnectionPool();

    private ConnectionPool() {}

    public static ConnectionPool getInstance() {
        return INSTANCE;
    }
}

new ConnectionPool();                          // 컴파일 오류: 생성자가 private
ConnectionPool.INSTANCE = new ConnectionPool(); // 컴파일 오류: private + final
```

단순히 `public static final`로 선언하면 교체는 막을 수 있지만 `new`로 인스턴스를 추가로 만드는 것은 막지 못합니다. 싱글턴의 핵심은 참조 교체 방지보다 인스턴스 생성 통제에 있습니다.

# Java 구현 방법

싱글턴 구현은 공통적으로 두 단계를 가집니다.

- 다른 객체가 싱글턴 클래스에 `new` 연산자를 쓰지 못하도록 기본 생성자를 비공개합니다.
- 인스턴스에 접근하는 정적 메서드를 정의합니다.

인스턴스를 언제 만드는지, 스레드 안전성을 어떻게 보장하는지에 따라 구현 방식이 나뉩니다. 앞의 예시와 같은 커넥션 풀 객체로 살펴보겠습니다.

## 지연 초기화 (Double-Checked Locking)

`getInstance()`가 처음 호출될 때 인스턴스를 생성합니다.

{% include code-header.html %}
```java
public class ConnectionPool {
    // 싱글턴 인스턴스를 저장하기 위한 필드는 정적으로 선언합니다.
    private static volatile ConnectionPool instance;

    private final Queue<Connection> idleConnections = new ConcurrentLinkedQueue<>();

    // 기본 생성자를 비공개 처리해 new 연산을 방지합니다.
    private ConnectionPool() {
        // 데이터베이스 커넥션 풀 설정 및 초기화 등의 작업 코드
    }

    // 싱글턴 인스턴스로의 접근을 제어하는 정적 메서드
    public static ConnectionPool getInstance() {
        if (instance == null) {
            // 락을 통해 여러 스레드에서 접근 시 여러 인스턴스 생성을 방지합니다.
            synchronized (ConnectionPool.class) {
                // 다른 스레드에 의해 초기화되지 않았는지 다시 확인합니다.
                if (instance == null) {
                    instance = new ConnectionPool();
                }
            }
        }
        return instance;
    }

    // 커넥션 풀에서 사용 가능한 커넥션을 반환합니다.
    public Connection getConnection() {
        return idleConnections.poll();
    }
}
```

{% include code-header.html %}
```java
ConnectionPool foo = ConnectionPool.getInstance();
Connection conn1 = foo.getConnection();

ConnectionPool bar = ConnectionPool.getInstance();
Connection conn2 = bar.getConnection();

// foo와 bar는 동일한 ConnectionPool 인스턴스를 참조합니다.
System.out.println(foo == bar); // true
```

`volatile`은 필수입니다. 이 키워드가 없으면 `instance = new ConnectionPool()`의 메모리 할당, 생성자 실행, 참조 대입 순서가 바뀌어 다른 스레드가 초기화가 끝나지 않은 객체를 볼 수 있습니다.

## 즉시 초기화

클래스 초기화 시점에 인스턴스를 미리 생성해 두는 방식입니다. 자바는 클래스 초기화 과정의 스레드 안전성을 보장하므로 별도의 동기화 코드가 필요 없습니다.

{% include code-header.html %}
```java
public class ConnectionPool {
    private static final ConnectionPool INSTANCE = new ConnectionPool();

    private ConnectionPool() {}

    public static ConnectionPool getInstance() {
        return INSTANCE;
    }
}
```

인스턴스를 사용하지 않아도 클래스가 로딩될 때 생성되므로, 생성 비용이 크다면 낭비가 될 수 있습니다.

## Holder 패턴

지연 초기화의 이점을 살리면서 동기화 코드를 쓰지 않는 방식입니다. 내부 정적 클래스는 `getInstance()`가 처음 호출될 때 로딩되므로 인스턴스도 그때 생성됩니다. 클래스 초기화의 스레드 안전성은 JVM이 보장합니다.

{% include code-header.html %}
```java
public class ConnectionPool {
    private ConnectionPool() {}

    private static class Holder {
        private static final ConnectionPool INSTANCE = new ConnectionPool();
    }

    public static ConnectionPool getInstance() {
        return Holder.INSTANCE;
    }
}
```

## enum 싱글턴

enum 상수는 JVM이 하나만 생성하는 것을 보장합니다. 리플렉션이나 직렬화로 인스턴스가 추가로 만들어지는 문제도 막을 수 있습니다.

{% include code-header.html %}
```java
public enum ConnectionPool {
    INSTANCE;

    private final Queue<Connection> idleConnections = new ConcurrentLinkedQueue<>();

    // 커넥션 풀에서 사용 가능한 커넥션을 반환합니다.
    public Connection getConnection() {
        return idleConnections.poll();
    }
}

ConnectionPool.INSTANCE.getConnection();
```

다만 다른 클래스를 상속할 수 없고 인터페이스 구현만 가능합니다.

# 장단점

싱글턴 패턴의 장단점은 다음과 같습니다.

**장점**

- 클래스가 하나의 인스턴스만 가진다는 것을 보장합니다.
- 인스턴스에 대한 전역 접근 지점을 얻습니다.
- 즉시 초기화 또는 지연 초기화를 통해 인스턴스의 생성 시점을 제어할 수 있습니다.

**단점**

- 단일 책임 원칙(SRP) 위반 가능성: 객체의 생성·관리 정책과 비즈니스 정책이 하나의 클래스에 결합될 수 있습니다. 서로 다른 변경 이유를 가진다면 SRP를 위반할 수 있습니다.
- 높은 결합도: 클라이언트가 `getInstance()`를 직접 호출하면 구체적인 싱글턴 클래스에 의존하게 되어 구현체 교체가 어려워집니다.
- 단위 테스트의 어려움: 클라이언트가 싱글턴 인스턴스를 직접 획득하는 구조에서는 의존성을 Mock 객체로 교체하기 어렵습니다.
- 공유 상태 관리 문제: 싱글턴이 변경 가능한 내부 상태를 가진다면 여러 클라이언트가 해당 상태를 공유하므로 예상치 못한 상태 변경이나 동시성 문제가 발생할 수 있습니다.

# Spring 싱글톤 빈과의 차이

Spring 빈의 기본 스코프도 싱글톤입니다. 하지만 싱글턴 패턴을 코드로 구현한 것은 아닙니다. 두 방식은 인스턴스를 하나로 유지한다는 결과만 같고, 그 책임을 지는 주체가 다릅니다.

| 구분 | 싱글턴 패턴 | Spring 싱글톤 빈 |
| --- | --- | --- |
| 단일성을 보장하는 주체 | 클래스 자신 (비공개 생성자 + `getInstance()`) | Spring 컨테이너 (싱글톤 레지스트리) |
| 클래스의 모습 | 싱글턴 구현 코드가 섞여 있음 | 평범한 클래스 |
| 클라이언트의 접근 | `getInstance()`로 구체 클래스에 직접 의존 | DI로 주입받음 |
| 단일성의 범위 | 클래스 로더 안에서 하나 | 컨테이너 안에서 빈 정의당 하나 |
| 테스트 | Mock으로 교체하기 어려움 | 테스트에서 다른 구현이나 Mock을 주입 가능 |

싱글턴 패턴의 단점 중 결합도와 테스트 문제는 컨테이너가 객체의 생성과 주입을 맡으면서 해결됩니다. 클라이언트는 `getInstance()`를 호출하지 않고 생성자로 의존성을 주입받으므로 구체 클래스를 알 필요가 없습니다.

{% include code-header.html %}
```java
@Component
public class OrderService {
    private final ConnectionPool pool;

    // 컨테이너가 인스턴스를 주입하므로 getInstance()를 호출하지 않습니다.
    public OrderService(ConnectionPool pool) {
        this.pool = pool;
    }
}
```

반면 공유 상태 문제는 그대로 남습니다. 싱글톤 빈은 여러 요청이 하나의 인스턴스를 공유하므로, 요청마다 달라지는 값을 필드에 저장하면 서로의 값을 덮어씁니다. Spring 빈은 상태를 유지하지 않는(stateless) 객체로 설계해야 합니다.

{% include code-header.html %}
```java
@Component
public class FuelService {
    private int fuel; // 모든 요청이 공유하는 필드

    public void fuelUp(int amount) {
        fuel += amount; // 요청 A의 3리터와 요청 B의 2리터가 하나로 합쳐집니다.
    }
}
```

또한 컨테이너가 하나만 만들도록 관리하는 것이지 클래스가 하나만 만들어지도록 막는 것이 아닙니다. 같은 클래스를 `new`로 만들거나 컨테이너를 여러 개 띄우면 인스턴스도 여러 개가 됩니다.

---

지금까지 살펴본 내용을 기준으로 싱글턴을 쓸지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 단일 인스턴스가 꼭 필요하고 생성 비용이 크다 | 쓴다 | 커넥션 풀처럼 연결 수립 비용이 큰 자원을 공유한다 |
| 인스턴스 생성과 접근 경로를 엄격히 통제해야 한다 | 쓴다 | 비공개 생성자로 `new`와 참조 교체를 막는다 |
| 변경 가능한 상태를 여러 클라이언트가 공유하게 된다 | 피한다 | 예상치 못한 상태 변경과 동시성 문제가 생긴다 |
| 구현 교체나 Mock 주입이 필요하다 | 피한다 | `getInstance()` 직접 호출은 구체 클래스에 결합된다 |
| 생성·관리 정책이 비즈니스 로직과 섞인다 | 피한다 | 서로 다른 변경 이유가 한 클래스에 묶여 SRP를 위반한다 |
| Spring 컨테이너를 쓰고 있다 | 직접 구현하지 않는다 | 컨테이너가 같은 일을 하면서 결합도와 테스트 문제를 해결한다 |

정리하면, 싱글턴이 필요한 상황에서 Spring을 쓰고 있다면 직접 구현하기보다 컨테이너에 맡기는 편이 낫습니다. 직접 구현한 싱글턴이 필요한 경우는 Spring 컨테이너 밖에서 동작하는 코드 정도로 줄어듭니다.

# 참고 자료

[**싱글턴 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/singleton)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
