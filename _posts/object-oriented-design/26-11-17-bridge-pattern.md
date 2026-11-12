---
title: 브리지 패턴 (구조와 합성을 통한 계층 분리)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-17

permalink: "/object-oriented-design/bridge-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 브리지를 다룹니다. 구조와 기기, 리모컨 예제를 살펴본 뒤, 상속 대신 합성을 적용하면 왜 브리지 구조에 도달하는지, 호환되지 않는 대상을 사후에 연결하는 [어댑터 패턴](/object-oriented-design/adapter-pattern/)과는 무엇이 다른지 정리하겠습니다.

# 브리지 패턴이란

밀접하게 관련된 클래스들의 집합을 두 개의 개별 계층구조인 추상화(고수준 제어)와 구현(저수준 실행)으로 나누어 각각 독립적으로 개발할 수 있도록 하는 구조 디자인 패턴입니다.

조금 더 쉽게 설명하자면, 하나의 클래스에 독립적으로 변하는 기능(차원)이 둘 이상 존재할 때, 이들을 각각 독립된 인터페이스와 구현체로 분리한 뒤 원본 클래스에서 합성을 통해 유연하게 조합하여 사용할 수 있게 하는 패턴입니다.

주로 다음 상황에서 사용합니다.

- 기능 변형이 너무 많은 거대한 클래스를 나누고 정돈하려 하는 경우

예를 들어 비즈니스 로직 이후에 데이터베이스 하나와 상호작용하는 클래스가 있을 때, 지원 데이터베이스가 늘어날수록 클래스가 비대해지고 분기가 복잡해집니다.

여기에 브리지 패턴을 도입해 비즈니스 로직을 담당하는 추상화 계층과 데이터베이스와 통신을 담당하는 구현 계층을 분리하면, 거대한 클래스의 복잡도를 낮추고 각 계층의 유지보수가 용이해집니다.

- 여러 독립 차원에서 클래스를 확장해야 하는 경우

서로 각각의 이유로 변경되는 2개 이상의 요구사항에 대해 상속 대신 별도 계층 구조로 추출 및 합성하는 것을 통해 상속의 폭발 문제를 방지할 수 있습니다.

- 런타임에 구현체를 전환해야 하는 경우

객체 합성 방식은 컴파일 타임에 관계가 고정되는 상속과 다르게 런타임에 구현 대상을 동적으로 교체하거나 주입할 수 있습니다.

# 구조

브리지 패턴은 다음과 같은 구조를 가집니다.

- 추상화는 고수준 제어 논리를 제공하며, 구현체에 의존해 저수준 작업을 실행합니다.
- 구현체는 모든 구상 구현들에 대한 공통 인터페이스를 가지고 있습니다.
- 클라이언트는 추상화와 상호작용하여 작업을 진행합니다.

# 구현 예시

기기와 리모컨을 관리하는 앱의 예시입니다. 기기(`Device`)가 구현 역할을, 리모컨(`RemoteControl`)이 추상화 역할을 맡습니다. 두 계층을 나누면 리모컨과 기기를 각각 독립적으로 확장할 수 있고, 같은 리모컨이 여러 기기 유형을 지원할 수 있습니다.

구현 인터페이스는 모든 구상 구현 클래스에 공통인 메서드를 선언합니다. 추상화의 인터페이스와 일치할 필요는 없으며, 보통 구현 인터페이스는 원시 작업만 제공하고 추상화가 이를 기반으로 더 상위 수준의 작업을 정의합니다.

{% include code-header.html %}
```java
// 구현 인터페이스: 모든 기기가 따르는 원시 작업을 선언한다.
public interface Device {
    boolean isEnabled();
    void enable();
    void disable();
    int getVolume();
    void setVolume(int percent);
    int getChannel();
    void setChannel(int channel);
}
```

모든 구상 기기는 같은 인터페이스를 구현합니다.

{% include code-header.html %}
```java
// 구상 구현 클래스
public class Tv implements Device {
    // ...
}

public class Radio implements Device {
    // ...
}
```

추상화는 구현 계층구조의 객체에 대한 참조를 유지하고, 실제 작업을 이 객체에 위임합니다. 이 참조 필드가 두 계층구조를 연결하는 다리입니다.

{% include code-header.html %}
```java
// 추상화: 제어 로직을 제공하고 실제 작업은 Device에 위임한다.
public class RemoteControl {
    protected final Device device;

    public RemoteControl(Device device) {
        this.device = device;
    }

    public void togglePower() {
        if (device.isEnabled()) {
            device.disable();
        } else {
            device.enable();
        }
    }

    public void volumeDown() {
        device.setVolume(device.getVolume() - 10);
    }

    public void volumeUp() {
        device.setVolume(device.getVolume() + 10);
    }

    public void channelDown() {
        device.setChannel(device.getChannel() - 1);
    }

    public void channelUp() {
        device.setChannel(device.getChannel() + 1);
    }
}
```

추상화 계층구조의 클래스는 기기 클래스와 독립적으로 확장할 수 있습니다. 새 리모컨 하위 클래스만 만들면 됩니다.

{% include code-header.html %}
```java
// 확장된 추상화: 기기 클래스를 수정하지 않고 기능을 추가한다.
public class AdvancedRemoteControl extends RemoteControl {
    public AdvancedRemoteControl(Device device) {
        super(device);
    }

    public void mute() {
        device.setVolume(0);
    }
}
```

