---
title: 전략 패턴 (구조와 조건 분기 대체 기준)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-23

permalink: "/object-oriented-design/strategy-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 전략 패턴을 다룹니다. 산술 연산 예제로 알고리즘을 전략으로 분리하고 교체하는 방법을 살펴본 뒤, 전략 선택을 팩토리로 분리하는 방법과 조건 분기를 전략으로 바꿔야 하는 기준을 정리하겠습니다.

# 전략 패턴이란

여러 유사한 알고리즘(비즈니스 규칙)의 공통 인터페이스를 정의하고, 각각을 개별 클래스로 구체화(캡슐화)하여 알고리즘을 사용하는 클라이언트의 변경 없이 서로 교환할 수 있게 하는 행동 디자인 패턴입니다.

# 활용 상황

전략 패턴은 주로 다음 상황에서 사용됩니다.

- 알고리즘의 다양한 변형이 존재하고, 런타임 중 이러한 변형을 교체하고자 하는 경우

객체의 동작을 하위 전략 객체들과 연결함으로써, 실행 중에도 상황에 따라 알고리즘을 교체할 수 있습니다.

- 전체 과정은 유사하지만 일부 행동이 다른 클래스가 많은 경우

공통 부분을 하나의 클래스로 결합하고, 하위 클래스에서 일부 행동을 확장하여 코드 중복을 줄일 수 있습니다.

- 클래스의 핵심 비즈니스 로직을 알고리즘의 구현 세부 사항으로부터 격리하고자 하는 경우

클라이언트는 제공되는 인터페이스만을 통해 전략 패턴에 접근하기 때문에 변경 범위를 최소화할 수 있습니다.

- 다양한 알고리즘의 변형을 사용하기 위해 사용하는 조건 분기문을 없애고자 하는 경우

공통 부분을 묶고, 하위 클래스에서 세부 사항을 결정하는 방식을 통해 복잡한 조건문을 제거할 수 있습니다.

# 구조

전략 패턴은 다음과 같은 구조를 가집니다.

- 전략(Strategy) 인터페이스는 모든 알고리즘이 공통으로 따르는 작업을 선언합니다.
- 구상 전략(Concrete Strategy)은 전략 인터페이스를 구현하며, 알고리즘 하나를 하나의 클래스로 캡슐화합니다.
- 컨텍스트(Context)는 전략 객체에 대한 참조를 유지하고, 작업을 전략에 위임합니다. 구상 전략의 클래스는 알지 못하고 전략 인터페이스만 사용합니다.
- 클라이언트는 구상 전략을 선택하여 컨텍스트에 전달합니다. 올바른 전략을 고르기 위해 전략 간 차이를 알고 있어야 합니다.

## 구현 방법

컨텍스트가 여러 전략을 사용하여 산술 연산을 실행하는 예시입니다. 연산(덧셈, 뺄셈, 곱셈)마다 알고리즘이 다르므로 전략 인터페이스로 공통 규격을 정의하고, 각 연산을 구상 전략으로 분리합니다.

컨텍스트는 생성자로 전략을 받고 `setStrategy()`로 런타임에 전략을 교체할 수 있습니다. 계산은 직접 하지 않고 전략에 위임합니다.

{% include code-header.html %}
```java
// 전략 인터페이스: 모든 알고리즘이 공통으로 따르는 작업을 선언한다.
public interface Strategy {
    int execute(int a, int b);
}

// 구상 전략: 알고리즘 하나를 하나의 클래스로 구현한다.
public class AddStrategy implements Strategy {
    @Override
    public int execute(int a, int b) {
        return a + b;
    }
}

public class SubtractStrategy implements Strategy {
    @Override
    public int execute(int a, int b) {
        return a - b;
    }
}

public class MultiplyStrategy implements Strategy {
    @Override
    public int execute(int a, int b) {
        return a * b;
    }
}

// 컨텍스트: 전략 인터페이스에만 의존하며 구상 전략을 알지 못한다.
public class Context {

    private Strategy strategy;

    public Context(Strategy strategy) {
        this.strategy = strategy;
    }

    // 런타임에 전략을 교체한다.
    public void setStrategy(Strategy strategy) {
        this.strategy = strategy;
    }

    public int executeStrategy(int a, int b) {
        return strategy.execute(a, b);
    }
}
```

클라이언트는 구상 전략을 선택하여 컨텍스트에 전달합니다. 아래 코드에서는 `switch`로 전략을 선택하므로 클라이언트가 각 전략의 차이를 알고 있어야 한다는 단점이 드러납니다.

{% include code-header.html %}
```java
public class Main {
    public static void main(String[] args) {
        String action = "multiplication";
        Context context = new Context(new AddStrategy());

        switch (action) {
            case "addition" -> context.setStrategy(new AddStrategy());
            case "subtraction" -> context.setStrategy(new SubtractStrategy());
            case "multiplication" -> context.setStrategy(new MultiplyStrategy());
        }

        int result = context.executeStrategy(6, 3);
        System.out.println(result); // 18
    }
}
```

