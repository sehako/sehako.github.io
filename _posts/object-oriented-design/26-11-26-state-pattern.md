---
title: 상태 패턴 (구조와 상태 전이 규칙 구현)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-26

permalink: "/object-oriented-design/state-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 상태 패턴을 다룹니다. 주문 상태별로 허용되는 동작과 전이 규칙을 구현해 보고, 상태 패턴을 도입해야 하는 기준을 정리하겠습니다.

# 상태 패턴이란

객체의 내부 상태가 변함에 따라서 행동을 동적으로 변경할 수 있도록 하는 행동 디자인 패턴입니다. 객체가 상태를 바꿀 때, 마치 객체의 클래스 자체가 완전히 다른 클래스로 바뀐 것처럼 동작하게 됩니다.

# 활용 상황

상태 패턴은 주로 다음 상황에서 사용됩니다.

- 현재 상태에 따라 다르게 행동하는 객체와 그 상태의 수가 많고, 상태의 변경에 따른 행동이 빈번한 경우

상태별 동작을 독립된 클래스로 분리하면, 새 상태를 추가하거나 기존 로직을 수정해도 다른 상태의 코드에 영향을 주지 않습니다.

- 상태 값에 따라 달라지는 `if-else`, `switch` 분기문이 메서드마다 반복되어 클래스가 비대해진 경우

분기마다 흩어져 있던 코드를 각 상태 클래스의 메서드로 옮기면, 메인 클래스에서 복잡한 조건문과 상태 확인용 필드를 제거할 수 있습니다.

- 상태 간에 중복되는 로직이나 공통 전이 규칙이 많은 경우

조건문으로 작성하면 전이 코드와 공통 로직이 여러 곳에 중복되기 쉽습니다. 중간 추상 클래스로 상태 계층을 만들어 공통 행동을 한곳에 모으면 중복을 줄일 수 있습니다.

# 구조

상태 패턴은 다음과 같은 구조를 가집니다.

- 컨텍스트(Context)는 현재 상태 객체를 참조하고, 요청을 그 객체에 위임합니다. 상태를 교체하는 메서드를 제공합니다.
- 상태(State) 인터페이스는 모든 상태가 공통으로 따르는 이벤트별 메서드를 선언합니다.
- 구상 상태(Concrete State)는 상태별 동작과 전이 규칙을 구현합니다. 조건이 충족되면 컨텍스트의 상태를 다음 상태로 교체합니다.
- 클라이언트는 컨텍스트에 요청만 보냅니다. 전이는 상태 객체가 일으킵니다.

## 구현 방법

주문 상태별로 허용되는 동작과 전이 규칙을 구현하는 예시입니다. 주문은 `결제 대기 → 배송 중 → 배송 완료` 순으로 진행되고, 결제 대기 상태에서만 `취소됨`으로 전이할 수 있습니다. 상태마다 허용되지 않는 이벤트는 예외를 던집니다.

컨텍스트는 상태를 검사하는 조건문 없이 요청을 현재 상태 객체에 위임합니다. 전이는 상태 객체가 `changeState()`로 일으킵니다.

{% include code-header.html %}
```java
// 상태 인터페이스: 모든 상태가 공통으로 따르는 이벤트를 선언한다.
public interface OrderState {
    void pay(OrderContext context);
    void deliver(OrderContext context);
    void cancel(OrderContext context);
}

// 컨텍스트: 현재 상태 객체를 참조하고 요청을 위임한다.
public class OrderContext {

    private OrderState state = new PaymentPendingState();

    // 상태 객체가 호출하는 전이 메서드
    public void changeState(OrderState next) {
        this.state = next;
    }

    public void pay() {
        state.pay(this);
    }

    public void deliver() {
        state.deliver(this);
    }

    public void cancel() {
        state.cancel(this);
    }
}
```

구상 상태는 자신이 허용하는 이벤트만 처리하고 다음 상태로 전이시킵니다.

{% include code-header.html %}
```java
public class PaymentPendingState implements OrderState {
    @Override
    public void pay(OrderContext context) {
        context.changeState(new ShippedState());
    }

    @Override
    public void deliver(OrderContext context) {
        throw new IllegalStateException("결제되지 않은 주문은 배송할 수 없다.");
    }

    @Override
    public void cancel(OrderContext context) {
        context.changeState(new CanceledState());
    }
}

public class ShippedState implements OrderState {
    @Override
    public void pay(OrderContext context) {
        throw new IllegalStateException("이미 결제된 주문이다.");
    }

    @Override
    public void deliver(OrderContext context) {
        context.changeState(new DeliveredState());
    }

    @Override
    public void cancel(OrderContext context) {
        throw new IllegalStateException("배송 중인 주문은 취소할 수 없다.");
    }
}
```

배송 완료와 취소됨은 더 이상 전이하지 않는 종료 상태이므로 모든 이벤트가 예외입니다.

{% include code-header.html %}
```java
public class DeliveredState implements OrderState {
    @Override
    public void pay(OrderContext context) {
        throw new IllegalStateException("이미 배송 완료된 주문이다.");
    }

    @Override
    public void deliver(OrderContext context) {
        throw new IllegalStateException("이미 배송 완료된 주문이다.");
    }

    @Override
    public void cancel(OrderContext context) {
        throw new IllegalStateException("배송 완료된 주문은 취소할 수 없다.");
    }
}

public class CanceledState implements OrderState {
    @Override
    public void pay(OrderContext context) {
        throw new IllegalStateException("취소된 주문이다.");
    }

    @Override
    public void deliver(OrderContext context) {
        throw new IllegalStateException("취소된 주문이다.");
    }

    @Override
    public void cancel(OrderContext context) {
        throw new IllegalStateException("이미 취소된 주문이다.");
    }
}
```

