---
title: 옵서버 패턴 (구조와 Spring 이벤트)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-21

permalink: "/object-oriented-design/observer-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 옵서버를 다룹니다. 회원가입 후속 처리 예제로 피관찰자와 관찰자를 나누는 방법을 살펴보고, Spring 이벤트가 옵서버 패턴과 어떻게 이어지고 어디서 달라지는지 정리하겠습니다.

# 옵서버 패턴이란

객체의 상태가 변경되었을 때 해당 객체에 의존하고 있는 모든 객체가 이러한 변경을 감지할 수 있도록 하는 행동 디자인 패턴입니다. 이러한 특성 때문에 발행-구독 패턴이라고도 부릅니다.

의존 대상이 되는 객체를 피관찰자(Observable) 또는 발행자(Publisher)라고 하고, 의존하고 있는 객체를 관찰자(Observer) 또는 구독자(Subscriber)라고 합니다.

# 활용 상황

옵서버 패턴은 주로 다음 상황에서 사용됩니다.

- 객체의 상태 변경을 반영해서 다른 객체의 상태를 변경해야 할 때, 이러한 의존 객체들의 집합이 동적으로 결정되어야 하는 경우

실시간 채팅 세션에 접속하는 상황을 가정해보겠습니다. 이 경우 사용자가 선택한 채팅 세션에 따라서 실시간으로 등록된 채팅 메시지를 전달해야 합니다.

여기에 옵서버 패턴을 도입하여 채팅 세션 객체를 피관찰자, 사용자 객체를 관찰자로 만들어 사용자가 선택한 채팅 세션을 관찰하도록 만듭니다. 이렇게 하면 관찰하는 채팅 세션에 메시지가 추가되었을 때 사용자가 알림을 받도록 할 수 있습니다.

- 일부 객체들이 제한된 시간 동안 또는 특정 경우에만 다른 객체들을 관찰해야 하는 경우

사용자들은 원하는 채팅 세션에 접속(관찰)하거나 접속 중단(관찰 중지)을 할 수 있습니다.

# 구조

옵서버 패턴은 다음과 같은 구조를 가집니다.

- 피관찰자와 관찰자를 정의하는 두 개의 인터페이스를 제공합니다.
- 피관찰자에서 관찰자에게 알림을 전달하거나 관찰자가 변경을 주기적으로 확인할 수 있는 수단을 정의합니다.
- 구상 피관찰자는 관찰자들을 관리하는 수단과, 알림 전달과 관리에 대한 하위 수준의 개념을 정의합니다.
- 클라이언트에서 피관찰자와 관찰자를 조합하여 사용합니다.

## 구현 방법

회원가입이 성공한 뒤 프로모션 지급과 환영 메시지 발송 같은 후속 작업을 처리하는 예시입니다. 후속 작업을 컨트롤러에 직접 호출하면 작업이 늘어날 때마다 컨트롤러를 수정해야 합니다.

{% include code-header.html %}
```java
// 적용 전: 후속 작업이 컨트롤러에 직접 결합되어 있다.
public class UserController {
    private UserService userService;
    private PromotionService promotionService;
    private NotificationService notificationService;

    public Long register(String telephone, String password) {
        long userId = userService.register(telephone, password);

        promotionService.issueNewUserExperienceCash(userId);
        notificationService.sendWelcomeMessage(userId);

        return userId;
    }
}
```

관찰자 인터페이스는 가입 성공 알림을 받는 쪽이 구현할 메서드를 선언합니다.

{% include code-header.html %}
```java
// 관찰자: 회원가입 성공 알림을 받는 쪽이 구현한다.
public interface RegObserver {
    void handleRegSuccess(long userId);
}
```

피관찰자 인터페이스는 관찰자를 등록하고 해제하며 알리는 수단을 선언합니다. 해제 수단이 있어서 관찰을 중단할 수 있습니다.

{% include code-header.html %}
```java
// 피관찰자: 관찰자를 관리하고 알림을 전달하는 수단을 선언한다.
public interface RegSubject {
    void registerObserver(RegObserver observer);
    void removeObserver(RegObserver observer);
    void notifyObservers(long userId);
}
```

구상 관찰자는 각자 하나의 후속 작업만 맡습니다.

{% include code-header.html %}
```java
// 구상 관찰자: 신규 회원 프로모션 지급
public class RegPromotionObserver implements RegObserver {
    private final PromotionService promotionService;

    public RegPromotionObserver(PromotionService promotionService) {
        this.promotionService = promotionService;
    }

    @Override
    public void handleRegSuccess(long userId) {
        promotionService.issueNewUserExperienceCash(userId);
    }
}

// 구상 관찰자: 환영 메시지 발송
public class RegNotificationObserver implements RegObserver {
    private final NotificationService notificationService;

    public RegNotificationObserver(NotificationService notificationService) {
        this.notificationService = notificationService;
    }

    @Override
    public void handleRegSuccess(long userId) {
        notificationService.sendWelcomeMessage(userId);
    }
}
```

