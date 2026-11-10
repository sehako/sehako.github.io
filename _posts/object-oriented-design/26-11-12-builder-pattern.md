---
title: 빌더 패턴 (GoF 방식과 자바 플루언트 빌더)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-12

permalink: "/object-oriented-design/builder-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 생성 패턴 편입니다. 이번에는 빌더를 다룹니다. 같은 생성 단계로 서로 다른 제품을 만드는 GoF 방식과, 선택적 매개변수가 많은 객체를 만드는 자바 플루언트 빌더 방식을 차례로 살펴보고 두 방식의 차이를 정리하겠습니다.

# 빌더 패턴이란

빌더는 복잡한 객체들을 단계별로 생성할 수 있도록 하는 생성 디자인 패턴입니다. 이 패턴을 통해 같은 제작 코드를 사용하여 객체의 다양한 유형들과 표현을 생성할 수 있습니다.

주로 다음 상황에서 사용합니다.

- 생성자 오버로딩(점층적 생성자)을 제거하고자 하는 경우

자바에서 10개의 선택적 매개변수가 있는 클래스가 있을 때, 각 필드에 대한 부분 초기화를 제공하는 생성자를 필드의 개수만큼 만들어야 합니다. 매개변수가 하나씩 늘어나는 생성자가 계속 이어지는 모양이어서 이를 점층적 생성자라고 부릅니다.

빌더 패턴을 활용하여 실제로 필요한 단계들만 사용하여 객체를 생성할 수 있으므로 이러한 생성자 오버로딩이 불필요해집니다.

- 같은 제품의 다른 표현들을 생성할 수 있도록 하고자 하는 경우

석조 주택과 목조 주택처럼, 다양한 표현의 생성 과정이 세부 사항만 다른 유사한 단계를 포함할 때 적용할 수 있습니다. 기본 빌더 인터페이스가 가능한 생성 단계를 정의하고, 구상 빌더가 각 단계를 구현해 서로 다른 제품을 만듭니다.

- 복합체 트리처럼 중첩된 복잡한 객체를 생성하는 경우

빌더의 생성 단계를 재귀적으로 호출할 수 있으므로, 객체 트리를 쌓아 올려야 할 때 특히 유용합니다.

# 구조

빌더 패턴은 다음과 같은 구조를 가집니다.

- 모든 빌더에 공통인 제품 생성 단계를 선언하는 빌더 인터페이스를 제공합니다.
- 구상 빌더는 생성 단계를 구현하고, 만든 제품을 반환하는 메서드를 직접 제공합니다. 제품마다 반환 타입이 다르므로 이 메서드는 빌더 인터페이스에 선언하지 않습니다.
- 제품은 빌더가 생성한 결과 객체이며, 구상 빌더마다 서로 다른 타입이어도 됩니다.
- 디렉터는 구상 빌더의 생성 단계를 호출하는 순서를 정합니다. 결과는 디렉터가 아니라 빌더에서 가져옵니다.
- 클라이언트는 구상 빌더 중 하나를 디렉터와 연결합니다.

자바에서는 디렉터 없이 빌더만으로 객체를 생성하는 플루언트 빌더(메서드 체이닝으로 단계를 이어 호출하는 방식)를 사용하기도 합니다.

# 구현 예시

같은 생성 단계로 자동차와 자동차 사용 설명서를 만드는 GoF 방식과, 선택적 매개변수가 많은 객체를 만드는 자바 플루언트 빌더 방식을 차례로 정리합니다.

## GoF 방식 (디렉터 사용)

자동차와 사용 설명서는 공통 인터페이스가 없는 서로 다른 제품입니다.

{% include code-header.html %}
```java
public class Car {
    int seats;
    String engine;
    boolean tripComputer;
    boolean gps;
}

public class Manual {
    final List<String> pages = new ArrayList<>();
}
```

빌더 인터페이스는 제품의 각 부분을 만드는 생성 단계만 선언합니다. 제품을 가져오는 메서드는 제품 타입이 다르므로 선언하지 않습니다.

