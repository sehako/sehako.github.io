---
title: 팩토리 메서드 패턴 (구조와 단순 팩토리와의 차이)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-10

permalink: "/object-oriented-design/factory-method-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 생성 패턴 편입니다. 이번에는 팩토리 메서드를 다룹니다. 구조와 예제를 살펴본 뒤, 이름이 비슷해서 자주 혼동되는 단순 팩토리와 무엇이 다른지, 언제 쓰는 것이 좋은지 정리하겠습니다.

# 팩토리 메서드 패턴이란

팩토리 메서드는 부모 클래스에서 객체들을 생성할 수 있는 인터페이스를 제공하여 자식 클래스들이 생성될 객체들의 유형을 변경할 수 있도록 하는 생성 디자인 패턴입니다.

`new` 연산자로 객체를 직접 생성하던 부분을 팩토리 메서드 호출로 바꾸고, 이 메서드가 반환한 인스턴스를 사용하도록 합니다. 이렇게 생성된 인스턴스를 '제품'이라고 표현합니다.

그리고 제품을 사용하는 코드는 제품 간 상세 구현 차이를 알지 못하고 공통 인터페이스로 취급합니다.

주로 다음 상황에서 사용합니다.

- 코드가 함께 작동해야 하는 객체들의 정확한 유형들과 의존관계들을 미리 모르는 경우

제품 생성 책임을 하위 클래스에 위임하여, 새로운 제품이 추가되더라도 기존 크리에이터의 핵심 로직을 수정하지 않고 확장할 수 있습니다.

- 라이브러리 또는 프레임워크 사용자들에게 내부 컴포넌트들을 확장하는 방법을 제공하고자 하는 경우

프레임워크가 객체 생성용 팩토리 메서드를 정의하고, 사용자가 이를 재정의해 원하는 구현체를 제공할 수 있도록 합니다.

# 구조

팩토리 메서드 패턴은 다음과 같은 구조를 가집니다.

- 제품은 인터페이스를 통해 선언하며, 이 인터페이스를 구현한 것을 구상 제품이라고 합니다.
- 팩토리 메서드를 가지는 크리에이터 클래스를 만듭니다. 이 클래스의 책임은 제품과 관련된 핵심 비즈니스 로직을 구상 제품 클래스들로부터 디커플링하는 것입니다.
- 기초 팩토리 메서드를 둔 경우, 이를 재정의하여 다른 유형의 제품을 반환하게 만듭니다. 이를 구상 크리에이터라고 합니다.

또한 팩토리 메서드 패턴은 항상 새로운 인스턴스를 생성해야 할 필요가 없으며, 기존 객체들을 캐시, 객체 풀 또는 다른 소스로부터 반환할 수 있습니다.

# 구현 예시

운영체제(또는 환경)에 따라 서로 다른 버튼을 사용하는 다이얼로그 예시입니다.

제품 인터페이스와 구상 제품입니다.

{% include code-header.html %}
```java
public interface Button {
    void render();
    void onClick(Runnable handler);
}

public class WindowsButton implements Button {
    @Override
    public void render() {
        // 버튼을 윈도우 스타일로 렌더링합니다.
        System.out.println("Windows 스타일 버튼 렌더링");
    }

    @Override
    public void onClick(Runnable handler) {
        // 실제로는 네이티브 클릭 이벤트에 핸들러를 바인딩하고 클릭 시점에 호출합니다.
        // 예제에서는 클릭 처리를 생략하고 등록만 출력합니다.
        System.out.println("Windows 클릭 핸들러 등록");
    }
}

public class HTMLButton implements Button {
    @Override
    public void render() {
        // 버튼의 HTML 표현을 출력합니다.
        System.out.println("<button>OK</button>");
    }

    @Override
    public void onClick(Runnable handler) {
        // 실제로는 브라우저 클릭 이벤트에 핸들러를 바인딩하고 클릭 시점에 호출합니다.
        // 예제에서는 클릭 처리를 생략하고 등록만 출력합니다.
        System.out.println("HTML 클릭 핸들러 등록");
    }
}
```

