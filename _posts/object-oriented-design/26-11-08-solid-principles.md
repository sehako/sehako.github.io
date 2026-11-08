---
title: SOLID 원칙 한 번에 정리하기 (Java 예제로 보는 5대 원칙)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-08

permalink: "/object-oriented-design/solid-principles/"
---

SOLID는 소프트웨어를 유지보수하기 쉽고, 이해하기 쉬우며, 유연하게 만들기 위해 제안된 다섯 가지 설계 원칙입니다. 이 원칙들이 해결하려는 문제는 크게 세 가지입니다.

- 의도치 않은 부작용(Side Effect)과 시스템 파손
- 잘못된 설계로 인한 낮은 확장성
- 상속과 추상화의 오용으로 인한 오류

다섯 원칙은 모두 모듈 간 결합도를 낮추고 응집도를 높여서 변경에 유연하게 대응하는 구조를 만드는 것을 목표로 합니다. 이름만 먼저 나열하면 다음과 같습니다.

- SRP(Single Responsibility Principle): 단일 책임 원칙
- OCP(Open/Closed Principle): 개방-폐쇄 원칙
- LSP(Liskov Substitution Principle): 리스코프 치환 원칙
- ISP(Interface Segregation Principle): 인터페이스 분리 원칙
- DIP(Dependency Inversion Principle): 의존성 역전 원칙

각 원칙마다 "원칙을 어긴 코드 → 어떤 문제가 생기는지 → 어떻게 고치는지" 순서로 살펴보겠습니다.

# SRP (Single Responsibility Principle)

소프트웨어의 모듈은 오직 하나의 변경 이유만 가져야 한다는 원칙입니다. 간단해 보이지만 꽤 어려운 원칙입니다. "변경 이유를 어떻게 정의할 것인가?"라는 질문에 답이 달려 있기 때문입니다.

버그를 고치거나 리팩토링을 할 때도 코드를 수정하니, 이것도 변경 이유로 볼 수 있지 않을까요? 하지만 로버트 C. 마틴은 이런 수정은 모듈의 책임이 아니라 개발자의 책임이라고 말합니다. 모듈의 변경 이유는 '책임'이라는 단어와 연결되어 있습니다.

모듈의 책임은 오직 하나의 비즈니스 역할(사람 또는 조직)의 요구사항에만 응답하는가로 정의됩니다. 다음 코드를 보겠습니다.

{% include code-header.html %}
```java
public class Employee {
    public Money calculatePay();
    public void save();
    public String reportHours();
}
```

겉으로는 `Employee`라는 하나의 개념을 다루는 것처럼 보이지만, 메서드마다 변경을 요구하는 주체가 다릅니다.

- `calculatePay()`: 급여 계산 알고리즘을 정의합니다. 회사의 재무를 총괄하는 CFO 조직이 변경을 요구합니다.
- `reportHours()`: 감사용 근무 시간 보고서 서식을 정의합니다. 운영과 감사를 담당하는 COO 조직이 변경을 요구합니다.
- `save()`: 데이터베이스에 저장하는 기술 인프라를 정의합니다. 기술 책임자인 CTO 조직이 변경을 요구합니다.

즉, 단일 책임 원칙은 단순히 코드나 기술적 기능을 나누는 원칙이 아닙니다. 변경을 요구하는 사람(조직)을 기준으로 나누는 원칙입니다.

이 시선으로 보면 `Employee`는 SRP를 위반한 설계입니다. CFO의 요청으로 `calculatePay()`를 고치다가 실수로 `reportHours()`에 오류가 생기면, COO 조직은 자신이 요청하지도 않은 변경 때문에 기존 업무 로직이 깨지고 시스템을 신뢰하지 못하게 됩니다. 이것이 모듈이 가지는 책임의 진짜 의미입니다.

## 재설계

내부의 세 메서드를 비즈니스 주체별로 독립된 모듈로 분리하고, 세 모듈이 공유하는 데이터 구조를 따로 둡니다.