구상 피관찰자인 컨트롤러는 관찰자를 인터페이스 타입으로 보관합니다. 핵심 로직인 가입이 끝나면 `notifyObservers()`로 알리기만 하고, 어떤 후속 작업이 있는지 알지 못합니다.

{% include code-header.html %}
```java
// 구상 피관찰자: 가입 성공 후 등록된 관찰자들에게 알린다.
public class UserController implements RegSubject {
    private final UserService userService;
    private final List<RegObserver> observers = new ArrayList<>();

    public UserController(UserService userService) {
        this.userService = userService;
    }

    @Override
    public void registerObserver(RegObserver observer) {
        observers.add(observer);
    }

    @Override
    public void removeObserver(RegObserver observer) {
        observers.remove(observer);
    }

    @Override
    public void notifyObservers(long userId) {
        for (RegObserver observer : observers) {
            observer.handleRegSuccess(userId);
        }
    }

    public Long register(String telephone, String password) {
        long userId = userService.register(telephone, password);
        notifyObservers(userId);
        return userId;
    }
}
```

클라이언트에서 피관찰자와 관찰자를 조합합니다. 통계 수집 같은 후속 작업이 추가되어도 `RegObserver` 구현체를 만들어 등록하면 되고, `register()`는 바뀌지 않습니다.

{% include code-header.html %}
```java
// 클라이언트 코드: 피관찰자와 관찰자를 조합한다.
public class Application {
    public static void main(String[] args) {
        UserController controller = new UserController(userService);

        controller.registerObserver(new RegPromotionObserver(promotionService));
        controller.registerObserver(new RegNotificationObserver(notificationService));

        controller.register("010-1234-5678", "password");
    }
}
```

# 장단점

옵서버 패턴의 장단점은 다음과 같습니다.

**장점**

- 피관찰자의 코드 변경 없이 새로운 관찰자를 확장할 수 있으며, 피관찰자에 추상화(인터페이스)를 도입할 경우 관찰자의 코드 변경 없이 피관찰자를 확장할 수 있어 개방-폐쇄 원칙을 준수할 수 있습니다.
- 런타임에 객체 간 관계를 동적으로 설정할 수 있습니다.

**단점**

- 관찰자 간 독립성을 전제하므로 변경에 대한 알림의 순서를 보장하지 않습니다. 따라서 관찰자 간 실행 순서에 의존하도록 설계하기 어렵습니다.
- 멀티 스레딩을 도입하지 않으면, 순차적으로 관찰자의 행동이 이루어지므로 처리 지연이나 예외 발생 시의 처리가 난해합니다.
- 관찰자의 참조를 명시적으로 해제하지 않으면 메모리가 회수되지 않을 수 있습니다.

# Spring 이벤트와 옵서버 패턴

Spring의 `ApplicationContext` 이벤트 처리는 옵서버 패턴을 프레임워크 수준에서 구현한 것입니다. 앞의 회원가입 예시에서 `RegSubject`, `RegObserver`, 관찰자 목록 관리를 직접 구현했다면, Spring에서는 이 부분을 프레임워크가 맡습니다.

## 구성 요소 대응

| 옵서버 패턴 | Spring 이벤트 | 설명 |
| --- | --- | --- |
| 피관찰자(Subject) | `ApplicationEventPublisher` | 이벤트를 발행합니다. `publishEvent()`를 호출하는 것으로 책임이 끝납니다. |
| 관찰자(Observer) | `ApplicationListener<E>`, `@EventListener` 메서드 | 발행된 이벤트를 수신해 처리합니다. |
| 알림 데이터 | `ApplicationEvent` 또는 임의의 객체 | 발행자에서 구독자로 전달되는 데이터입니다. |
| 관찰자 목록 관리와 알림 전파 | `ApplicationEventMulticaster` | 리스너 목록을 관리하고 이벤트를 전달합니다. |

발행자는 어떤 리스너가 있는지 알지 못하므로 발행자와 구독자의 결합이 끊어진다는 점은 같습니다.

회원가입 예시를 Spring 이벤트로 바꾸면 다음과 같습니다. 발행자인 `UserService`는 관찰자 목록을 가지지 않고, 관찰자도 인터페이스 구현 없이 메서드에 애너테이션만 붙입니다.

{% include code-header.html %}
```java
// 이벤트: 회원가입 성공 사실과 필요한 데이터를 담는다.
public record RegSuccessEvent(long userId) {
}

// 발행자: 가입 후 이벤트만 발행한다.
@Service
public class UserService {
    private final ApplicationEventPublisher publisher;

    public UserService(ApplicationEventPublisher publisher) {
        this.publisher = publisher;
    }

    public long register(String telephone, String password) {
        long userId = save(telephone, password);
        publisher.publishEvent(new RegSuccessEvent(userId));
        return userId;
    }
}

// 구독자: 메서드에 @EventListener만 붙이면 관찰자가 된다.
@Component
public class RegPromotionListener {
    @EventListener
    public void handle(RegSuccessEvent event) {
        // 신규 회원 프로모션 지급
    }
}
```