클라이언트는 생성자를 통해 원하는 유형의 리모컨을 특정 기기와 연결합니다.

{% include code-header.html %}
```java
// 클라이언트 코드
public class Application {
    public void example() {
        Device tv = new Tv();
        RemoteControl remote = new RemoteControl(tv);
        remote.togglePower();

        Device radio = new Radio();
        AdvancedRemoteControl advancedRemote = new AdvancedRemoteControl(radio);
        advancedRemote.mute();
    }
}
```

# 장단점

브리지 패턴의 장단점은 다음과 같습니다.

**장점**

- 플랫폼 독립적인 클래스들과 앱들을 만들 수 있습니다.
- 클라이언트 코드는 상위 수준의 추상화를 통해 동작하기 때문에 플랫폼 세부 정보에 노출되지 않습니다.
- 새로운 추상화들과 구현들을 상호 독립적으로 도입할 수 있어 개방-폐쇄 원칙을 준수할 수 있게 됩니다.
- 추상화의 상위 수준 논리와 구현의 플랫폼 세부 정보에 집중할 수 있어 단일 책임 원칙을 준수할 수 있게 됩니다.

**단점**

- 결합도가 높은 클래스에 패턴을 적용하면 코드의 복잡성이 증가할 수 있습니다.

# 합성과 브리지 패턴

사실 브리지 패턴은 특별한 기법이 아닌, "상속보다 합성"과 단일 책임 원칙을 지키다 보면 자연스럽게 도달하는 구조라고 볼 수 있습니다.

차량에 엔진 종류를 상속으로 붙이는 경우를 예로 들면 다음과 같은 흐름입니다.

1. 상속으로 풀면 `전기세단`, `가솔린세단`, `전기SUV`처럼 조합마다 클래스가 생겨 관리하기 어려워집니다.
2. 상속으로 클래스를 늘리는 대신 엔진을 `Engine` 인터페이스로 분리하고, 차량이 필드로 주입받아 사용하도록 바꿉니다.
3. 차량의 형태(세단, SUV)는 차량 계층에서, 엔진(전기, 가솔린)은 엔진 계층에서 각각 확장합니다.

이렇게 정리한 코드는 추상화(차량)와 구현(엔진)이 참조로 연결된 브리지 패턴의 구조와 같습니다.

그럼에도 GoF가 이 구조에 이름을 붙인 이유는 다음과 같이 해석할 수 있습니다.

- 기능을 확장할 때 일단 상속부터 쓰기 쉽습니다. 서로 다른 이유로 변하는 두 기능 축이 보이면 상속하지 말고 다리를 놓으라는 신호가 됩니다.
- 설계 의도를 짧게 전달하는 용어가 됩니다. "서로 다른 변경 이유를 가진 두 기능을 독립된 인터페이스로 분리해 주입하는 구조로 바꾸자"를 "브리지 패턴으로 리팩터링하자"로 말할 수 있습니다.

---

지금까지 살펴본 내용을 기준으로 브리지를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황                                          | 판단        | 근거                                                   |
| ------------------------------------------- | --------- | ---------------------------------------------------- |
| 한 클래스에 독립적으로 변하는 기능(차원)이 둘 이상 있고 각각 확장해야 한다 | 쓴다        | 차원별로 인터페이스와 구현체를 나누고 합성으로 조합하므로 하위 클래스가 곱으로 늘어나지 않는다 |
| 추상화와 구현을 서로 영향 없이 각각 확장해야 한다                | 쓴다        | 새 추상화와 새 구현을 독립적으로 도입할 수 있어 개방-폐쇄 원칙을 지킨다            |
| 런타임에 구현체를 바꿔야 한다                            | 쓴다        | 추상화가 구현 인터페이스에 대한 참조만 가지므로 다른 구현 객체로 교체할 수 있다        |
| 클라이언트 코드를 플랫폼 세부 정보와 분리하고 싶다                | 쓴다        | 클라이언트는 추상화만 다루고 플랫폼별 작업은 구현 계층이 맡는다                  |
| 이미 만들어진 호환되지 않는 인터페이스를 맞춰야 한다               | 어댑터를 검토한다 | 브리지는 처음부터 두 계층으로 나눠 설계하는 패턴이고, 어댑터는 기존 코드를 사후에 연결한다  |
| 변하는 차원이 하나뿐이다                               | 피한다       | 추상화와 구현을 나눠도 얻는 것 없이 클래스와 인터페이스만 늘어난다                |
| 결합도가 높아 나누기 어려운 클래스다                        | 피한다       | 패턴을 적용해도 분리가 깔끔하지 않아 코드의 복잡성만 증가할 수 있다               |

정리하면, 브리지는 독립적으로 변하는 두 기능을 추상화와 구현의 두 계층으로 나누고 합성으로 연결하는 패턴입니다. 상속으로 조합마다 클래스를 늘리는 대신 각 계층을 따로 확장할 수 있다는 점이 핵심이며, 처음부터 두 계층으로 설계한다는 점에서 이미 만들어진 대상을 사후에 맞추는 어댑터와 다릅니다.

# 참고 자료

[**브리지 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/bridge)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
