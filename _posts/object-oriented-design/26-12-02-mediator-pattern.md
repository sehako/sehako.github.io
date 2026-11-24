---
title: 중재자 패턴 (구조와 옵서버 패턴과의 차이)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-12-02

permalink: "/object-oriented-design/mediator-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 중재자 패턴을 다룹니다. 로그인 대화상자 예시로 구조와 구현 방법을 살펴보고, 중재자 패턴의 장단점과 옵서버 패턴과의 차이를 정리하겠습니다.

# 중재자 패턴이란

객체들이 서로를 직접 참조하며 상호작용하는 대신, 상호작용을 캡슐화한 중재자 객체에 위임하게 하여 객체 간의 직접적인 의존을 없애는 행동 디자인 패턴입니다.

여러 객체가 서로 직접 통신하면 객체마다 다른 모든 객체를 알아야 하는 다대다(M:N) 의존 관계가 생기고 결합도가 높아집니다. 중재자 패턴은 중앙에 중재자를 두고 모든 객체가 중재자하고만 통신하게 해서, 관계를 중재자를 중심으로 한 일대다(1:N) 스타형 구조로 단순화합니다.

항공 관제탑이 대표적인 비유입니다. 비행기끼리 무전으로 직접 통신하면 통신망이 복잡해지지만, 모든 비행기가 관제탑하고만 통신하고 관제탑이 경로와 일정을 조정하면 구조가 단순해집니다.

# 활용 상황

중재자 패턴은 주로 다음 상황에서 사용됩니다.

- 다수의 객체가 서로 복잡하게 얽혀 통신하여 결합도가 높고 코드 수정과 재사용이 어려운 경우

복잡한 GUI 대화상자처럼 체크박스를 클릭하면 텍스트 필드가 활성화되고, 텍스트 필드가 채워져야 확인 버튼이 활성화되는 식으로 컴포넌트 간 상호작용이 얽혀 있을 때 사용합니다. 각 컴포넌트가 서로를 참조하지 않고 대화상자(중재자)에게만 이벤트를 전달하도록 설계하면 컴포넌트를 다른 화면에서도 쉽게 재사용할 수 있습니다.

- 한 객체의 상태 변화가 여러 객체의 동작에 영향을 주고, 그 동작의 실행 순서나 업무 흐름을 중앙에서 조정해야 하는 경우

항공 관제나 복잡한 결제·주문 처리 흐름처럼 참여자의 동작 순서에 엄격한 규칙이 필요할 때 적합합니다. 순서를 제어하는 로직을 중재자 클래스에 캡슐화하면 참여 객체를 수정하지 않고도 처리 절차를 일관되게 관리할 수 있습니다.

# 구조

중재자 패턴은 다음과 같은 구조를 가집니다.

- 중재자(Mediator) 인터페이스는 동료 객체와 통신하며 상호작용을 조정하는 공통 규격을 선언합니다. `notify(sender, event)`가 예입니다.
- 구상 중재자(Concrete Mediator)는 여러 동료 객체에 대한 참조를 유지하고, 객체 간의 상호작용 관계와 실행 흐름, 조정 로직을 캡슐화합니다.
- 동료 객체(Colleague, Component)는 각자의 고유 기능에만 집중하는 단위 객체입니다. 대화상자의 버튼, 텍스트 필드, 체크박스가 예입니다. 다른 동료 객체의 존재를 알지 못하고, 상태가 바뀌거나 이벤트가 발생하면 자신이 아는 중재자에게만 알립니다.

## 구현 방법

아이디와 비밀번호를 모두 입력해야 로그인 버튼이 활성화되는 로그인 대화상자 예시입니다. 텍스트 박스와 버튼은 서로를 모르고, 대화상자(중재자)에게만 이벤트를 알립니다.

`Mediator`는 중재자 인터페이스입니다. `Component`는 동료 객체의 기초 클래스로 중재자에 대한 참조만 가집니다.

{% include code-header.html %}
```java
public interface Mediator {
    void notify(Component sender, String event);
}

public abstract class Component {
    protected Mediator mediator;

    public void setMediator(Mediator mediator) {
        this.mediator = mediator;
    }
}
```

`TextBox`와 `Button`은 동료 객체입니다. 상태가 바뀌거나 클릭되면 중재자에게 이벤트를 알릴 뿐 다른 컴포넌트는 참조하지 않습니다.

{% include code-header.html %}
```java
public class TextBox extends Component {
    private String text = "";

    public void setText(String text) {
        this.text = text;
        mediator.notify(this, "textChanged");
    }

    public String getText() {
        return text;
    }

    public boolean isEmpty() {
        return text.isBlank();
    }
}

public class Button extends Component {
    private boolean enabled = false;

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public void click() {
        if (enabled) {
            mediator.notify(this, "click");
        }
    }
}
```