{% include code-header.html %}
```java
public interface Builder {
    void reset();
    void setSeats(int seats);
    void setEngine(String engine);
    void setTripComputer(boolean tripComputer);
    void setGPS(boolean gps);
}
```

구상 빌더는 같은 생성 단계를 각자 구현합니다. 제품을 가져오는 `getProduct()`는 구상 빌더마다 따로 가지고, 제품을 반환한 뒤에는 `reset()`으로 다음 제품을 만들 준비를 합니다.

{% include code-header.html %}
```java
public class CarBuilder implements Builder {
    private Car car;

    public CarBuilder() {
        reset();
    }

    @Override
    public void reset() {
        this.car = new Car();
    }

    @Override
    public void setSeats(int seats) {
        car.seats = seats;
    }

    @Override
    public void setEngine(String engine) {
        car.engine = engine;
    }

    @Override
    public void setTripComputer(boolean tripComputer) {
        car.tripComputer = tripComputer;
    }

    @Override
    public void setGPS(boolean gps) {
        car.gps = gps;
    }

    public Car getProduct() {
        Car product = this.car;
        reset();
        return product;
    }
}

public class CarManualBuilder implements Builder {
    private Manual manual;

    public CarManualBuilder() {
        reset();
    }

    @Override
    public void reset() {
        this.manual = new Manual();
    }

    @Override
    public void setSeats(int seats) {
        manual.pages.add("좌석 " + seats + "개 사용법");
    }

    @Override
    public void setEngine(String engine) {
        manual.pages.add(engine + " 엔진 사용법");
    }

    @Override
    public void setTripComputer(boolean tripComputer) {
        if (tripComputer) manual.pages.add("트립 컴퓨터 사용법");
    }

    @Override
    public void setGPS(boolean gps) {
        if (gps) manual.pages.add("GPS 사용법");
    }

    public Manual getProduct() {
        Manual product = this.manual;
        reset();
        return product;
    }
}
```

디렉터는 생성 단계를 호출하는 순서만 책임집니다. `Builder` 인터페이스에만 의존하므로 어떤 구상 빌더가 전달되어도 같은 순서로 제품을 만듭니다.

{% include code-header.html %}
```java
public class Director {
    public void constructSportsCar(Builder builder) {
        builder.reset();
        builder.setSeats(2);
        builder.setEngine("스포츠");
        builder.setTripComputer(true);
        builder.setGPS(true);
    }
}
```

클라이언트는 구상 빌더를 만들어 디렉터에 전달하고, 결과는 디렉터가 아니라 빌더에서 가져옵니다. 디렉터는 `Car`와 `Manual`의 존재를 알지 못하기 때문입니다.

{% include code-header.html %}
```java
Director director = new Director();

CarBuilder carBuilder = new CarBuilder();
director.constructSportsCar(carBuilder);
Car car = carBuilder.getProduct();

CarManualBuilder manualBuilder = new CarManualBuilder();
director.constructSportsCar(manualBuilder);
Manual manual = manualBuilder.getProduct();
```

디렉터는 필수가 아닙니다. 클라이언트가 빌더의 생성 단계를 직접 호출해도 되며, 디렉터는 같은 순서와 설정을 재사용할 때 유용합니다.

## 자바 플루언트 빌더

활용 상황의 점층적 생성자 문제를 해결하는 방식입니다. 디렉터와 빌더 인터페이스 없이, 대상 클래스의 정적 중첩 클래스로 빌더를 두고 메서드마다 `this`를 반환해 호출을 이어 붙입니다. 앞의 `Car`와 같은 주제이지만 별개의 클래스입니다.

클라이언트가 값을 메서드 인자로 직접 넘기고, 빌더는 그 값을 자신의 필드에 보관했다가 `build()`에서 한 번에 제품으로 만듭니다. 필수 값은 빌더 생성자로 받고, 나머지는 기본값을 둡니다.