전략 인터페이스의 메서드가 하나뿐이면 람다로도 같은 효과를 낼 수 있습니다. 이 경우 구상 전략 클래스를 따로 정의하지 않아도 됩니다.

{% include code-header.html %}
```java
Context context = new Context((a, b) -> a + b);
context.setStrategy((a, b) -> a * b);
```

### 전략 선택을 팩토리로 분리하기

앞의 클라이언트처럼 `switch`로 전략을 고르면 전략이 늘어날 때마다 조건문이 길어집니다. 전략 객체에 상태가 없다면 `Map`에 미리 만들어 두고 조회하는 팩토리로 선택 책임을 분리할 수 있습니다. 주문 유형별 할인 정책을 예로 들겠습니다.

{% include code-header.html %}
```java
public enum OrderType {
    NORMAL, GROUPON, PROMOTION
}

public interface DiscountStrategy {
    long calculateDiscount(long originalPrice);
}

public class DiscountStrategyFactory {

    private static final Map<OrderType, DiscountStrategy> STRATEGIES = new EnumMap<>(OrderType.class);

    static {
        STRATEGIES.put(OrderType.NORMAL, price -> 0L);
        STRATEGIES.put(OrderType.GROUPON, price -> price / 10);    // 10% 할인
        STRATEGIES.put(OrderType.PROMOTION, price -> price / 5);   // 20% 할인
    }

    public static DiscountStrategy getStrategy(OrderType type) {
        DiscountStrategy strategy = STRATEGIES.get(type);
        if (strategy == null) {
            throw new IllegalArgumentException("지원하지 않는 주문 유형입니다: " + type);
        }
        return strategy;
    }
}
```

주문 서비스는 전략의 선택을 팩토리에 맡기고, 선택된 전략의 `calculateDiscount()`만 호출합니다.

{% include code-header.html %}
```java
public class OrderService {

    public long calculateFinalPrice(OrderType type, long originalPrice) {
        DiscountStrategy strategy = DiscountStrategyFactory.getStrategy(type);
        return originalPrice - strategy.calculateDiscount(originalPrice);
    }
}
```

새로운 할인 정책(예: VIP)이 추가되어도 `OrderService`는 수정하지 않습니다. 다만 `OrderType`에 값을 추가하고 팩토리에 전략을 등록하는 변경은 필요합니다. 전략 객체가 상태를 가져 매번 새로 만들어야 한다면 팩토리 메서드 안에서 `new`로 생성합니다.

이 구조는 컨텍스트가 전략을 필드로 들고 있는 기본형과 달리, 서비스가 팩토리를 직접 호출하는 전략 + 팩토리 형태입니다.

# 장단점

**장점**

- **런타임 교체 유연성**: 런타임에 다양한 알고리즘(전략)을 상황에 맞춰 유연하게 교체할 수 있습니다.
- **구현 세부 사항 은닉**: 알고리즘을 실행하는 클라이언트(컨텍스트) 코드로부터 알고리즘의 복잡한 계산식이나 내부 데이터를 은닉(캡슐화)할 수 있습니다.
- **합성(Composition) 활용**: 알고리즘 확장을 위해 상속을 쓰는 대신 객체 합성을 사용하여, 상속으로 인한 클래스 폭발을 막고 결합도를 낮출 수 있습니다.
- **개방-폐쇄 원칙(OCP) 준수**: 기존 컨텍스트 코드를 변경하지 않고도 새로운 전략 클래스를 자유롭게 추가·확장할 수 있습니다.

**단점**

- **오버엔지니어링 위험**: 알고리즘의 개수가 몇 개 없고 거의 변하지 않는다면, 오히려 클래스 수만 늘어나 프로그램의 복잡도를 불필요하게 높일 수 있습니다.
- **클라이언트의 인지 부담**: 적절한 전략을 선택하기 위해 클라이언트가 각 전략 간의 세부적인 동작 차이를 미리 이해하고 있어야 합니다.
- **현대 언어의 함수형 대체재 존재**: Java 8+ 등의 현대 언어는 람다(익명 함수)를 지원하므로, 굳이 개별 전략 클래스를 일일이 정의하지 않고도 간단한 함수 전달만으로 동일한 효과를 낼 수 있어 클래스 기반 구현의 메리트가 떨어질 수 있습니다.

---

정리하면, 전략은 알고리즘이 복잡하거나 계속 늘어나거나 런타임에 교체해야 할 때 전략의 정의, 생성, 사용을 분리하는 패턴입니다. 분기가 단순하고 변경 가능성이 낮다면 조건 분기를 유지하는 편이 낫고, 알고리즘이 메서드 하나라면 람다로 충분한지 먼저 따져 봅니다.

# 참고 자료

[**전략 패턴**](https://refactoring.guru/ko/design-patterns/strategy)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
