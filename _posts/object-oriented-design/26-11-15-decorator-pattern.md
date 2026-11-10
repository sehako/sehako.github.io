---
title: 데코레이터 패턴 (구조와 프록시 패턴과의 차이)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-15

permalink: "/object-oriented-design/decorator-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 데코레이터를 다룹니다. 구조와 압축·암호화 예제를 살펴본 뒤, 구조가 거의 같은 [프록시 패턴](/object-oriented-design/proxy-pattern/)과 무엇이 다른지 정리하겠습니다.

# 데코레이터 패턴이란

대상 객체를 새로운 행동을 포함한 특수 래퍼 객체로 감싸 기존 코드를 수정하지 않고 런타임에 동적으로 객체에 새로운 기능과 책임을 추가하는 구조 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 객체들을 사용하는 코드를 훼손하지 않으면서 런타임에 추가 행동을 대상 객체에 할당해야 하는 경우

비즈니스 로직을 여러 계층(데코레이터)으로 나누고, 런타임에 원하는 조합으로 감싸서 구성합니다.

모든 데코레이터가 공통 인터페이스를 공유하므로 클라이언트는 동일한 방식으로 호출할 수 있습니다.

- 상속을 사용하여 객체의 행동을 확장하는 것이 어색하거나 불가능한 경우

`final` 키워드 등으로 상속이 제한된 클래스의 기능을 재사용 및 확장해야 할 때 유용합니다.

기능 조합마다 하위 클래스를 만들어야 하는 상속의 폭발 문제를 방지합니다.

>**상속의 폭발(Class Explosion)**
>
>기능 조합마다 하위 클래스를 만들면 기능이 늘수록 클래스 수가 급격히 늘어나는 문제입니다.
>
>예를 들어 파일 데이터 소스에 압축, 암호화 기능을 상속으로 추가하면 `CompressedFileDataSource`, `EncryptedFileDataSource`, `CompressedEncryptedFileDataSource`가 필요합니다. 기능이 하나 더 늘어나면 조합은 3개에서 7개가 됩니다. (기능 n개의 조합은 2ⁿ-1개)
>
>데코레이터는 기능마다 래퍼 하나만 만들고 필요한 조합으로 겹쳐 감싸므로, 기능이 n개여도 클래스는 n개로 충분합니다.
>
>이는 '상속보다 합성(집합)을 사용하라'는 설계 원칙과 맞닿아 있습니다. 상속은 컴파일 시점에 기능 조합이 클래스로 고정되지만, 합성은 다른 객체를 필드로 가지고 위임하므로 런타임에 조합을 바꿀 수 있습니다.
>
>데코레이터는 대상을 합성하고 같은 인터페이스로 위임해 이 원칙을 구현한 패턴입니다.

# 구조

데코레이터 패턴은 다음과 같은 구조를 가집니다.

- 비즈니스 코드의 핵심 기능과 그에 대한 부가 기능에 대한 공통 인터페이스를 제공합니다.
- 핵심 기능을 구현하는 구상 컴포넌트와 모든 데코레이터가 해당 인터페이스를 구현하므로, 클라이언트는 둘을 구분하지 않고 같은 타입으로 다룹니다.
- 기초 데코레이터는 래핑할 컴포넌트를 필드로 가지고 모든 요청을 그대로 위임합니다.
- 구상 데코레이터는 기초 데코레이터를 상속해, 위임 호출 전이나 후에 자신의 부가 기능을 실행합니다.
- 어떤 데코레이터를 어떤 순서로 감쌀지는 클라이언트(또는 설정 코드)가 조립 시점에 결정합니다.

# 구현 예시

데이터를 디스크에 쓰고 읽는 데이터 소스에 압축과 암호화를 추가하는 예시입니다. 데이터 소스 클래스는 압축과 암호화를 알지 못하고, 데코레이터가 디스크에 기록되기 직전에 데이터를 암호화 및 압축하고 읽은 직후에 압축을 풀고 복호화합니다.

데이터 소스와 데코레이터가 함께 구현할 컴포넌트 인터페이스를 선언합니다.

{% include code-header.html %}
```java
// 컴포넌트 인터페이스: 데코레이터가 변경할 수 있는 작업을 정의한다.
public interface DataSource {
    void writeData(String data);
    String readData();
}
```

구상 컴포넌트는 작업의 기본 구현을 제공합니다.

{% include code-header.html %}
```java
// 구상 컴포넌트: 파일에 데이터를 읽고 쓰는 기본 구현이다.
public class FileDataSource implements DataSource {
    public FileDataSource(String filename) {
        // 대상 파일을 설정한다.
    }

    @Override
    public void writeData(String data) {
        // 파일에 데이터를 쓴다.
    }

    @Override
    public String readData() {
        // 파일에서 데이터를 읽는다.
    }
}
```