## 고전 옵서버와의 차이

- 인터페이스 의존 제거: 고전 옵서버는 `Observer` 인터페이스를 구현해야 합니다. Spring 4.2부터는 일반 클래스의 메서드에 `@EventListener`를 붙이면 리스너가 되고, 이벤트도 `ApplicationEvent`를 상속하지 않은 객체를 발행할 수 있습니다.
- 실행 순서 지정: 고전 옵서버는 알림 순서를 보장하지 않습니다. Spring은 리스너에 `@Order`를 붙여 실행 순서를 지정할 수 있습니다.
- 조건부 수신: `@EventListener`의 `condition` 속성에 SpEL을 지정하면 조건을 만족하는 이벤트만 수신합니다.

## 동기와 비동기

Spring 이벤트는 기본적으로 동기로 실행됩니다. 옵서버 패턴의 단점인 순차 실행에 따른 처리 지연과 예외 처리 문제가 그대로 나타납니다.

- 동기(기본): `publishEvent()`를 호출하면 모든 리스너의 처리가 끝날 때까지 발행자가 대기합니다. 발행자의 스레드와 트랜잭션을 리스너가 그대로 공유하므로 흐름은 단순하지만, 리스너가 지연되면 발행자도 함께 지연됩니다.
- 비동기: 리스너에 `@Async`를 붙이거나 `ApplicationEventMulticaster`에 `TaskExecutor`를 설정하면 별도 스레드에서 실행됩니다. 지연은 해소되지만 다음을 감수해야 합니다.
  - 리스너의 예외가 발행자에게 전파되지 않습니다.
  - `ThreadLocal`과 보안, 로깅 컨텍스트가 기본적으로 전파되지 않습니다.
  - 발행자의 트랜잭션과 리스너의 트랜잭션이 분리됩니다.

## 트랜잭션과 이벤트

발행자가 `@Transactional` 안에서 이벤트를 발행하는 경우를 구분해야 합니다.

- `@EventListener`(동기): 리스너가 발행자와 같은 트랜잭션에서 실행됩니다. 리스너에서 런타임 예외가 발생하면 발행자의 트랜잭션도 롤백됩니다. 부가 작업인 알림 발송이 실패했는데 핵심 작업인 가입 또는 결제까지 롤백되는 문제가 생길 수 있습니다.
- `@TransactionalEventListener`: 발행자의 트랜잭션 단계에 맞춰 리스너 실행 시점을 정합니다. 기본값인 `AFTER_COMMIT`은 트랜잭션이 커밋된 뒤에만 리스너를 실행하므로, 롤백된 작업에 대해 알림이 나가는 불일치를 막습니다.

`AFTER_COMMIT` 리스너는 이미 커밋된 이후에 실행되므로 리스너 안에서 DB를 변경하려면 새 트랜잭션(`REQUIRES_NEW`)이 필요합니다.

---

지금까지 살펴본 내용을 기준으로 옵서버를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 한 객체의 상태 변경에 반응할 객체 집합이 동적으로 결정된다 | 쓴다 | 피관찰자는 관찰자 인터페이스에만 의존하고, 관찰자는 런타임에 등록하고 해제할 수 있다 |
| 일부 객체가 제한된 시간 동안 또는 특정 경우에만 다른 객체를 관찰한다 | 쓴다 | 관찰자를 등록하고 해제하는 수단으로 관찰의 시작과 중단을 제어한다 |
| 핵심 로직 뒤에 붙는 후속 작업이 계속 늘어난다 | 쓴다 | 후속 작업을 관찰자로 분리하면 새 작업을 추가해도 피관찰자의 코드를 바꾸지 않는다 |
| 알림을 받는 쪽의 실행 순서에 의존해야 한다 | 피한다 | 관찰자 간 독립성을 전제하므로 알림 순서를 보장하지 않는다. Spring 이벤트를 쓴다면 `@Order`로 지정할 수 있다 |
| 후속 작업이 느리거나 실패해도 핵심 로직에 영향을 주면 안 된다 | 비동기를 검토한다 | 기본 동기 방식에서는 관찰자의 지연과 예외가 피관찰자에게 그대로 미친다. 비동기로 분리하면 예외 전파와 트랜잭션 분리를 감수해야 한다 |
| 핵심 트랜잭션이 커밋된 뒤에만 후속 작업을 실행해야 한다 | `@TransactionalEventListener`를 검토한다 | `AFTER_COMMIT`은 커밋 이후에만 리스너를 실행해 롤백된 작업에 대한 알림을 막는다 |

# 참고 자료

[**옵서버 패턴**](https://refactoring.guru/ko/design-patterns/observer)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