### 데이터와 행위의 분리

{% include code-header.html %}
```java
public record EmployeeInfo(
        String name,
        Money money,
        int hoursWorked
) {}
```

### 변경 요구의 주체에 따른 모듈 분리

{% include code-header.html %}
```java
// 1. CFO 전담 클래스 (재무 및 급여 계산 책임)
public class PayCalculator {
    public Money calculatePay(EmployeeInfo employee) {
        // CFO 조직의 요구사항에 따른 급여 계산 알고리즘 수행
        return employee.money();
    }
}

// 2. COO 전담 클래스 (운영 및 근무 시간 보고 책임)
public class HourReporter {
    public String reportHours(EmployeeInfo employee) {
        // COO 조직의 요구사항에 따른 감사/보고용 문자열 생성
        return "Employee " + employee.name() + ": " + employee.hoursWorked() + " hours";
    }
}

// 3. CTO 전담 클래스 (기술 인프라 및 DB 저장 책임)
public class EmployeeSaver {
    public void save(EmployeeInfo employee) {
        // CTO 조직의 요구사항에 따른 DB Persistence 로직 수행
    }
}
```

이제 급여 계산 방식이 바뀌어도 `PayCalculator`만 수정하면 되고, 다른 조직의 업무에는 영향이 가지 않습니다.

# OCP (Open/Closed Principle)

소프트웨어 모듈은 비즈니스 요구사항의 변경에 대해 확장에는 열려 있고, 변경에는 닫혀 있어야 한다는 원칙입니다.

더하기와 빼기를 하는 계산기를 예로 들어 보겠습니다. 먼저 계산 동작을 나타내는 인터페이스와 구현 클래스를 만듭니다.

{% include code-header.html %}
```java
public interface CalculatorOperation {}

public class Addition implements CalculatorOperation {
    private double left;
    private double right;
    private double result = 0.0;

    public Addition(double left, double right) {
        this.left = left;
        this.right = right;
    }
}

public class Subtraction implements CalculatorOperation {
    private double left;
    private double right;
    private double result = 0.0;

    public Subtraction(double left, double right) {
        this.left = left;
        this.right = right;
    }
}
```

그리고 계산기 클래스를 이렇게 구현했다고 가정해 보겠습니다.

{% include code-header.html %}
```java
public class Calculator {

    public void calculate(CalculatorOperation operation) {
        if (operation == null) {
            throw new InvalidParameterException("Can not perform operation");
        }

        if (operation instanceof Addition) {
            Addition addition = (Addition) operation;
            addition.setResult(addition.getLeft() + addition.getRight());
        } else if (operation instanceof Subtraction) {
            Subtraction subtraction = (Subtraction) operation;
            subtraction.setResult(subtraction.getLeft() - subtraction.getRight());
        }
    }
}
```

이 코드는 OCP를 지키지 못합니다. 곱셈이나 나눗셈이 추가되면 `Calculator` 내부의 `if-else`도 함께 고쳐야 하기 때문입니다. 새 기능을 넣을 때마다 기존 코드를 건드려야 하니, 의도치 않은 부작용이 생길 가능성도 커집니다.

## 재설계

각 연산이 해야 할 행위를 인터페이스로 추상화하고, `Calculator`는 그 추상화에만 의존하게 만듭니다.

{% include code-header.html %}
```java
public interface CalculatorOperation {
    void perform();
}
```

구현 클래스는 `perform()`을 재정의해서 자신의 연산을 수행합니다.

{% include code-header.html %}
```java
public class Addition implements CalculatorOperation {
    private double left;
    private double right;
    private double result;

    // constructor, getters and setters

    @Override
    public void perform() {
        result = left + right;
    }
}
```

`Calculator`는 인터페이스의 `perform()`을 호출하기만 합니다.

