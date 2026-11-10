---
title: 추상 팩토리 패턴 (구조와 DI 컨테이너와의 차이)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-11

permalink: "/object-oriented-design/abstract-factory-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 생성 패턴 편입니다. 이번에는 추상 팩토리를 다룹니다. 구조와 예제를 살펴본 뒤, 팩토리 패턴과 스프링의 DI 컨테이너가 무엇이 다른지 정리하겠습니다.

# 추상 팩토리 패턴이란

추상 팩토리는 클라이언트가 구체적인 구현 클래스를 직접 지정하거나 의존하지 않고, 서로 관련된 제품군을 생성할 수 있도록 하는 생성 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 코드가 다양한 구현 클래스들과 작동해야 하지만 해당 제품들의 구상 클래스들에 대한 의존도를 내리고 싶은 경우

관련된 제품군의 객체들을 생성하는 인터페이스를 제공하면, 클라이언트는 구상 클래스를 알지 못해도 제품을 생성할 수 있습니다.

- 여러 팩토리 메서드가 클래스의 주요 책임을 불분명하게 만드는 경우

클래스가 자신의 책임 외에 여러 제품의 생성 책임까지 담당한다면, 클래스의 팩토리 메서드들을 독립실행형 팩토리 클래스 또는 완전한 추상 팩토리 구현으로 추출하는 것이 적절할 수 있습니다.

# 구조

추상 팩토리 패턴은 다음과 같은 구조를 가집니다.

- 서로 관련된 제품군을 이루는 각 제품의 인터페이스(추상 제품)를 선언하며, 구상 제품은 변형별로 이 인터페이스를 구현합니다.
- 각 추상 제품을 생성하는 메서드가 정의된 팩토리 인터페이스(추상 팩토리)를 제공합니다.
- 추상 팩토리의 구현 클래스(구상 팩토리)는 하나의 변형에 속한 구상 제품만 생성하도록 메서드를 재정의합니다. 메서드의 반환 타입은 구상 제품이 아니라 추상 제품으로 선언하므로, 클라이언트는 특정 변형에 묶이지 않습니다.

# 구현 예시

운영체제에 따라 서로 다른 스타일의 버튼과 체크박스를 사용하는 GUI 예시입니다. Refactoring.Guru의 의사 코드를 Java로 옮기면서 `paint()`를 `render()`로, `WinFactory`를 `WindowsFactory`로 바꾸는 등 이름을 일부 변경했습니다.

제품 패밀리의 각 제품은 기초 인터페이스를 가지며, 모든 변형은 이 인터페이스를 구현합니다.

{% include code-header.html %}
```java
public interface Button {
    void render();
}

public interface Checkbox {
    void render();
}
```

구상 제품은 변형(운영체제)별로 만듭니다. 같은 변형에 속한 제품끼리만 함께 사용해야 합니다.

{% include code-header.html %}
```java
public class WindowsButton implements Button {
    @Override
    public void render() {
        // 버튼을 윈도우 스타일로 렌더링합니다.
    }
}

public class WindowsCheckbox implements Checkbox {
    @Override
    public void render() {
        // 체크박스를 윈도우 스타일로 렌더링합니다.
    }
}

public class MacButton implements Button {
    @Override
    public void render() {
        // 버튼을 맥 스타일로 렌더링합니다.
    }
}

public class MacCheckbox implements Checkbox {
    @Override
    public void render() {
        // 체크박스를 맥 스타일로 렌더링합니다.
    }
}
```

추상 팩토리 인터페이스는 제품 패밀리를 구성하는 각 추상 제품의 생성 메서드를 선언합니다.

{% include code-header.html %}
```java
public interface GUIFactory {
    Button createButton();
    Checkbox createCheckbox();
}
```

구상 팩토리는 하나의 변형에 속한 제품들만 생성하므로 제품 간 호환이 보장됩니다. 메서드의 반환 타입은 추상 제품이고, 실제 인스턴스화는 메서드 내부에서 일어납니다.

{% include code-header.html %}
```java
public class WindowsFactory implements GUIFactory {
    @Override
    public Button createButton() {
        return new WindowsButton();
    }

    @Override
    public Checkbox createCheckbox() {
        return new WindowsCheckbox();
    }
}

public class MacFactory implements GUIFactory {
    @Override
    public Button createButton() {
        return new MacButton();
    }

    @Override
    public Checkbox createCheckbox() {
        return new MacCheckbox();
    }
}
```

클라이언트 코드는 `GUIFactory`, `Button`, `Checkbox` 같은 추상 타입으로만 팩토리와 제품을 다룹니다. 따라서 어떤 팩토리를 전달해도 클라이언트 코드를 수정할 필요가 없습니다.

