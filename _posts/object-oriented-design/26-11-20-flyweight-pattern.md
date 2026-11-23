---
title: 플라이웨이트 패턴 (공유 상태와 Java 문자열·정수 캐시)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-20

permalink: "/object-oriented-design/flyweight-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 플라이웨이트를 다룹니다. 병사 생성 예제로 공유 상태와 고유 상태를 나누는 방법을 살펴보고, Java 문자열 풀과 정수 캐시의 공유 원리를 정리하겠습니다.

# 플라이웨이트 패턴이란

메모리 절약을 목적으로 여러 객체들 간 공통 부분들을 공유하여 사용할 수 있도록 하는 구조 디자인 패턴입니다. 이때 공유되는 객체는 불변 객체여야만 합니다.

# 활용 상황

플라이웨이트 패턴은 주로 다음 상황에서 사용됩니다.

- 애플리케이션이 공통 데이터를 가지는 객체를 제공하기 위한 메모리가 부족한 경우

대규모 전쟁 전략 게임에서 병사 객체가 가지고 있는 공통 데이터를 기반으로 약간의 변형만 거치면 하나의 병사를 생성할 수 있다고 가정해보겠습니다.

이런 경우 플라이웨이트 패턴을 통해 병사가 가지는 공통 데이터를 각 병사 객체들 간 공유할 수 있도록 하고, 추가적인 데이터를 통해 병사를 제공하므로 메모리를 절약할 수 있습니다.

# 구조

플라이웨이트 패턴은 다음과 같은 구조를 가집니다.

- 대상 객체의 필드를 공유할 수 있는 상태와 객체가 가지는 고유한 상태로 나눕니다.
- 공유할 수 있는 상태를 플라이웨이트로 제공하고, 고유한 상태를 컨텍스트로 만듭니다.
- 컨텍스트는 내부 필드로 플라이웨이트 객체를 참조합니다.
- 플라이웨이트 객체를 생성하는 책임을 가진 플라이웨이트 팩토리를 제공합니다.
- 클라이언트는 팩토리를 통해 플라이웨이트를 제공받고, 이와 컨텍스트를 기반으로 대상 객체를 구성합니다.

## 구현 방법

대규모 전쟁 전략 게임에서 수많은 병사를 생성하는 예시입니다. 병과별 이름, 3D 모델, 기본 능력치는 공유 상태로 분리하고, 병사별 ID, 위치, 현재 체력은 고유 상태로 보관합니다.

플라이웨이트인 `SoldierType`은 병과별 불변 데이터를 보관합니다. 렌더링에 필요한 개별 병사의 상태는 호출할 때 전달받습니다.

{% include code-header.html %}
```java
// 플라이웨이트: 같은 병과의 병사들이 공유하는 불변 데이터
public final class SoldierType {
    private final String typeName;
    private final Model graphic3DModel;
    private final int baseAttackPower;
    private final int baseDefensePower;

    public SoldierType(String typeName, Model model, int attackPower, int defensePower) {
        this.typeName = typeName;
        this.graphic3DModel = model;
        this.baseAttackPower = attackPower;
        this.baseDefensePower = defensePower;
    }

    public void render(int id, int x, int y, int currentHp) {
        // 공유 모델을 사용해 주어진 위치에 병사와 현재 체력을 표시한다.
    }
}
```

콘텍스트인 `Soldier`는 개별 병사의 상태와 `SoldierType` 참조를 보관합니다. 이동과 피해는 자신의 상태에 반영하고, 렌더링은 현재 상태를 전달하며 공유 객체에 위임합니다.

{% include code-header.html %}
```java
// 콘텍스트: 병사별 고유 상태와 공유 객체의 참조를 보관한다.
public class Soldier {
    private final int id;
    private int x;
    private int y;
    private int currentHp;
    private final SoldierType type;

    public Soldier(int id, int x, int y, int currentHp, SoldierType type) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.currentHp = currentHp;
        this.type = type;
    }

    public void move(int newX, int newY) {
        this.x = newX;
        this.y = newY;
    }

    public void takeDamage(int damage) {
        this.currentHp = Math.max(0, this.currentHp - damage);
    }

    public void draw() {
        type.render(id, x, y, currentHp);
    }
}
```

팩토리는 요청받은 병과의 공유 객체가 있으면 재사용하고, 없으면 모델을 로딩해 생성합니다. 이 예시에서는 병과 이름으로 모델과 기본 능력치가 정해진다고 가정합니다.