{% include code-header.html %}
```java
public class Calculator {

    public void calculate(CalculatorOperation operation) {
        if (operation == null) {
            throw new InvalidParameterException("Cannot perform operation");
        }
        operation.perform();
    }
}
```

이제 어떤 연산이 추가되어도 `Calculator`의 코드는 바뀌지 않습니다. 나눗셈이 필요하면 구현체를 하나 더 만들면 됩니다.

{% include code-header.html %}
```java
public class Division implements CalculatorOperation {
    private double left;
    private double right;
    private double result;

    // constructor, getters and setters

    @Override
    public void perform() {
        if (right != 0) {
            result = left / right;
        }
    }
}
```

기존 코드는 수정하지 않고, 새 코드를 추가하는 것만으로 새 기능을 만들었습니다. 이것이 OCP가 말하는 "확장에는 열려 있고 변경에는 닫혀 있다"입니다.

# LSP (Liskov Substitution Principle)

리스코프 치환 원칙은 하위 타입의 객체가 상위 타입의 객체를 대체하더라도 프로그램의 정상적인 동작이 유지되어야 한다는 원칙입니다.

여기서 말하는 대체 가능성은 두 타입이 상속 관계라는 것만을 뜻하지 않습니다. 상위 타입을 사용하던 클라이언트 코드가 하위 타입으로 바뀌어도 기존에 기대하던 동작이 그대로 유지되어야 한다는 뜻입니다. 즉, LSP는 타입 호환성이 아니라 행위의 호환성을 다루는 원칙입니다.

또 LSP는 OCP의 전제 조건이기도 합니다. 하위 타입이 상위 타입에서 기대한 동작을 하지 못하면, 상위 타입에 의존하던 클라이언트는 예상하지 못한 오류를 겪게 됩니다. 그러면 이를 보정하려고 클라이언트 내부 코드를 고치게 되고, 결국 OCP도 깨집니다.

은행 시스템에서 `Account` 추상 클래스에 입금과 출금을 정의했다고 가정해 보겠습니다.

{% include code-header.html %}
```java
public abstract class Account {
    protected abstract void deposit(BigDecimal amount);
    protected abstract void withdraw(BigDecimal amount);
}
```

일반 입출금 계좌라면 이를 상속해서 이렇게 구현하겠죠.

{% include code-header.html %}
```java
public class CurrentAccount extends Account {
    @Override
    protected void deposit(BigDecimal amount) {
        // 입금 처리
    }

    @Override
    protected void withdraw(BigDecimal amount) {
        // 출금 처리
    }
}
```

그런데 은행이 출금이 허용되지 않는 정기예금 계좌를 추가한다면 어떨까요?

{% include code-header.html %}
```java
public class FixedTermDepositAccount extends Account {
    @Override
    protected void deposit(BigDecimal amount) {
        // 입금 처리
    }

    @Override
    protected void withdraw(BigDecimal amount) {
        throw new UnsupportedOperationException();
    }
}
```

타입 호환성 측면에서는 컴파일 에러가 나지 않습니다. 문제는 이 클래스를 사용하는 클라이언트 코드에서 터집니다.

{% include code-header.html %}
```java
public class BankingAppWithdrawalService {
    private final Account account;

    public BankingAppWithdrawalService(Account account) {
        this.account = account;
    }

    public void withdraw(BigDecimal amount) {
        account.withdraw(amount);
    }
}
```

이 서비스는 모든 `Account`의 하위 타입이 출금을 정상적으로 구현했을 것이라고 기대합니다. 그런데 `FixedTermDepositAccount`가 주입되는 순간 예외가 발생합니다. 즉, `FixedTermDepositAccount`는 `Account`의 행위적 하위 타입이 아닙니다.

## 재설계

행위를 기준으로 상위 타입을 나눕니다. 입금만 가능한 `Account`와, 출금까지 가능한 `WithdrawableAccount`로 분리합니다.

