---
title: 어댑터 패턴 (구조와 인터페이스 변환)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-16

permalink: "/object-oriented-design/adapter-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 어댑터를 다룹니다. 구조와 정사각형 못·둥근 구멍 예제를 살펴본 뒤, 구조가 비슷한 [데코레이터 패턴](/object-oriented-design/decorator-pattern/), [프록시 패턴](/object-oriented-design/proxy-pattern/)과 무엇이 다른지 정리하겠습니다.

# 어댑터 패턴이란

호환되지 않는 인터페이스를 호환 가능하도록 변환하여 동작하게 만드는 구조 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 애플리케이션이 레거시 코드, 외부 라이브러리와 호환되지 않는 경우

애플리케이션에서는 JSON 데이터를 요구하지만 라이브러리는 XML을 반환하는 경우 등 호환되지 않는 레거시 코드, 인터페이스, 외부 라이브러리 간 변환기 역할을 할 수 있습니다.

- 상위 클래스에 기능을 추가할 수 없는 상황에서, 여러 하위 클래스에 대해 공통 기능을 추가해야 하는 경우

어댑터 클래스에 이러한 공통 기능을 정의하고, 객체를 감싸서 필요한 기능을 추가할 수 있습니다.

객체를 감싸서 기능을 덧붙이는 구조는 데코레이터와 유사하지만, 기존 인터페이스를 그대로 유지하는 데코레이터와 달리 클라이언트가 요구하는 다른 인터페이스로 변환할 수 있다는 차이점이 있습니다.

# 구조

어댑터 패턴은 다음과 같은 구조를 가집니다.

- 클라이언트에서 호환되지 않는 대상을 호출하는 부분을 분리해 인터페이스로 제공합니다.
- 클라이언트는 인터페이스에 의존하고, 어댑터는 인터페이스를 구현합니다.
- 어댑터에서 호환되지 않는 대상을 호출해 호환성을 맞춰 클라이언트에게 제공합니다.

# 구현 예시

둥근 구멍에 정사각형 못을 맞춰 넣는 예시입니다. 구멍은 둥근 못만 받을 수 있고 정사각형 못은 너비만 알고 있어 서로 호환되지 않습니다. 어댑터가 정사각형 못을 감싸서, 그 못이 들어갈 수 있는 가장 작은 원의 반지름을 가진 둥근 못인 척합니다.

구멍이 받을 수 있는 못의 인터페이스를 선언합니다.

{% include code-header.html %}
```java
// 클라이언트(RoundHole)가 기대하는 인터페이스
public interface RoundPeg {
    double getRadius();
}
```

기존 구상 클래스는 이 인터페이스를 이미 만족합니다. 구멍은 `RoundPeg` 인터페이스에만 의존합니다.

{% include code-header.html %}
```java
// 둥근 구멍: 반지름이 못 이상이면 못이 들어간다.
public class RoundHole {
    private final double radius;

    public RoundHole(double radius) {
        this.radius = radius;
    }

    public double getRadius() {
        return radius;
    }

    public boolean fits(RoundPeg peg) {
        return this.getRadius() >= peg.getRadius();
    }
}
```

{% include code-header.html %}
```java
// 둥근 못: 인터페이스의 기본 구현이다.
public class BasicRoundPeg implements RoundPeg {
    private final double radius;

    public BasicRoundPeg(double radius) {
        this.radius = radius;
    }

    @Override
    public double getRadius() {
        return radius;
    }
}
```

호환되지 않는 클래스는 인터페이스를 구현하지 않고 너비만 제공합니다. 라이브러리 코드처럼 수정할 수 없다고 가정합니다.

{% include code-header.html %}
```java
// 호환되지 않는 클래스: RoundPeg 인터페이스를 구현하지 않는다.
public class SquarePeg {
    private final double width;

    public SquarePeg(double width) {
        this.width = width;
    }

    public double getWidth() {
        return width;
    }
}
```

어댑터는 클라이언트가 기대하는 인터페이스를 구현하고, 호환되지 않는 객체를 필드로 가집니다. 호출을 받으면 감싼 객체의 값을 인터페이스가 요구하는 값으로 변환합니다.