`LoginDialog`는 구상 중재자입니다. 모든 동료 객체를 참조하며 생성자에서 자신을 중재자로 등록합니다. 컴포넌트 간의 상호작용 규칙은 `notify()` 한 곳에 모여 있습니다. 텍스트가 바뀌면 두 입력창이 모두 채워졌는지에 따라 버튼을 활성화하고, 버튼이 클릭되면 로그인을 처리합니다.

{% include code-header.html %}
```java
public class LoginDialog implements Mediator {
    private final TextBox usernameBox;
    private final TextBox passwordBox;
    private final Button submitButton;

    public LoginDialog(TextBox usernameBox, TextBox passwordBox, Button submitButton) {
        this.usernameBox = usernameBox;
        this.passwordBox = passwordBox;
        this.submitButton = submitButton;

        usernameBox.setMediator(this);
        passwordBox.setMediator(this);
        submitButton.setMediator(this);
    }

    @Override
    public void notify(Component sender, String event) {
        if (event.equals("textChanged")) {
            submitButton.setEnabled(!usernameBox.isEmpty() && !passwordBox.isEmpty());
        } else if (event.equals("click")) {
            login(usernameBox.getText(), passwordBox.getText());
        }
    }

    private void login(String username, String password) {
        // 로그인 처리
    }
}
```

클라이언트는 컴포넌트를 만들고 중재자에 연결합니다.

{% include code-header.html %}
```java
public class Client {
    public static void main(String[] args) {
        TextBox usernameBox = new TextBox();
        TextBox passwordBox = new TextBox();
        Button submitButton = new Button();
        new LoginDialog(usernameBox, passwordBox, submitButton);

        usernameBox.setText("user"); // 버튼 비활성화 유지
        submitButton.click();        // 아무 일도 일어나지 않음

        passwordBox.setText("pw");   // 버튼 활성화
        submitButton.click();        // login("user", "pw") 호출
    }
}
```

`Button`과 `TextBox`에는 특정 대화상자의 규칙이 들어 있지 않으므로 다른 화면에서도 그대로 재사용할 수 있습니다. 새로운 컴포넌트가 생기면 `LoginDialog`의 `notify()`만 수정합니다.

# 장단점

중재자 패턴의 장단점은 다음과 같습니다.

**장점**

- **단일 책임 원칙(SRP) 준수와 결합도 감소**: 개별 컴포넌트는 고유한 작업에만 집중하고, 객체 간 통신과 협업 로직은 중재자로 분리되어 상호 의존이 사라집니다.
- **개방-폐쇄 원칙(OCP) 준수**: 새 컴포넌트를 추가하거나 기존 컴포넌트를 바꿀 때 다른 컴포넌트를 수정하지 않고 중재자만 조정하면 됩니다.
- **객체 관계의 단순화**: 객체 N개가 서로 직접 연결되면 최대 N×(N−1)/2개의 연결이 필요하지만, 중재자를 두면 N개의 연결로 줄어듭니다.

**단점**

- **중재자가 전지전능한 객체(God Object)가 될 위험**: 모든 컴포넌트의 상호작용과 조정 로직이 한곳에 모이므로 중재자 클래스가 지나치게 커지고 복잡해져 유지보수하기 어려워질 수 있습니다. 컴포넌트 간 상호작용이 실제로 복잡하고 관리 비용이 클 때만 제한적으로 도입합니다.

# 옵서버 패턴과의 차이

두 패턴 모두 컴포넌트가 중간 매개체와 상호작용하여 결합도를 낮춘다는 점은 같지만 성격이 다릅니다.

| 구분 | 중재자 패턴 | 옵서버 패턴 |
| --- | --- | --- |
| 상호작용 방향 | 양방향. 동료 객체는 메시지를 보내는 주체이면서 중재자의 통제를 받는 수신자이기도 하다. | 단방향. 참여자는 대부분 이벤트를 발행하는 쪽이거나 받기만 하는 쪽 중 하나의 역할만 한다. |
| 조정과 순서 제어 | 참여자의 상태 변경에 따라 다른 참여자의 메서드를 특정 순서로 호출하며 실행 흐름을 조정할 수 있다. | 상태 변경을 구독자에게 전파할 뿐이며, 구독자 간의 실행 순서를 제어하거나 조정하지 않는다. |
| 중심 객체의 성격 | 컴포넌트 간 협업 로직을 중앙에 집중해 처리한다. | 이벤트를 구독자 목록에 전달하는 디스패칭에 집중한다. |

---

정리하면, 중재자는 여러 객체가 서로 얽혀 직접 통신하며 결합도가 높을 때 상호작용과 조정 로직을 중재자 한 곳으로 모으는 패턴입니다. 상호작용이 단순하다면 직접 호출을 유지하는 편이 낫고, 중재자가 모든 조정 로직을 떠안아 비대해지지 않도록 주의합니다. 단순한 상태 변경 통보라면 중재자 대신 옵서버로 충분합니다.

# 참고 자료

[**중재자 패턴**](https://refactoring.guru/ko/design-patterns/mediator)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