{% include code-header.html %}
```java
public abstract class Account {
    protected abstract void deposit(BigDecimal amount);
}

public abstract class WithdrawableAccount extends Account {
    protected abstract void withdraw(BigDecimal amount);
}

public class CurrentAccount extends WithdrawableAccount {
    // 입금, 출금 구현
}

public class FixedTermDepositAccount extends Account {
    // 입금 구현
}
```

출금 기능을 사용하는 클라이언트도 출금 가능한 상위 타입을 사용하도록 바꿉니다.

{% include code-header.html %}
```java
public class BankingAppWithdrawalService {
    private final WithdrawableAccount account;

    public BankingAppWithdrawalService(WithdrawableAccount account) {
        this.account = account;
    }

    public void withdraw(BigDecimal amount) {
        account.withdraw(amount);
    }
}
```

이제 `FixedTermDepositAccount`는 출금 서비스에 주입할 수 없습니다. 런타임 예외가 컴파일 에러로 바뀌어서 문제를 더 빨리 발견할 수 있습니다.

## LSP를 판단하는 7가지 규칙

Barbara Liskov와 John Guttag은 LSP가 잘 지켜졌는지 판단하는 7개의 규칙을 세 범주로 나눠 제시합니다.

- Signature Rule
- Properties Rule
- Methods Rule

### Signature Rule

#### 1. 매개변수 타입

하위 타입의 메서드는 상위 타입과 동일하거나 더 넓은 범위의 매개변수를 받아야 합니다.

{% include code-header.html %}
```java
class Parent {
    void process(Number value) {}
}

class Child extends Parent {
    void process(Integer value) {}
}
```

상위 타입이 `Number`를 받는데 하위 타입이 `Integer`만 받도록 제한하면 문제가 됩니다. 다만 자바의 오버라이딩 규칙은 매개변수 타입이 정확히 일치해야 하므로, 위 코드는 오버라이딩이 아니라 오버로딩이 되어 이런 문제가 실제로 일어나지는 않습니다.

#### 2. 반환 타입

하위 타입은 상위 타입과 동일하거나 더 좁은 타입을 반환할 수 있습니다. 이를 공변 반환 타입이라고 합니다.

{% include code-header.html %}
```java
abstract class Parent {
    abstract Number getValue();
}

class Child extends Parent {
    @Override
    Integer getValue() {
        return 10;
    }
}
```

클라이언트는 `Number`를 기대하고 하위 타입은 `Integer`를 반환하니 문제없이 사용할 수 있습니다. 반대로 `Object`를 반환한다면 문제가 생깁니다. 자바는 이렇게 반환 타입을 넓히는 것을 허용하지 않으므로 이 규칙 역시 언어 차원에서 지켜집니다.

#### 3. 예외

하위 타입은 상위 타입보다 더 넓거나 새로운 검사 예외를 던져서는 안 됩니다.

{% include code-header.html %}
```java
class Parent {
    void read() throws IOException {}
}

class Child extends Parent {
    @Override
    void read() throws FileNotFoundException {}
}
```

`FileNotFoundException`은 `IOException`의 하위 타입이라 괜찮습니다. 하지만 `IOException`과 관계없는 별도의 예외를 던진다면 클라이언트가 가정한 예외 처리 계약이 깨집니다. 자바는 이 규칙을 컴파일 단계에서 검사하지만, `RuntimeException` 계열은 오버라이딩 규칙으로 제한하지 않기 때문에 직접 주의해야 합니다.

### Properties Rule

#### 4. 클래스 불변식

하위 타입은 상위 타입의 불변식을 유지하거나 강화해야 합니다.

> 불변식: 객체가 유효한 상태일 때 항상 만족해야 하는 조건

`Car`에 "속도는 제한 속도보다 작아야 한다"는 불변식이 있다고 해 보겠습니다.

{% include code-header.html %}
```java
abstract class Car {
    protected int speed;
    protected int limit;

    // Invariant: speed < limit
    abstract void accelerate();
}
```