{% include code-header.html %}
```java
public class Car {
    private final int seats;
    private final String engine;
    private final boolean tripComputer;
    private final boolean gps;

    // 빌더를 통해서만 생성할 수 있도록 생성자를 비공개합니다.
    private Car(Builder builder) {
        this.seats = builder.seats;
        this.engine = builder.engine;
        this.tripComputer = builder.tripComputer;
        this.gps = builder.gps;
    }

    public static Builder builder(int seats) {
        return new Builder(seats);
    }

    public static class Builder {
        private final int seats; // 필수 값은 생성자로 받습니다.
        private String engine = "기본"; // 선택 값은 기본값을 둡니다.
        private boolean tripComputer;
        private boolean gps;

        private Builder(int seats) {
            this.seats = seats;
        }

        public Builder engine(String engine) {
            this.engine = engine;
            return this;
        }

        public Builder tripComputer(boolean tripComputer) {
            this.tripComputer = tripComputer;
            return this;
        }

        public Builder gps(boolean gps) {
            this.gps = gps;
            return this;
        }

        public Car build() {
            return new Car(this);
        }
    }
}
```

필요한 값만 골라 넘기므로 매개변수 조합마다 생성자를 만들 필요가 없습니다.

{% include code-header.html %}
```java
Car sportsCar = Car.builder(2)
        .engine("스포츠")
        .tripComputer(true)
        .gps(true)
        .build();

Car basicCar = Car.builder(4).build();
```

두 방식의 차이는 다음과 같습니다.

| 구분 | GoF 방식 | 자바 플루언트 빌더 |
| --- | --- | --- |
| 구성 요소 | 빌더 인터페이스, 구상 빌더, 디렉터 | 대상 클래스 안의 빌더 클래스 하나 |
| 단계 호출 순서 | 디렉터가 정한다 | 클라이언트가 직접 정한다 |
| 결과를 가져오는 곳 | 구상 빌더의 `getProduct()` | 빌더의 `build()` |
| 같은 단계로 다른 제품 생성 | 가능하다 (자동차와 사용 설명서) | 한 종류의 제품만 만든다 |
| 주로 해결하는 문제 | 같은 과정의 다양한 표현, 복잡한 객체 | 점층적 생성자 |

# 장단점

빌더 패턴의 장단점은 다음과 같습니다.

**장점**

- 객체를 단계별로 생성하고, 일부 단계를 나중으로 미루거나 재귀적으로 실행할 수 있습니다.
- 다양한 표현의 제품을 만들 때 같은 생성 코드를 재사용할 수 있습니다.
- 제품의 비즈니스 로직에서 복잡한 생성 코드에 대한 책임을 분리할 수 있어 단일 책임 원칙을 준수할 수 있습니다.

**단점**

- 여러 개의 새 빌더 클래스를 생성해야 하므로 코드의 전반적인 복잡성이 증가합니다.

---

지금까지 살펴본 내용을 기준으로 빌더를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 선택적 매개변수가 많아 생성자 오버로딩이 계속 늘어난다 | 쓴다 | 필요한 값만 골라 넘기므로 매개변수 조합마다 생성자를 만들 필요가 없다 |
| 같은 생성 과정으로 서로 다른 표현의 제품을 만들어야 한다 | 쓴다 | 구상 빌더만 바꿔 같은 생성 코드로 자동차와 사용 설명서 같은 다른 제품을 만든다 |
| 복합체 트리처럼 중첩된 객체를 쌓아 올려야 한다 | 쓴다 | 생성 단계를 재귀적으로 호출할 수 있다 |
| 한 종류의 제품만 만들고 생성 순서를 재사용할 일이 없다 | 플루언트 빌더로 충분하다 | 디렉터와 빌더 인터페이스 없이 빌더 클래스 하나로 해결할 수 있고, 디렉터는 선택 사항이다 |
| 제품의 설정이 단순하고 값이 몇 개뿐이다 | 피한다 | 빌더는 복잡하고 광범위한 설정이 필요할 때 의미가 있고, 새 클래스가 늘어 복잡성만 증가한다 |

정리하면, 빌더는 설정이 복잡한 객체를 단계별로 만들어야 할 때 쓰는 패턴입니다. 한 종류의 제품에 선택적 값이 많을 뿐이라면 플루언트 빌더로 충분한지 먼저 따져 보는 편이 낫고, 설정이 단순하다면 일반 생성자로 충분합니다.

# 참고 자료

[**빌더 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/builder)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