크리에이터 클래스는 팩토리 메서드 `createButton()`을 선언하고, 이 메서드가 반환한 제품을 사용하는 핵심 로직(`render()`)을 가집니다. 이 로직은 구상 제품 클래스를 알지 못합니다.

{% include code-header.html %}
```java
public abstract class Dialog {
    // 팩토리 메서드: 제품 객체의 생성을 자식 클래스에 맡깁니다.
    protected abstract Button createButton();

    public void render() {
        Button okButton = createButton();
        okButton.onClick(this::closeDialog);
        okButton.render();
    }

    private void closeDialog() {
        System.out.println("다이얼로그 닫기");
    }
}
```

구상 크리에이터는 팩토리 메서드를 재정의해서 반환하는 제품의 유형을 바꿉니다.

{% include code-header.html %}
```java
public class WindowsDialog extends Dialog {
    @Override
    protected Button createButton() {
        return new WindowsButton();
    }
}

public class WebDialog extends Dialog {
    @Override
    protected Button createButton() {
        return new HTMLButton();
    }
}
```

클라이언트는 설정에 따라 크리에이터의 유형을 선택하고, 이후에는 `Dialog`의 인터페이스로만 사용합니다.

{% include code-header.html %}
```java
public class Application {
    private Dialog dialog;

    private void initialize(String os) {
        if (os.equals("Windows")) {
            dialog = new WindowsDialog();
        } else if (os.equals("Web")) {
            dialog = new WebDialog();
        } else {
            throw new IllegalArgumentException("Unknown platform: " + os);
        }
    }

    public static void main(String[] args) {
        Application app = new Application();
        // 설정 파일을 읽는 대신 인자로 설정 값을 전달합니다.
        app.initialize(args.length > 0 ? args[0] : "Web");
        app.dialog.render();
    }
}
```

`Application`은 `WindowsDialog`와 `WebDialog` 중 무엇이 선택되었는지와 상관없이 `dialog.render()`만 호출합니다. 새로운 플랫폼을 지원하려면 `Button` 구현체와 `Dialog` 자식 클래스를 추가하면 되고, `Dialog.render()`는 수정하지 않아도 됩니다.

# 단순 팩토리와의 차이

단순 팩토리는 객체 생성 분기를 한 클래스(또는 정적 메서드)에 모아두는 관용구입니다. GoF가 정의한 디자인 패턴은 아니며, 팩토리 메서드와 이름이 비슷해서 자주 혼동됩니다.

{% include code-header.html %}
```java
public class ButtonFactory {
    public static Button create(String os) {
        if (os.equals("Windows")) {
            return new WindowsButton();
        } else if (os.equals("Web")) {
            return new HTMLButton();
        }
        throw new IllegalArgumentException("Unknown platform: " + os);
    }
}
```

| 구분 | 단순 팩토리 | 팩토리 메서드 |
| --- | --- | --- |
| 생성 위임 방식 | 한 클래스의 분기문 | 서브클래스의 메서드 재정의 |
| 새 제품 추가 | 팩토리 코드를 직접 수정 | 새 구상 크리에이터를 추가 |
| 개방-폐쇄 원칙 | 확장하려면 기존 팩토리 코드 수정 | 서브클래스로 확장, 크리에이터 선택 지점만 수정 |
| 클래스 수 | 적음 | 제품과 크리에이터가 함께 늘어남 |

제품 유형이 적고 거의 바뀌지 않는다면 단순 팩토리로 충분합니다. 팩토리 메서드의 단점으로 꼽는 서브클래스 증가를 피하는 현실적인 대안이기도 합니다.