하위 타입인 `HybridCar`도 이 불변식을 유지해야 합니다. 대신 추가 조건을 붙이는 것은 허용됩니다.

{% include code-header.html %}
```java
class HybridCar extends Car {
    private int charge;

    // Invariant: charge >= 0

    @Override
    void accelerate() {
        // speed < limit를 유지하면서 가속
    }
}
```

충전량이 0 이상이어야 가속할 수 있다면, `accelerate()`의 불변식은 `speed < limit && charge >= 0`처럼 확장됩니다. 하지만 상위 타입이 정의한 불변식을 깨뜨리면, 그것을 믿고 있던 클라이언트의 기대가 무너집니다.

#### 5. 이력 제약 (History Constraint)

하위 타입은 상위 타입에서 허용하지 않는 상태 변경을 허용해서는 안 됩니다.

{% include code-header.html %}
```java
abstract class Car {
    // 생성 시 설정되며 이후 증가만 가능
    protected int mileage;

    protected Car(int mileage) {
        this.mileage = mileage;
    }
}
```

`mileage`는 생성 이후 감소하거나 초기화되지 않고 증가만 한다는 제약이 있습니다.

{% include code-header.html %}
```java
class ToyCar extends Car {
    ToyCar(int mileage) {
        super(mileage);
    }

    public void reset() {
        mileage = 0;
    }
}
```

`ToyCar`가 자체 메서드로 `mileage`를 초기화하면 상위 타입이 보장하던 상태 변경 규칙을 위반합니다. 이처럼 LSP를 검토할 때는 하위 타입이 새로 제공하는 메서드까지 고려해야 합니다.

### Methods Rule

#### 6. 사전조건

사전조건은 메서드가 정상 실행되기 위해 호출자가 만족해야 하는 조건입니다. 하위 타입은 상위 타입의 사전조건을 강화해서는 안 되고, 약화는 할 수 있습니다.

{% include code-header.html %}
```java
class Parent {
    // Precondition: 0 < num <= 5
    void process(int num) {
        if (num <= 0 || num > 5) {
            throw new IllegalArgumentException();
        }
    }
}
```

1부터 5까지만 허용하던 메서드를 하위 타입이 1부터 10까지로 넓힌다면 어떨까요?

{% include code-header.html %}
```java
class Child extends Parent {
    @Override
    void process(int num) {
        if (num <= 0 || num > 10) {
            throw new IllegalArgumentException();
        }
    }
}
```

이 경우는 LSP 위반이 아닙니다. 상위 타입이 허용하던 입력을 하위 타입도 모두 처리할 수 있기 때문입니다. 반대로 허용 범위를 1부터 3까지로 줄이면 위반입니다. 정리하면 사전조건의 약화는 기존 입력을 유지하면서 더 많은 입력을 허용하는 것을 뜻합니다.

#### 7. 사후조건

사후조건은 메서드가 정상 실행된 뒤에 보장해야 하는 결과입니다. 사전조건과 반대로 하위 타입은 사후조건을 약화해서는 안 되고, 강화할 수는 있습니다.

{% include code-header.html %}
```java
abstract class Car {
    protected int speed;

    // Postcondition: speed decreases
    abstract void brake();
}
```

`brake()`는 속도를 감소시키는 행위를 정의합니다.

{% include code-header.html %}
```java
class HybridCar extends Car {
    private int charge;

    @Override
    void brake() {
        // 기존보다 speed 감소
        // charge 증가
    }
}
```

하이브리드 자동차는 `brake()`에서 배터리 충전이라는 결과를 추가로 보장합니다. 이런 강화는 허용되지만, 속도가 줄지 않는다면 LSP 위반입니다. 상위 타입의 클라이언트는 `brake()`가 속도를 줄인다고 믿고 있기 때문입니다.

## LSP 위반을 의심해 볼 수 있는 4가지 코드

7가지 규칙을 일일이 따져 보기 어려울 때는, 다음 네 가지 코드 유형을 위반의 신호로 활용할 수 있습니다.