{% include code-header.html %}
```java
// 플라이웨이트 팩토리: 병과별 공유 객체를 캐싱한다.
public class SoldierTypeFactory {
    private static final Map<String, SoldierType> soldierTypes = new HashMap<>();

    public static SoldierType getSoldierType(String typeName) {
        SoldierType type = soldierTypes.get(typeName);
        if (type == null) {
            // 병과에 맞는 불변 모델과 기본 능력치를 로딩한다.
            type = new SoldierType(typeName, model, attackPower, defensePower);
            soldierTypes.put(typeName, type);
        }
        return type;
    }
}
```

클라이언트는 팩토리에서 공유 객체를 받아 병사들에게 전달합니다. 보병 10만 명과 궁수 5만 명을 생성해도 무거운 모델을 가진 `SoldierType`은 병과별로 하나씩만 생성됩니다.

{% include code-header.html %}
```java
// 클라이언트: 병과 데이터를 공유하는 병사들을 생성한다.
public class BattleField {
    private final List<Soldier> army = new ArrayList<>();

    public void init() {
        SoldierType infantry = SoldierTypeFactory.getSoldierType("보병");
        SoldierType archer = SoldierTypeFactory.getSoldierType("궁수");

        army.add(new Soldier(1, 10, 20, 100, infantry));
        army.add(new Soldier(2, 30, 40, 100, infantry));
        army.add(new Soldier(3, 50, 60, 70, archer));

        // 같은 방식으로 보병 10만 명과 궁수 5만 명까지 추가한다.
    }

    public void draw() {
        for (Soldier soldier : army) {
            soldier.draw();
        }
    }
}
```

# 장단점

**장점**

- **메모리 절약**: 유사한 객체들 간 공통 데이터 공유를 통해 메모리를 절약할 수 있습니다.

**단점**

- **메모리와 CPU의 트레이드오프**: 메모리를 절약하는 대신, 객체가 공유 상태를 기반으로 연산하는 과정이 들어갈 수 있으므로 CPU 연산 부담이 증가하게 되는 트레이드-오프가 발생할 수 있습니다.
- **구조 이해의 난해함**: 공유 상태와 고유 상태를 보관하는 객체, 그리고 이 둘을 조합하여 생성하는 객체를 만들어야 하므로 일반적인 객체 정의에 비해서 한 번에 이해하기 난해할 수 있습니다.

# Java 표준 라이브러리의 공유 사례

Java의 문자열 풀과 래퍼 클래스 캐시는 불변 객체를 재사용한다는 점에서 플라이웨이트의 공유 원리를 보여줍니다.

## String과 문자열 상수 풀

같은 내용의 문자열 리터럴은 풀의 동일한 `String` 객체를 참조합니다. `String`은 불변이므로 여러 곳에서 안전하게 공유할 수 있습니다. `new String(...)`은 별도의 객체를 생성하며, `intern()`은 같은 내용의 풀 객체를 반환합니다.

{% include code-header.html %}
```java
String s1 = "J-Pub";
String s2 = "J-Pub";
String s3 = new String("J-Pub");

System.out.println(s1 == s2); // true: 같은 풀 객체를 참조한다.
System.out.println(s1 == s3); // false: 별도로 생성한 객체다.
System.out.println(s1 == s3.intern()); // true: 같은 내용의 풀 객체를 반환한다.
```

## Integer와 래퍼 클래스 캐시

`Integer.valueOf(int)`는 -128부터 127까지의 객체를 반드시 캐시하여 재사용합니다. 오토박싱에서도 이 범위의 정수 리터럴은 같은 객체를 공유합니다. 그 밖의 값도 구현이나 JVM 설정에 따라 캐시될 수 있으므로, 129의 참조 비교 결과를 항상 `false`라고 가정해서는 안 됩니다.

{% include code-header.html %}
```java
Integer i1 = 56;
Integer i2 = 56;
Integer i3 = 129;
Integer i4 = 129;

System.out.println(i1 == i2); // true: 캐시된 동일 객체를 공유한다.
System.out.println(i3 == i4); // 기본 캐시 범위에서는 false. 캐시가 확장되면 true일 수 있다.
System.out.println(i3.equals(i4)); // true: 값은 같다.
```

이 예시의 `==`는 객체 공유 여부를 확인하기 위한 비교입니다. 실제 문자열이나 정수의 값 비교에는 `equals()`를 사용합니다.

---

정리하면, 플라이웨이트는 많은 객체가 같은 데이터를 중복 보관해 메모리가 부족할 때, 공통 데이터를 불변 공유 객체로 분리해 메모리를 줄이는 패턴입니다. 메모리를 아끼는 대신 공유 상태를 관리하고 조합하는 비용이 생기므로, 객체마다 데이터가 달라 공유할 부분이 적거나 객체 수가 적다면 도입하지 않는 편이 낫습니다.

# 참고 자료

[**플라이웨이트 패턴**](https://refactoring.guru/ko/design-patterns/flyweight)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