{% include code-header.html %}
```java
public class Application {
    private final GUIFactory factory;
    private Button button;
    private Checkbox checkbox;

    public Application(GUIFactory factory) {
        this.factory = factory;
    }

    public void createUI() {
        button = factory.createButton();
        checkbox = factory.createCheckbox();
    }

    public void render() {
        button.render();
        checkbox.render();
    }
}
```

애플리케이션은 설정이나 환경에 따라 팩토리 유형을 선택하고, 보통 초기화 단계에서 이를 한 번 생성합니다.

{% include code-header.html %}
```java
public class ApplicationConfigurator {
    public static void main(String[] args) {
        // 원본 의사 코드의 readApplicationConfigFile() 대신 인자로 설정 값을 전달합니다.
        String os = args.length > 0 ? args[0] : "Windows";

        GUIFactory factory;
        if (os.equals("Windows")) {
            factory = new WindowsFactory();
        } else if (os.equals("Mac")) {
            factory = new MacFactory();
        } else {
            throw new IllegalArgumentException("Unknown operating system: " + os);
        }

        Application app = new Application(factory);
        app.createUI();
        app.render();
    }
}
```

`Application`은 `WindowsFactory`와 `MacFactory` 중 무엇이 선택되었는지와 상관없이 `GUIFactory`만 사용합니다. 새로운 운영체제를 지원하려면 해당 제품들과 구상 팩토리를 추가하면 되고, `Application`은 수정하지 않아도 됩니다.

# 장단점

추상 팩토리 패턴의 장단점은 다음과 같습니다.

**장점**

- 팩토리에서 생성되는 제품들의 상호 호환을 보장할 수 있습니다.
- 구상 제품들과 클라이언트 코드 사이의 결합도를 낮출 수 있습니다.
- 팩토리 메서드 패턴과 마찬가지로, 제품 생성 관련 변경 책임을 담당하는 클래스를 만들기 때문에 단일 책임 원칙을 준수할 수 있게 합니다.
- 기존 클라이언트 코드를 훼손하지 않고 제품의 새로운 변형들을 생성할 수 있으므로 개방 폐쇄 원칙을 준수할 수 있게 합니다.

**단점**

- 패턴과 함께 새로운 인터페이스들과 클래스들이 많이 도입되기 때문에 코드가 필요 이상으로 복잡해질 수 있습니다.

# DI 컨테이너와 팩토리 패턴의 차이

DI 컨테이너는 애플리케이션에 필요한 객체를 생성하고, 의존관계에 따라서 객체를 주입해주는 도구를 의미합니다. 스프링은 이를 IoC 컨테이너라고도 부릅니다.

객체를 생성하는 과정에서 팩토리 패턴이 사용되므로, DI 컨테이너는 애플리케이션 전체를 대상으로 하는 팩토리라고 볼 수 있습니다. 하지만 DI 컨테이너는 그 외에도 설정 분석, 객체 수명 주기 관리 등의 작업도 함께 처리합니다.

## 설정 분석

팩토리 메서드와 추상 팩토리 패턴은 `new WindowsButton()`처럼 생성할 제품의 클래스가 팩토리 코드에 직접 적혀 있습니다. 반면 DI 컨테이너는 프레임워크 형태로 제공되므로 애플리케이션이 어떤 클래스를 만들지 컴파일 시점에 알 수 없습니다.

따라서 DI 컨테이너와 애플리케이션의 코드는 느슨한 결합도를 가져야 합니다. 이를 위해 생성 대상이 되는 객체를 알려주는 도구가 바로 설정이고, 컨테이너는 이를 분석해서 대상 객체를 판별합니다.

스프링에서는 XML 파일이나 애너테이션(`@Component`, `@Bean` 등)이 이 설정에 해당됩니다.

## 객체 생성

컨테이너는 설정에 따라서 대상 객체를 생성합니다. 빈마다 팩토리 클래스를 따로 작성할 수는 없으므로, 스프링은 하나의 빈 팩토리(`BeanFactory`)가 설정 정보를 읽고 리플렉션(런타임에 클래스 정보를 읽어 생성자를 호출하는 기능)으로 대상 객체를 생성합니다.

## 객체 수명 주기 관리

스프링은 빈의 스코프를 설정할 수 있습니다. 요청할 때마다 새 객체를 만드는 prototype 스코프와 컨테이너 안에서 하나만 유지하는 singleton 스코프가 대표적이며, 지연 로딩 여부도 설정할 수 있습니다. 또한 생명주기 시점에 실행할 메서드(초기화·소멸 콜백)도 지정할 수 있습니다.

컨테이너는 이러한 설정을 기반으로 빈의 생성부터 소멸까지를 관리합니다.