### 1. 상위 타입의 메서드에서 예외를 던지는 하위 타입

{% include code-header.html %}
```java
class FixedTermDepositAccount extends Account {
    @Override
    void withdraw(BigDecimal amount) {
        throw new UnsupportedOperationException();
    }
}
```

행위를 기준으로 타입을 분리해야 한다는 신호입니다.

### 2. 상위 타입의 메서드를 빈 메서드로 구현

{% include code-header.html %}
```java
interface FileSystem {
    File[] listFiles(String path);
    void deleteFile(String path) throws IOException;
}

class ReadOnlyFileSystem implements FileSystem {
    @Override
    public File[] listFiles(String path) {
        return new File[0];
    }

    @Override
    public void deleteFile(String path) {
        // 아무것도 수행하지 않음
    }
}
```

예외를 던지지 않더라도 메서드가 보장해야 하는 사후조건을 만족하지 못하므로 LSP 위반입니다. 이 경우도 행위별로 타입을 분리해야 한다는 신호입니다.

### 3. 클라이언트가 하위 타입을 직접 확인

{% include code-header.html %}
```java
class FilePurgingJob {
    private final FileSystem fileSystem;

    FilePurgingJob(FileSystem fileSystem) {
        this.fileSystem = fileSystem;
    }

    void purge(String path) throws IOException {
        if (!(fileSystem instanceof ReadOnlyFileSystem)) {
            fileSystem.deleteFile(path);
        }
    }
}
```

하위 타입의 계약 위반을 보완하려고 클라이언트가 `instanceof`로 타입을 검사하고 있다면 LSP 위반을 강하게 의심해 볼 수 있습니다.

### 4. 하위 타입의 메서드가 항상 같은 값을 반환

{% include code-header.html %}
```java
class ToyCar extends Car {
    @Override
    int getRemainingFuel() {
        return 0;
    }
}
```

가장 발견하기 어려운 유형입니다. 상위 타입이 정의한 의미 있는 상태나 동작을 하위 타입이 실제로는 제공하지 못할 가능성이 있기 때문에 문제가 됩니다. 다만 설계 의도상 항상 같은 값을 반환하는 것이 자연스러운 경우도 있으므로, 상위 타입이 어떤 행위를 보장하는지와 반환값의 의도를 함께 확인해야 합니다.

# ISP (Interface Segregation Principle)

인터페이스 분리 원칙은 클라이언트가 사용하지 않는 메서드에 의존하도록 강요받아서는 안 된다는 원칙입니다.

하나의 인터페이스가 서로 다른 클라이언트의 요구를 모두 담고 있으면, 각 클라이언트는 자신이 쓰지 않는 기능까지 포함된 계약에 의존하게 됩니다. 이런 인터페이스를 Fat Interface라고 부릅니다.

ISP는 클라이언트의 역할과 요구사항을 기준으로 인터페이스를 나눕니다. 각 클라이언트가 필요한 기능만 계약으로 사용하면, 서로 관련 없는 기능의 변경이 다른 클라이언트로 번지는 것을 줄일 수 있습니다. 여기서 몇 가지를 짚고 가겠습니다.

- 인터페이스가 크다고 해서 반드시 ISP 위반은 아닙니다.
- 인터페이스에 메서드가 하나만 있어야 하는 것도 아닙니다.
- 판단 기준은 서로 다른 클라이언트가 요구하는 기능과 변경 영향이 실제로 분리되어야 하는지입니다.

결제 승인과 취소 기능을 제공하는 PG 연동 모듈을 예로 들어 보겠습니다.

{% include code-header.html %}
```java
public interface PaymentGateway {
    boolean approve(String orderId);
    void cancel(String paymentId);
    String getStatus(String paymentId);
}
```

승인 처리만 담당하는 클라이언트가 이 인터페이스를 사용한다고 가정해 보겠습니다.