클라이언트는 상태 객체를 모른 채 컨텍스트에만 요청합니다. 허용되지 않는 전이는 예외로 차단됩니다.

{% include code-header.html %}
```java
OrderContext order = new OrderContext();
order.pay();      // 결제 대기 -> 배송 중
order.cancel();   // IllegalStateException: 배송 중인 주문은 취소할 수 없다.
order.deliver();  // 배송 중 -> 배송 완료
```

위 예시는 전이마다 `new`로 상태 객체를 만듭니다. 상태 객체가 필드를 갖지 않으면 매번 생성하지 않고 싱글턴으로 공유해도 됩니다. 상태가 컨텍스트의 데이터를 써야 한다면 생성자로 컨텍스트를 주입받습니다.

# 장단점

**장점**

- **단일 책임 원칙(SRP) 준수**: 상태별 동작과 상태 전이 코드를 개별 상태 클래스로 분리해 상태마다 관심사를 독립적으로 관리할 수 있습니다.
- **개방-폐쇄 원칙(OCP) 준수**: 기존 상태 클래스나 컨텍스트를 수정하지 않고 새 상태 클래스를 추가해 확장할 수 있습니다.
- **조건 분기문 제거**: 컨텍스트 안의 상태 확인용 `if-else`, `switch`를 상태 객체에 대한 위임 호출로 바꿔 가독성과 유지보수성을 높입니다.
- **상태 전이의 명시화 및 무결성 확보**: 상태를 원시값이 아닌 객체로 캡슐화하므로 허용되지 않는 전이를 막기 쉽고, 상태 변화 흐름을 코드에서 바로 파악할 수 있습니다.

**단점**

- **클래스 증가에 따른 오버엔지니어링**: 상태가 몇 개 안 되고 전이 규칙이 단순하면 상태마다 클래스를 만드는 것이 오히려 복잡도와 파일 수만 늘립니다.
- **상태 클래스 간 결합과 순환 참조**: 다음 상태로 전이하려고 구상 상태 클래스를 직접 생성(`new NextState()`)하거나 컨텍스트와 양방향 의존을 맺기 쉽습니다.
- **미지원 이벤트의 빈 메서드 부담**: 공통 상태 인터페이스에 모든 이벤트 메서드가 선언되므로, 처리할 필요 없는 이벤트도 빈 메서드나 예외 처리로 구현해야 합니다.

---

지금까지 살펴본 내용을 기준으로 상태 패턴을 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 객체의 상태가 많고 상태에 따라 같은 이벤트의 동작이 달라진다 | 쓴다 | 상태별 동작을 상태 클래스로 분리하면 새 상태를 추가하거나 로직을 고쳐도 다른 상태의 코드에 영향을 주지 않는다(SRP, OCP) |
| 상태를 검사하는 `if-else`, `switch`가 메서드마다 반복된다 | 쓴다 | 분기마다 흩어진 코드를 상태 클래스로 옮기면 컨텍스트는 위임만 하므로 조건문과 상태 확인용 필드가 사라진다 |
| 허용되지 않는 상태 전이를 코드 수준에서 막아야 한다 | 쓴다 | 상태가 객체로 캡슐화되어 상태마다 허용하는 이벤트와 전이가 한 클래스에 모이므로 전이 규칙을 바로 파악할 수 있다 |
| 상태 간에 중복되는 로직이나 공통 전이 규칙이 많다 | 상태 계층으로 쓴다 | 중간 추상 클래스로 공통 행동을 한곳에 모으면 상태 클래스 간 중복을 줄일 수 있다 |
| 상태가 두세 개이고 전이 규칙이 단순하며 변경 가능성이 낮다 | 피한다 | 상태마다 클래스를 만들면 파일 수와 복잡도만 늘어 조건 분기가 더 직관적이다 |
| 상태 전이가 없고 클라이언트가 알고리즘을 골라 주입한다 | 전략을 검토한다 | 구조는 비슷하지만 전략은 구상 전략끼리 서로를 모르고 선택을 외부가 하며, 상태는 상태 객체가 스스로 다음 상태로 전이시킨다 |
| 상태 객체가 다음 구상 상태를 직접 생성(`new`)하고 컨텍스트와 양방향으로 의존한다 | 구조를 검토한다 | 상태 클래스 간 결합과 순환 참조가 생기기 쉬우므로 전이 규칙을 컨텍스트나 별도 테이블로 모으는 방법을 따져 본다 |
| 상태 객체가 필드를 갖지 않는다 | 공유를 검토한다 | 전이마다 `new`로 만들지 않고 싱글턴으로 공유하면 객체 생성 비용을 줄일 수 있다 |

정리하면, 상태 패턴은 상태에 따라 객체의 행동이 달라지고 상태별 분기문이 반복될 때 상태별 동작과 전이 규칙을 상태 클래스로 분리하는 패턴입니다. 상태가 적고 전이가 단순하다면 조건 분기를 유지하는 편이 낫고, 전이 없이 알고리즘만 바꾼다면 전략이 낫습니다.

# 참고 자료

[**상태 패턴**](https://refactoring.guru/ko/design-patterns/state)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