기초 데코레이터는 같은 인터페이스를 구현하고, 래핑한 컴포넌트(`wrappee`)를 필드로 가집니다. 필드 타입이 구상 클래스가 아니라 인터페이스이므로 원본뿐 아니라 다른 데코레이터도 받을 수 있고, 이 덕분에 여러 겹으로 쌓을 수 있습니다. 모든 작업을 래핑된 컴포넌트에 위임하며, 구상 데코레이터가 필요한 부분만 재정의해 동작을 추가합니다.

{% include code-header.html %}
```java
// 기초 데코레이터: 모든 작업을 래핑된 컴포넌트에 위임한다.
public class DataSourceDecorator implements DataSource {
    protected final DataSource wrappee;

    public DataSourceDecorator(DataSource source) {
        this.wrappee = source;
    }

    @Override
    public void writeData(String data) {
        wrappee.writeData(data);
    }

    @Override
    public String readData() {
        return wrappee.readData();
    }
}
```

구상 데코레이터는 래핑된 객체를 호출하기 전이나 후에 자기 동작을 실행합니다. 쓸 때는 변환한 뒤 위임하고, 읽을 때는 위임한 결과를 역변환합니다.

{% include code-header.html %}
```java
// 구상 데코레이터: 쓰기 전에 암호화하고, 읽은 뒤에 복호화한다.
public class EncryptionDecorator extends DataSourceDecorator {
    public EncryptionDecorator(DataSource source) {
        super(source);
    }

    @Override
    public void writeData(String data) {
        // 1. 전달된 데이터를 암호화한다.
        String encrypted = encrypt(data);
        // 2. 암호화된 데이터를 wrappee의 writeData에 전달한다.
        wrappee.writeData(encrypted);
    }

    @Override
    public String readData() {
        // 1. wrappee의 readData에서 데이터를 가져온다.
        String data = wrappee.readData();
        // 2. 암호화되어 있다면 복호화한다.
        // 3. 결과를 반환한다.
        return decrypt(data);
    }
}
```

{% include code-header.html %}
```java
// 구상 데코레이터: 쓰기 전에 압축하고, 읽은 뒤에 압축을 푼다.
public class CompressionDecorator extends DataSourceDecorator {
    public CompressionDecorator(DataSource source) {
        super(source);
    }

    @Override
    public void writeData(String data) {
        // 1. 전달된 데이터를 압축한다.
        String compressed = compress(data);
        // 2. 압축된 데이터를 wrappee의 writeData에 전달한다.
        wrappee.writeData(compressed);
    }

    @Override
    public String readData() {
        // 1. wrappee의 readData에서 데이터를 가져온다.
        String data = wrappee.readData();
        // 2. 압축되어 있다면 압축을 푼다.
        // 3. 결과를 반환한다.
        return decompress(data);
    }
}
```

데코레이터는 여러 계층으로 겹쳐 객체를 래핑할 수 있습니다. 바깥쪽 데코레이터부터 차례로 실행됩니다.

{% include code-header.html %}
```java
// 데코레이터를 직접 조립하는 예시
public class Application {
    public void dumbUsageExample() {
        DataSource source = new FileDataSource("somefile.dat");
        source.writeData(salaryRecords);
        // 대상 파일에 일반 데이터로 기록된다.

        source = new CompressionDecorator(source);
        source.writeData(salaryRecords);
        // 대상 파일에 압축된 데이터로 기록된다.

        source = new EncryptionDecorator(source);
        // source는 암호화 > 압축 > 파일 데이터 소스 순서로 감싸져 있다.
        source.writeData(salaryRecords);
        // 바깥쪽 데코레이터부터 실행되므로, 암호화한 데이터를 압축해 기록한다.
    }
}
```

클라이언트는 인터페이스 타입만 사용하므로 데이터 저장 세부 사항을 알지 못합니다. 외부에서 구성한 데이터 소스를 전달받아 사용합니다.

{% include code-header.html %}
```java
// 클라이언트: 데이터 소스가 어떤 데코레이터로 감싸져 있는지 알지 못한다.
public class SalaryManager {
    private final DataSource source;

    public SalaryManager(DataSource source) {
        this.source = source;
    }

    public String load() {
        return source.readData();
    }

    public void save() {
        source.writeData(salaryRecords);
    }
    // 다른 유용한 메서드들
}
```

데코레이터 스택은 설정이나 환경에 따라 런타임에 조합할 수 있습니다.

{% include code-header.html %}
```java
public class ApplicationConfigurator {
    public void configurationExample() {
        DataSource source = new FileDataSource("salary.dat");
        if (enabledEncryption) {
            source = new EncryptionDecorator(source);
        }
        if (enabledCompression) {
            source = new CompressionDecorator(source);
        }

        SalaryManager manager = new SalaryManager(source);
        String salary = manager.load();
    }
}
```

# 장단점

데코레이터 패턴의 장단점은 다음과 같습니다.

**장점**