{% include code-header.html %}
```java
public class PaymentApprovalHandler {
    private final PaymentGateway gateway;

    public PaymentApprovalHandler(PaymentGateway gateway) {
        this.gateway = gateway;
    }

    public void handle(String orderId) {
        gateway.approve(orderId);
    }
}
```

`PaymentApprovalHandler`는 `approve()`만 사용하지만, 실제로는 `cancel()`과 `getStatus()`까지 포함한 `PaymentGateway` 전체 계약에 의존합니다. 그래서 `cancel()`의 시그니처가 바뀌면 승인 기능과 관계없는 변경이 이 핸들러가 의존하는 인터페이스에도 일어납니다.

이처럼 서로 다른 클라이언트의 요구가 하나의 인터페이스에 묶여서, 쓰지 않는 기능의 계약 변경에도 노출되는 것이 ISP가 해결하려는 문제입니다.

## 재설계

클라이언트가 요구하는 역할을 기준으로 인터페이스를 나눕니다.

{% include code-header.html %}
```java
public interface PaymentApprover {
    boolean approve(String orderId);
}

public interface PaymentCanceller {
    void cancel(String paymentId);
}

public interface PaymentStatusReader {
    String getStatus(String paymentId);
}
```

이제 승인 처리 클라이언트는 자신에게 필요한 계약에만 의존합니다.

{% include code-header.html %}
```java
public class PaymentApprovalHandler {
    private final PaymentApprover approver;

    public PaymentApprovalHandler(PaymentApprover approver) {
        this.approver = approver;
    }

    public void handle(String orderId) {
        approver.approve(orderId);
    }
}
```

핵심은 구현체까지 쪼갤 필요는 없다는 점입니다.

{% include code-header.html %}
```java
public class PgAdapter implements PaymentApprover,
                                  PaymentCanceller,
                                  PaymentStatusReader {}
```

구현체는 필요한 기능을 모두 제공하면서, 클라이언트는 자신이 의존하는 기능에 해당하는 좁은 인터페이스를 통해서만 구현체에 접근합니다.

# DIP (Dependency Inversion Principle)

의존성 역전 원칙은 상위 수준의 정책이 하위 수준의 구현 세부사항에 종속되지 않도록 의존성의 방향을 설계해야 한다는 원칙입니다.

모듈 A가 모듈 B의 타입이나 기능을 참조하면 "A는 B에 의존한다"고 표현합니다. 필드 선언, 생성자 매개변수, 메서드 호출 등에서 이런 의존이 드러납니다.

{% include code-header.html %}
```java
public class ModuleA {
    private final ModuleB moduleB;
}
```

상위 수준과 하위 수준이 무엇인지 알아보기 위해 "데이터를 저장한다"는 요구사항을 생각해 보겠습니다. 이 요구사항은 두 가지로 나눌 수 있습니다.

- 상위 수준의 정책: 애플리케이션은 데이터를 저장해야 한다.
- 하위 수준의 구현 세부사항: JPA나 JDBC로 데이터베이스에 저장한다.

{% include code-header.html %}
```java
public class DataService {
    private final DataJpaRepository repository;

    public DataService(DataJpaRepository repository) {
        this.repository = repository;
    }

    public void createData(Data data) {
        repository.save(DataEntity.from(data));
    }
}
```

`DataService`는 데이터를 저장하려고 JPA 기반 구현체에 직접 의존하고 있습니다. 실행 흐름과 의존성 구조를 나란히 놓으면 이렇습니다.

{% include code-header.html %}
```text
실행 흐름: DataService -> DataJpaRepository -> Database
의존성 구조: DataService -> DataJpaRepository
```

상위 수준 모듈이 하위 수준 모듈에 직접 의존하는 구조입니다. 이 상태에서 저장 기술이 JPA에서 JDBC로 바뀌면 `DataService`도 변경 범위에 들어갑니다. 저장 기술은 정책이 아니라 세부사항인데도, 정책을 담은 코드가 세부사항 때문에 수정되는 것입니다.

