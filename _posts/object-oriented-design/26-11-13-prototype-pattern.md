---
title: 프로토타입 패턴 (복사 생성자와 Cloneable)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-13

permalink: "/object-oriented-design/prototype-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 생성 패턴 편입니다. 이번에는 프로토타입을 다룹니다. 구조를 살펴본 뒤, 복사 생성자로 직접 복제하는 방식과 자바의 `Cloneable`을 사용하는 방식을 차례로 정리하겠습니다.

# 프로토타입 패턴이란

프로토타입은 객체가 자기 자신을 복제할 수 있게 만드는 생성 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 복사해야 하는 객체들의 구상 클래스들에 코드가 의존하면 안 되는 경우

클라이언트 코드가 인터페이스를 통해 다른 코드로부터 전달된 객체인 경우에 인터페이스를 통해 구상 클래스를 복제할 수 있습니다.

- 객체를 초기화하는 방식만 다른 자식 클래스들의 수를 줄이고자 하는 경우

사용 전 많은 설정을 진행해야 하는 객체인 경우 이를 미리 생성 및 캐싱해두고 필요할 때마다 복제하여 사용할 수 있습니다.

# 구조

프로토타입 패턴은 다음과 같은 구조를 가집니다.

- 기초 프로토타입은 `clone()` 메서드를 선언합니다. 인터페이스나 추상 클래스로 만들며, 복제 결과는 기초 프로토타입 타입으로 반환합니다.
- 구상 프로토타입은 `clone()`을 구현하고, 복제할 필드와 복사 범위(얕은 복사 또는 깊은 복사)를 직접 정합니다. 복사 생성자를 쓰면 부모 클래스의 필드는 `super(source)`로 복사합니다.
- 클라이언트는 구상 클래스를 모른 채 기초 프로토타입 타입으로 `clone()`을 호출합니다. 어떤 구상 클래스의 복제본이 만들어질지는 다형성이 결정합니다.

# 구현 예시

도형(`Shape`)을 구상 클래스에 의존하지 않고 복제하는 예시입니다. 복사 생성자로 직접 복제하는 방식과, 자바가 제공하는 `Cloneable`을 사용하는 방식을 차례로 정리합니다.

## 복사 생성자 방식

기초 프로토타입은 복제 메서드를 선언하고, 기존 객체의 값으로 새 객체를 초기화하는 복사 생성자를 가집니다.

{% include code-header.html %}
```java
public abstract class Shape {
    int x;
    int y;
    String color;

    public Shape() {}

    // 프로토타입 생성자: 기존 객체의 값으로 새 객체를 초기화합니다.
    protected Shape(Shape source) {
        this.x = source.x;
        this.y = source.y;
        this.color = source.color;
    }

    // 복제 결과는 Shape의 자식 클래스 중 하나입니다.
    @Override
    public abstract Shape clone();
}
```

구상 프로토타입의 `clone()`은 자신의 복사 생성자에 현재 객체를 넘겨 새 객체를 만듭니다. 복사를 생성자 안에서 모두 끝내므로 완성되지 않은 복제본을 참조할 수 없습니다. 부모 클래스의 필드는 `super(source)`로 복사합니다.

{% include code-header.html %}
```java
public class Rectangle extends Shape {
    int width;
    int height;

    public Rectangle() {
    }

    private Rectangle(Rectangle source) {
        super(source);
        this.width = source.width;
        this.height = source.height;
    }

    @Override
    public Shape clone() {
        return new Rectangle(this);
    }
}

public class Circle extends Shape {
    int radius;

    public Circle() {
    }

    private Circle(Circle source) {
        super(source);
        this.radius = source.radius;
    }

    @Override
    public Shape clone() {
        return new Circle(this);
    }
}
```

클라이언트는 복사할 객체의 구상 타입을 몰라도 `clone()`만 호출해 복제본을 얻습니다. 다형성 덕분에 각 객체가 자신의 클래스에 맞는 복제본을 만듭니다.

{% include code-header.html %}
```java
public class Application {
    private final List<Shape> shapes = new ArrayList<>();

    public Application() {
        Circle circle = new Circle();
        circle.x = 10;
        circle.y = 10;
        circle.radius = 20;
        shapes.add(circle);

        // circle과 같은 값을 가진 별개의 객체입니다.
        Shape anotherCircle = circle.clone();
        shapes.add(anotherCircle);

        Rectangle rectangle = new Rectangle();
        rectangle.width = 10;
        rectangle.height = 20;
        shapes.add(rectangle);
    }

    public void businessLogic() {
        List<Shape> shapesCopy = new ArrayList<>();

        // 요소가 Shape라는 사실만 알고, 실제 클래스는 모른 채 복제합니다.
        for (Shape s : shapes) {
            shapesCopy.add(s.clone());
        }
        // shapesCopy에는 shapes의 각 요소와 같은 타입, 같은 값의 복제본이 들어 있습니다.
    }
}
```