스프링의 prototype 스코프는 요청마다 새로 생성하는 것이며, 기존 객체를 복제하는 [프로토타입 패턴](/object-oriented-design/prototype-pattern/)과는 다릅니다. singleton 스코프도 컨테이너 안에서 빈마다 하나라는 뜻이어서, JVM 전체에서 단일 인스턴스를 보장하는 [싱글턴 패턴](/object-oriented-design/singleton-pattern/)과 다릅니다.

## 차이 정리

DI 컨테이너와 팩토리 패턴의 차이를 정리하면 다음과 같습니다.

| 구분 | 팩토리 패턴 (추상 팩토리) | DI 컨테이너 |
| --- | --- | --- |
| 제품 선택 | 코드에서 직접 선택한다 | 설정을 분석해 선택한다 |
| 생성 대상 | 팩토리마다 정해진 제품군 | 애플리케이션 전체의 객체 |
| 생성 코드 | 팩토리 클래스에 작성되어 있다 | 리플렉션으로 생성한다 |
| 주된 책임 | 제품군의 상호 호환 보장 | 생성, 의존관계 주입, 수명 주기 관리 |
| 형태 | 설계 패턴 | 프레임워크가 제공하는 도구 |

## 스프링이 순환 참조를 막는 이유

개발을 하다보면 두 개의 스프링 빈이 생성자 주입 방식을 통해 서로를 의존하도록 설정하고 애플리케이션을 실행할 경우 예외가 발생하는 것을 경험했을 것입니다.

이러한 일이 일어나는 이유는 스프링 빈 생성 단계에서 의존하는 스프링 빈을 발견했을 때, 해당 빈이 아직 만들어지지 않았으면 이를 주입하기 위해 먼저 만드는 재귀적인 방식을 사용하기 때문입니다.

{% include code-header.html %}
```java
Object getBean(String name) {
    if (isCreating(name)) throw new CircularDependencyException();
    if (exists(name)) return getExistingBean(name);

    return createBean(name);
}

Object createBean(String name) {
    for (String dependency : getDependencies(name)) {
        getBean(dependency); // 재귀 발생
    }
    return instantiate(name); // 객체 생성
}
```

따라서 이러한 경우 스프링 빈의 생성 시도가 무한 반복되기 때문에 순환 참조 예외를 던지는 것입니다.

참고로 필드 주입이나 setter 주입은 객체를 먼저 다 만든 다음에 의존성을 주입하므로, 순환 참조를 허용하면 동작했었습니다. 하지만 스프링 부트 2.6부터는 순환 참조가 기본으로 금지되어(`spring.main.allow-circular-references=false`) 이 방식도 기본 설정에서는 예외가 발생합니다.

---

지금까지 살펴본 내용을 기준으로 추상 팩토리를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 서로 관련된 제품을 한 세트로 묶어 함께 써야 한다 | 쓴다 | 구상 팩토리가 한 세트의 제품만 생성하므로 제품 간 호환이 보장된다 |
| 제품 세트가 늘어날 수 있고 클라이언트는 구상 클래스를 몰라야 한다 | 쓴다 | 새 세트는 구상 제품과 구상 팩토리를 추가해서 만들고 클라이언트는 수정하지 않는다 |
| 한 클래스의 팩토리 메서드들이 여러 제품의 생성 책임까지 떠안고 있다 | 쓴다 | 팩토리 메서드들을 독립된 팩토리 클래스로 추출해 단일 책임 원칙을 지킨다 |
| 만들 제품이 하나뿐이다 | 피한다 | 제품군이 없으면 호환을 보장할 대상도 없어서 팩토리 메서드나 단순 팩토리로 충분하다 |
| 제품 세트가 하나뿐이거나 바꿀 일이 거의 없다 | 피한다 | 인터페이스와 클래스만 늘어 코드가 필요 이상으로 복잡해진다 |

정리하면, 추상 팩토리는 함께 쓰여야 하는 제품들이 서로 어긋나지 않게, 팩토리만 바꿔서 제품 전체를 교체해야 할 때 쓰는 패턴입니다. 제품이 하나이거나 바꿀 일이 없다면 더 단순한 팩토리부터 검토하는 편이 낫습니다.

# 참고 자료

[**추상 팩토리 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/abstract-factory)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)

[**Dependency Injection - Dependency Resolution Process**](https://docs.spring.io/spring-framework/reference/core/beans/dependencies/factory-collaborators.html#beans-dependency-resolution)

[**AbstractAutowireCapableBeanFactory.java**](https://github.com/spring-projects/spring-framework/blob/main/spring-beans/src/main/java/org/springframework/beans/factory/support/AbstractAutowireCapableBeanFactory.java#L2990-L3084)