## 재설계

상위 수준 모듈과 하위 수준 모듈이 모두 추상화에 의존하도록 바꿉니다.

{% include code-header.html %}
```java
public interface DataRepository {
    void save(Data data);
}

public class DataService {
    private final DataRepository repository;

    public DataService(DataRepository repository) {
        this.repository = repository;
    }

    public void createData(Data data) {
        repository.save(data);
    }
}
```

JPA 기반 구현체는 이 인터페이스를 구현합니다. 다음은 어댑터 패턴을 적용한 예시입니다.

{% include code-header.html %}
```java
public class DataRepositoryAdapter implements DataRepository {
    private final DataJpaRepository jpaRepository;

    public DataRepositoryAdapter(DataJpaRepository jpaRepository) {
        this.jpaRepository = jpaRepository;
    }

    @Override
    public void save(Data data) {
        jpaRepository.save(DataEntity.from(data));
    }
}
```

`DataRepository`라는 추상화를 두면, 저장 기술이 바뀌어도 계약만 유지되는 한 `DataService`를 수정하지 않고 구현체만 교체할 수 있습니다.

의존성 방향을 보면 왜 "역전"이라고 부르는지 알 수 있습니다.

{% include code-header.html %}
```text
의존성 구조: DataService -> DataRepository <- DataRepositoryAdapter
                                                      |
                                                      v
                                              DataJpaRepository
```

실행 흐름은 그대로입니다. 하지만 상위 모듈과 하위 모듈이 모두 추상화에 의존하게 되면서, 하위 모듈이 상위 모듈이 필요로 하는 계약을 따르는 구조로 의존성의 방향이 뒤집혔습니다.

---

지금까지 살펴본 다섯 원칙과 위반을 의심할 수 있는 신호를 표로 나타내면 다음과 같습니다.

| 원칙  | 한 줄 요약                     | 위반의 신호                                  |
| --- | -------------------------- | --------------------------------------- |
| SRP | 변경을 요구하는 주체별로 모듈을 나눈다      | 한 클래스를 서로 다른 조직의 요청으로 수정한다              |
| OCP | 기존 코드를 고치지 않고 추가로 확장한다     | 기능이 늘 때마다 `if-else`나 `instanceof`가 늘어난다 |
| LSP | 하위 타입은 상위 타입의 행위 계약을 지킨다   | 예외를 던지거나 빈 메서드로 구현한 하위 타입이 있다           |
| ISP | 클라이언트가 쓰는 기능만 계약으로 의존하게 한다 | 쓰지 않는 메서드의 변경에도 영향을 받는다                 |
| DIP | 정책과 구현 모두 추상화에 의존하게 한다     | 상위 모듈이 JPA 같은 구체 기술을 직접 참조한다            |

다섯 원칙은 서로 독립된 규칙이 아니라 맞물려 있습니다. LSP가 지켜져야 OCP를 안전하게 적용할 수 있고, DIP로 만든 추상화는 OCP의 확장 지점이 됩니다.

다만 원칙을 외우는 것보다 중요한 것은 "이 변경이 들어오면 어디를 고쳐야 하는가?"를 스스로 묻는 습관입니다. 모든 코드에 원칙을 기계적으로 적용하기보다는 실제로 변경이 일어나는 지점에 적용하는 편이 효과적입니다.

# 참고 자료

[**The Single Responsibility Principle**](https://blog.cleancoder.com/uncle-bob/2014/05/08/SingleReponsibilityPrinciple.html)

[**The Dependency Inversion Principle in Java**](https://www.baeldung.com/java-dependency-inversion-principle)

[**Open/Closed Principle in Java**](https://www.baeldung.com/java-open-closed-principle)

[**Liskov Substitution Principle in Java**](https://www.baeldung.com/java-liskov-substitution-principle)

[**Interface Segregation Principle in Java**](https://www.baeldung.com/java-interface-segregation)