## 자바의 Cloneable

자바는 프로토타입 패턴을 언어 차원에서 지원합니다. `Cloneable` 인터페이스가 복제 가능한 객체임을 표시하고, `Object.clone()`이 실제 복사를 수행합니다. `Cloneable`은 메서드가 없는 마커 인터페이스이므로, 이를 구현하지 않은 객체에서 `Object.clone()`을 호출하면 `CloneNotSupportedException`이 발생합니다.

`Object.clone()`은 필드 값을 그대로 복사하고 실제 런타임 타입의 객체를 반환합니다. 그래서 부모 클래스에서 `super.clone()`만 호출해도 자식 클래스의 필드까지 복사됩니다. 복사 생성자 방식과 달리 자식 클래스마다 `clone()`을 구현하지 않아도 됩니다.

{% include code-header.html %}
```java
public abstract class Shape implements Cloneable {
    int x;
    int y;
    String color;

    @Override
    public Shape clone() {
        try {
            // 필드 값을 복사한 새 객체를 반환합니다. 실제 타입은 Rectangle, Circle 등입니다.
            return (Shape) super.clone();
        } catch (CloneNotSupportedException e) {
            // Cloneable을 구현했으므로 발생하지 않습니다.
            throw new AssertionError(e);
        }
    }
}

public class Rectangle extends Shape {
    int width;
    int height;
}

public class Circle extends Shape {
    int radius;
}
```

클라이언트 코드는 복사 생성자 방식과 같습니다. 다만 `Object.clone()`은 얕은 복사라서, 필드가 객체 참조이면 원본과 복제본이 같은 객체를 공유합니다. 이런 필드가 있는 클래스는 `clone()`에서 참조 필드도 직접 복제해야 합니다.

{% include code-header.html %}
```java
public class Canvas extends Shape {
    List<Shape> children = new ArrayList<>();

    @Override
    public Canvas clone() {
        Canvas copy = (Canvas) super.clone();
        copy.children = new ArrayList<>();
        for (Shape child : children) {
            copy.children.add(child.clone());
        }
        return copy;
    }
}
```

`final` 필드는 `clone()`에서 다시 대입할 수 없고 예외 처리도 번거로워서, 복잡한 객체는 복사 생성자나 정적 팩터리로 복제하는 편이 안전하다는 의견도 많습니다. 이는 단점의 순환 참조가 있는 복잡한 객체를 복제하는 어려움과도 이어집니다.

# 장단점

프로토타입 패턴의 장단점은 다음과 같습니다.

**장점**

- 구상 클래스의 결합없이 객체를 복사할 수 있습니다.
- 미리 만들어진 프로토타입을 사용하여 반복되는 초기화 코드를 제거할 수 있습니다.
- 복잡한 객체들에 대한 사전 설정들을 처리할 수 있어 생성이 간편해집니다.

**단점**

- 순환 참조가 있는 복잡한 객체들을 복제하는 경우에는 구현 난이도가 증가합니다.

---

지금까지 살펴본 내용을 기준으로 프로토타입을 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 복사할 객체의 구상 클래스를 모른 채 복제해야 한다 | 쓴다 | 기초 프로토타입 타입으로 `clone()`만 호출하면 다형성이 실제 클래스에 맞는 복제본을 만든다 |
| 설정이 많은 객체를 미리 만들어 두고 일부만 바꿔 쓴다 | 쓴다 | 설정을 마친 프로토타입을 복제하므로 반복되는 초기화 코드와 초기화 방식만 다른 자식 클래스가 줄어든다 |
| 순환 참조가 있는 복잡한 객체를 복제해야 한다 | 신중히 쓴다 | 복제 구현 난이도가 증가하므로 복사 범위를 직접 설계해야 한다 |
| 참조 필드가 있는 객체를 `Cloneable`로 복제한다 | 깊은 복사를 직접 구현한다 | `Object.clone()`은 얕은 복사라서 원본과 복제본이 참조 필드의 객체를 공유한다 |

정리하면, 프로토타입은 구상 클래스를 모른 채 객체를 복제하거나, 설정을 마친 객체를 재사용해 초기화 코드를 줄일 때 쓰는 패턴입니다. 참조 필드나 순환 참조가 있는 객체라면 복사 범위를 먼저 설계한 뒤 적용해야 합니다.

# 참고 자료

[**프로토타입 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/prototype)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