{% include code-header.html %}
```java
// 어댑터: 정사각형 못을 둥근 못처럼 보이게 변환한다.
public class SquarePegAdapter implements RoundPeg {
    private final SquarePeg peg;

    public SquarePegAdapter(SquarePeg peg) {
        this.peg = peg;
    }

    @Override
    public double getRadius() {
        // 정사각형 못이 들어갈 수 있는 가장 작은 원의 반지름(대각선의 절반)을 반환한다.
        return peg.getWidth() * Math.sqrt(2) / 2;
    }
}
```

클라이언트는 `RoundPeg` 타입만 사용하므로 정사각형 못을 어댑터로 감싸서 전달하면 코드를 바꿀 필요가 없습니다.

{% include code-header.html %}
```java
// 클라이언트 코드
public class Application {
    public void example() {
        RoundHole hole = new RoundHole(5);
        RoundPeg roundPeg = new BasicRoundPeg(5);
        hole.fits(roundPeg); // true

        SquarePeg smallSquarePeg = new SquarePeg(5);
        SquarePeg largeSquarePeg = new SquarePeg(10);
        // hole.fits(smallSquarePeg); // 컴파일되지 않는다. (호환되지 않는 타입)

        RoundPeg smallAdapter = new SquarePegAdapter(smallSquarePeg);
        RoundPeg largeAdapter = new SquarePegAdapter(largeSquarePeg);
        hole.fits(smallAdapter); // true (반지름 약 3.54)
        hole.fits(largeAdapter); // false (반지름 약 7.07)
    }
}
```

# 장단점

**장점**

- **단일 책임 원칙(SRP) 준수**: 비즈니스 로직에서 인터페이스 또는 데이터 변환 코드를 분리하여 단일 책임 원칙을 준수할 수 있습니다.
- **개방-폐쇄 원칙(OCP) 준수**: 클라이언트가 인터페이스에 의존하므로 기존의 클라이언트 코드를 변경하지 않고 새로운 유형의 어댑터를 도입할 수 있는 개방-폐쇄 원칙을 준수할 수 있습니다.

**단점**

- **복잡도 증가**: 다수의 새로운 인터페이스와 클래스를 도입해야 하므로 코드의 전반적인 복잡도가 증가합니다.

# 자바 로깅과 어댑터 패턴

Java 생태계에는 Log4j, Logback, JUL(Java Util Logging)과 같은 다양한 로깅 프레임워크와 JCL(Apache Commons Logging), SLF4J 등의 로깅 API가 존재합니다.

이러한 로깅 기술들은 유사한 기능을 제공하지만 통일된 인터페이스를 사용하지 않습니다. 따라서 라이브러리나 프레임워크마다 서로 다른 로깅 API를 사용하는 경우, 로그 출력과 설정을 일관되게 관리하기 어려워질 수 있습니다.

이러한 문제를 해결하기 위해 등장한 SLF4J는 다양한 로깅 구현체를 사용할 수 있도록 공통 인터페이스를 제공하는 로깅 추상화 계층(Facade)입니다.

그러나 JUL, JCL, Log4j 1.x 등은 SLF4J보다 먼저 등장했기 때문에 SLF4J의 인터페이스를 직접 사용하지 않습니다. 따라서 기존 라이브러리가 사용하는 로깅 API를 SLF4J로 통합하려면 별도의 호환성 계층이 필요합니다.

이를 위해 SLF4J는 기존 로깅 API의 호출을 SLF4J API로 전달하는 어댑터를 제공합니다. 서로 다른 인터페이스 사이의 호환성을 확보한다는 점에서 어댑터 패턴의 활용 사례로 볼 수 있습니다.

이렇게 SLF4J로 통합된 로그 호출은 Provider를 통해 Logback이나 Log4j2 등의 로깅 구현체로 전달되며, 애플리케이션은 로깅 구현체를 중심으로 로그를 일관되게 관리할 수 있습니다.

---

정리하면, 어댑터는 호환되지 않는 대상을 감싸 클라이언트가 기대하는 인터페이스로 변환해 주는 패턴입니다. 감싸는 구조는 데코레이터, 프록시와 비슷하지만, 두 패턴이 원본의 인터페이스를 유지하는 것과 달리 어댑터는 인터페이스 자체를 바꾼다는 점이 다릅니다.

# 참고 자료

[**어댑터 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/adapter)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