반대로 팩토리 메서드가 필요한 경우는 제품을 추가하는 쪽이 팩토리 코드를 수정할 수 없을 때입니다. 예를 들어 프레임워크 사용자는 프레임워크의 분기문을 고칠 수 없지만, 크리에이터를 상속해 팩토리 메서드를 재정의할 수는 있습니다.

# 장단점

팩토리 메서드 패턴의 장단점은 다음과 같습니다.

**장점**

- 크리에이터와 구상 제품들에 대한 결합도를 낮춰줍니다.
- 제품 생성 관련 변경 책임을 담당하는 클래스를 만들기 때문에 단일 책임 원칙을 준수할 수 있게 합니다.
- 크리에이터의 핵심 로직(예제의 `Dialog.render()`)을 수정하지 않고 새로운 유형의 구상 제품을 제공할 수 있으므로 개방-폐쇄 원칙을 준수할 수 있게 됩니다.

**단점**

- 패턴을 구현하기 위해서 새로운 자식 클래스를 도입해야 하므로 코드의 복잡성이 증가합니다.

새로운 기능을 추가할 때, 기능 구현에 필요한 클래스 외에도 패턴의 구조를 유지하기 위한 구상 크리에이터 클래스가 추가로 필요할 수 있습니다.

예를 들어 위 구조에서 Linux 버튼을 추가한다면 다음 두 클래스가 필요합니다.

{% include code-header.html %}
```java
// 기능 구현에 필요한 클래스
class LinuxButton implements Button {
    // ...
}

// 패턴의 구조를 유지하기 위해 필요한 클래스
class LinuxDialog extends Dialog {
    @Override
    protected Button createButton() {
        return new LinuxButton();
    }
}
```

`Dialog`의 다른 동작은 같고 버튼만 다르다면, `createButton(String type)`처럼 팩토리 메서드가 매개변수로 만들 제품을 고르게 할 수도 있습니다. GoF는 이를 팩토리 메서드의 변형인 '매개변수화된 팩토리 메서드'로 설명합니다. 이 방식은 `LinuxDialog` 없이 구현할 수 있지만 새 제품마다 분기를 수정해야 하므로 단순 팩토리와 비슷한 한계를 가집니다. 서브클래스 재정의로 제품을 바꾸는 기본 형태에서는 제품 유형마다 구상 크리에이터가 필요합니다.

따라서 크리에이터로 쓸 수 있는 기존 클래스의 상속 계층이 이미 있다면 이를 활용하여 불필요한 클래스 증가를 줄이는 것이 좋습니다.

---

지금까지 살펴본 내용을 기준으로 팩토리 메서드를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 핵심 로직이 제품 유형을 몰라야 하고 유형이 늘어날 수 있다 | 쓴다 | 새 유형은 구상 크리에이터를 추가해서 만들고 핵심 로직은 수정하지 않는다 |
| 라이브러리나 프레임워크가 사용자에게 확장 지점을 제공해야 한다 | 쓴다 | 사용자가 팩토리 메서드를 재정의해 원하는 구현체를 제공한다 |
| 제품 유형이 적고 거의 바뀌지 않는다 | 단순 팩토리로 충분하다 | 서브클래스 증가 없이 분기를 한 곳에 모을 수 있다 |
| 서브클래스를 만들 이유가 생성 방식 하나뿐이고 확장 지점이 필요 없다 | 피한다 | 매개변수화된 팩토리 메서드나 단순 팩토리로 충분하며, 구상 크리에이터만 늘어난다 |
| 서로 관련된 제품군을 함께 만들어야 한다 | 추상 팩토리를 검토한다 | 팩토리 메서드는 제품 하나의 생성을 서브클래스에 맡기는 구조다 |

정리하면, 팩토리 메서드는 상속을 통한 확장 지점을 열어야 할 때 쓰는 패턴입니다. 생성 로직을 분리하는 것만이 목적이라면 단순 팩토리로 충분한지 먼저 따져 보는 편이 낫습니다.

# 참고 자료

[**팩토리 메서드 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/factory-method)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