- 새로운 하위 클래스 없이 객체의 행동을 확장할 수 있습니다.
- 런타임에 객체들에게 동적으로 책임을 부여하거나 제거할 수 있습니다.
- 다양한 행동들의 여러 변형을 구현하는 하나의 클래스를 여러 개의 작은 클래스로 나눌 수 있어 단일 책임 원칙을 준수할 수 있습니다.

**단점**

- 데코레이터 스택 내 순서에 의존하지 않도록 구현하기 까다로우며, 특정 래퍼만 제거하기도 어렵습니다.
- 계층들의 초기 설정 코드가 복잡합니다.

# 프록시 패턴과의 차이점

데코레이터 패턴과 [프록시 패턴](/object-oriented-design/proxy-pattern/)은 원본과 같은 인터페이스를 구현하고, 원본 객체를 필드로 가지며, 요청을 위임하는 래퍼 구조가 거의 같습니다. 코드 뼈대로는 구분하기 어렵고, 해결하려는 의도와 객체를 제어하는 방식으로 구분합니다.

- 목적: 데코레이터는 원본의 핵심 기능과 밀접한 동작(압축, 암호화, 버퍼링 등)을 확장합니다. 프록시는 원본에 대한 접근 제어나 비즈니스와 무관한 부가 관심사(권한 검사, 지연 초기화, 캐싱, 로깅, 트랜잭션 등)를 처리합니다.
- 객체 생성과 조합: 데코레이터는 클라이언트가 원본을 만들고 어떤 데코레이터로 어떤 순서로 감쌀지 결정합니다. 프록시는 서비스 객체의 생성과 수명주기를 직접 관리할 수 있습니다. (가상 프록시의 지연 초기화가 대표적입니다.)
- 중첩: 데코레이터는 `A(B(C(원본)))`처럼 여러 겹으로 쌓아 기능 조합을 만드는 것이 전제입니다. 프록시는 보통 클라이언트와 서비스 객체 사이에 대리인 하나를 두는 구조로 쓰며, 여러 겹 중첩을 설계 전제로 삼지 않습니다.

| 구분 | 데코레이터 | 프록시 |
| --- | --- | --- |
| 핵심 의도 | 원본 객체의 기능 확장 | 원본 객체에 대한 접근 제어와 관리 |
| 추가되는 기능 | 원본 비즈니스와 관련이 깊은 기능 | 비즈니스와 관련 없는 부가 관심사 |
| 생성과 조합 | 클라이언트가 제어 | 프록시가 서비스 객체의 생성과 수명주기를 관리할 수 있음 |
| 중첩 | 여러 겹 중첩이 기본 | 보통 대리인 하나 |

---

지금까지 살펴본 내용을 기준으로 데코레이터를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 객체를 사용하는 코드를 바꾸지 않고 런타임에 추가 행동을 붙이거나 떼야 한다 | 쓴다 | 데코레이터가 공통 인터페이스를 공유하므로 클라이언트는 같은 방식으로 호출하고, 조합은 런타임에 정할 수 있다 |
| `final` 등으로 상속이 제한된 클래스의 기능을 확장해야 한다 | 쓴다 | 상속 대신 래핑으로 기능을 덧붙일 수 있다 |
| 기능 조합마다 하위 클래스를 만들어야 해서 클래스가 폭발한다 | 쓴다 | 기능별 데코레이터를 만들어 필요한 조합으로 감싸면 하위 클래스가 늘지 않는다 |
| 여러 변형을 구현한 큰 클래스를 책임별로 나누고 싶다 | 쓴다 | 변형마다 작은 데코레이터로 분리해 단일 책임 원칙을 지킬 수 있다 |
| 접근 제어, 지연 초기화, 로깅처럼 비즈니스와 무관한 관심사를 처리해야 한다 | 프록시를 검토한다 | 데코레이터는 핵심 기능을 확장하는 것이 목적이고, 접근 제어와 서비스 객체의 수명주기 관리는 프록시의 역할이다 |
| 데코레이터를 적용하는 순서에 따라 결과가 달라지거나, 특정 래퍼만 제거해야 한다 | 주의한다 | 순서에 의존하지 않게 구현하기 까다롭고 스택 중간의 래퍼만 제거하기도 어렵다 |
| 확장할 기능이 고정되어 있고 조합할 필요가 없다 | 피한다 | 조합이 필요 없다면 계층 초기 설정 코드의 복잡성만 늘어난다 |

정리하면, 데코레이터는 공통 인터페이스를 구현한 래퍼를 겹쳐 쌓아, 기존 코드를 수정하지 않고 런타임에 기능을 조합해 붙일 때 쓰는 패턴입니다. 구조가 프록시와 거의 같으므로, 기능을 확장하려는 것인지 접근을 제어하려는 것인지 의도로 구분해야 합니다.

# 참고 자료

[**데코레이터 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/decorator)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
