---
title: 템플릿 메서드 패턴 (구조와 콜백과의 차이)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-22

permalink: "/object-oriented-design/template-method-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 템플릿 메서드를 다룹니다. 데이터 마이닝 예제로 알고리즘의 골격을 상위 클래스에 고정하고 일부 단계만 하위 클래스에서 구현하는 방법을 살펴본 뒤, 콜백과 무엇이 같고 어디서 달라지는지 정리하겠습니다.

# 템플릿 메서드 패턴이란

상위 클래스가 하나의 메서드 안에 알고리즘(비즈니스 로직)의 골격을 정의하고, 하위 클래스가 골격의 특정 단계를 재정의하여 기능을 확장할 수 있도록 하는 행동 디자인 패턴입니다.

# 활용 상황

템플릿 메서드 패턴은 주로 다음 상황에서 사용됩니다.

- 클라이언트가 알고리즘의 특정 단계만 커스텀(확장)하되, 전체 알고리즘의 뼈대와 실행 흐름은 변경하지 못하도록 제한하려는 경우

상위 클래스에서 전체 처리 순서와 골격을 고정(보통 `final` 메서드로 선언)해 두고, 하위 클래스는 허용된 특정 단계만 오버라이드하도록 유도하여 시스템의 핵심 흐름과 안정성을 보장할 수 있습니다.

- 유사한 알고리즘을 포함하는 여러 클래스가 존재하여, 알고리즘 변경 시 여러 클래스를 동시에 수정해야 하는 경우

중복되는 공통 단계를 상위 클래스의 템플릿 메서드로 끌어올려 코드 중복을 제거하고, 차이가 나는 세부 단계만 하위 클래스로 분리함으로써 향후 알고리즘 변경 시 수정 영향 범위를 최소화할 수 있습니다.

즉, 템플릿 메서드 패턴은 공통 단계를 모아서 재사용하고, 각 단계에 필요한 추가 기능이나 별도 기능을 하위 클래스에서 재정의하는 확장을 위해 설계된 디자인 패턴입니다.

# 구조

템플릿 메서드 패턴은 다음과 같은 구조를 가집니다.

- 알고리즘(비즈니스 로직)의 각 단계를 파악하여 필수적 단계와 선택적 단계로 나눕니다.
- 추상 기초 클래스에서 필수적 단계와 선택적 단계를 정의합니다. 이때 선택적 단계는 하위 클래스에서 재정의하지 않아도 사용할 수 있도록 기본(디폴트)으로 제공합니다. 이러한 선택적 단계를 훅(hook) 메서드라고 합니다.
- 추상 기초 클래스를 확장하는 하위 클래스들은 필수적 단계를 반드시 구현해야 하며, 선택적 단계는 필요에 따라서 재정의합니다.

## 구현 방법

파일에서 데이터를 추출해 분석하고 결과를 알리는 데이터 마이닝 예시입니다. 파일 열기, 분석, 파일 닫기는 모든 형식이 같고, 원시 데이터를 추출하는 단계만 파일 형식(PDF, CSV)마다 다릅니다.

추상 클래스는 전체 흐름을 `final` 템플릿 메서드로 고정합니다. 공통 단계는 기본 구현을 제공하고, 형식마다 달라지는 단계는 추상 메서드로 선언합니다. `shouldSendAlert()`는 알림 여부를 정하는 훅 메서드로, 기본값은 `false`이고 하위 클래스가 필요할 때만 재정의합니다.

{% include code-header.html %}
```java
// 추상 클래스: 알고리즘의 골격을 정의한다.
public abstract class DataMiner {

    // 템플릿 메서드: 처리 순서를 고정하며 하위 클래스가 재정의하지 못한다.
    public final void mine(String path) {
        File file = openFile(path);
        byte[] rawData = extractData(file);
        String analyzedResult = analyzeData(rawData);

        if (shouldSendAlert(analyzedResult)) {
            sendAlert(analyzedResult);
        }

        closeFile(file);
    }

    // 공통 단계: 모든 하위 클래스가 같은 구현을 사용한다.
    protected File openFile(String path) {
        return new File(path);
    }

    // 필수 단계: 파일 형식에 맞게 하위 클래스가 반드시 구현한다.
    protected abstract byte[] extractData(File file);

    protected String analyzeData(byte[] data) {
        // 표준 규격으로 분석한 결과를 반환한다.
        return new String(data);
    }

    // 훅 메서드: 기본값은 알림 없음이며 필요한 하위 클래스만 재정의한다.
    protected boolean shouldSendAlert(String result) {
        return false;
    }

    protected void sendAlert(String result) {
        // 담당자에게 처리 결과를 알린다.
    }

    protected void closeFile(File file) {
        // 파일 리소스를 해제한다.
    }
}
```

구상 클래스는 필수 단계인 `extractData()`만 구현합니다. CSV는 훅까지 재정의해 분석 후 알림을 보냅니다.

{% include code-header.html %}
```java
// 구상 클래스: PDF에서 텍스트를 추출한다.
public class PdfDataMiner extends DataMiner {
    private final PdfParser pdfParser;

    public PdfDataMiner(PdfParser pdfParser) {
        this.pdfParser = pdfParser;
    }

    @Override
    protected byte[] extractData(File file) {
        // PDF 파서로 텍스트를 추출해 반환한다.
        return pdfParser.extractText(file);
    }
}

// 구상 클래스: CSV 행을 파싱하고, 훅을 재정의해 알림을 보낸다.
public class CsvDataMiner extends DataMiner {
    private final CsvParser csvParser;

    public CsvDataMiner(CsvParser csvParser) {
        this.csvParser = csvParser;
    }

    @Override
    protected byte[] extractData(File file) {
        // 쉼표 구분자로 행을 파싱해 반환한다.
        return csvParser.parseRows(file);
    }

    @Override
    protected boolean shouldSendAlert(String result) {
        return true;
    }
}
```

클라이언트는 하위 클래스를 골라 `mine()`만 호출합니다. 처리 순서는 두 클래스 모두 `DataMiner`가 정한 그대로입니다.

{% include code-header.html %}
```java
// 클라이언트 코드
public class Client {
    public static void main(String[] args) {
        DataMiner pdfMiner = new PdfDataMiner(pdfParser);
        pdfMiner.mine("financial_report.pdf");

        DataMiner csvMiner = new CsvDataMiner(csvParser);
        csvMiner.mine("user_logs.csv");
    }
}
```

# 장단점

**장점**

- **변경 영향 최소화**: 대규모 알고리즘의 특정 부분만 하위 클래스에서 재정의할 수 있어 변경에 대한 영향을 최소화할 수 있습니다.
- **중복 코드 제거**: 공통 및 중복되는 코드 부분을 상위 클래스에 모아, 알고리즘이 바뀔 때 상위 클래스 한 곳만 수정하면 됩니다.
- **처리 순서 고정**: `final` 템플릿 메서드로 처리 순서를 고정하므로 하위 클래스가 전체 흐름을 바꾸지 못합니다.

**단점**

- **하위 클래스 유연성 저하**: 상위 클래스에서 미리 정해둔 알고리즘 골격(스켈레톤)의 제약으로 인해 하위 클래스의 유연성이 떨어질 수 있습니다.
- **리스코프 치환 원칙(LSP) 위배 위험**: 하위 클래스에서 상위 클래스가 기본 제공하는 단계별 구현을 무력화(빈 메서드 작성, 예외 발생 등)하여 리스코프 치환 원칙(LSP)을 위배하기 쉽습니다.
- **유지보수의 어려움**: 알고리즘 단계의 수가 많아지거나 상속 계층이 깊어질수록 전체 흐름을 추적하고 유지보수하기가 어렵습니다.

# 템플릿 메서드 패턴과 콜백

템플릿 메서드처럼, 콜백 또한 재사용과 확장 문제를 해결할 수 있습니다. 콜백은 확장할 동작을 객체(익명 클래스, 람다)로 만들어 메서드의 인자로 전달하고, 메서드는 정해진 시점에 이를 호출합니다.

{% include code-header.html %}
```java
public interface Callback {
    void method();
}

public class Example {
    public void doSomething(Callback callback) {
        // ...
        callback.method();
        // ...
    }
}
```

콜백은 동기식과 비동기식으로 나누어집니다. 애플리케이션 관점에서 동기식 콜백은 템플릿 메서드 패턴과 상당히 유사하고, 비동기식 콜백은 [옵서버 패턴](/object-oriented-design/observer-pattern/)과 유사합니다.

## 동기식 콜백

동기식 콜백은 `doSomething()`이 반환되기 전에 콜백이 실행됩니다. 앞의 `DataMiner`에서 형식마다 달라지는 `extractData()`를 콜백으로 바꾸면 다음과 같습니다. 알림 훅은 생략했습니다.

{% include code-header.html %}
```java
// 콜백: 파일 형식마다 달라지는 추출 단계
public interface DataExtractor {
    byte[] extract(File file);
}

public class DataMiner {
    public void mine(String path, DataExtractor extractor) {
        File file = openFile(path);
        byte[] rawData = extractor.extract(file);
        String analyzedResult = analyzeData(rawData);
        // ...
        closeFile(file);
    }
}
```

`PdfDataMiner`, `CsvDataMiner` 같은 하위 클래스 없이 호출할 때 람다로 확장 지점을 넘깁니다. 처리 순서를 `mine()`이 정한다는 점은 템플릿 메서드와 같습니다.

{% include code-header.html %}
```java
// 클라이언트 코드
DataMiner miner = new DataMiner();

miner.mine("financial_report.pdf", file -> pdfParser.extractText(file));
miner.mine("user_logs.csv", file -> csvParser.parseRows(file));
```

## 비동기식 콜백

비동기식 콜백은 `doSomething()`이 먼저 반환되고 콜백은 나중에 실행됩니다. 작업이 끝났을 때나 이벤트가 발생했을 때 미리 등록해 둔 동작을 호출한다는 점에서, 상태가 바뀌면 관찰자를 호출하는 옵서버 패턴과 닮았습니다.

{% include code-header.html %}
```java
// 비동기식 콜백: 작업을 다른 스레드에 맡기고 바로 반환한다. (종료 처리 등은 생략)
public class AsyncExample {
    private final ExecutorService executor = Executors.newSingleThreadExecutor();

    public void doSomething(Callback callback) {
        executor.submit(() -> {
            // ...
            callback.method();
            // ...
        });
    }
}
```

클라이언트는 콜백을 람다로 전달할 수 있으며, `doSomething()` 호출 뒤의 코드는 콜백 실행을 기다리지 않습니다.

{% include code-header.html %}
```java
// 클라이언트 코드
AsyncExample example = new AsyncExample();

example.doSomething(() -> {
    // 작업이 끝난 뒤 실행된다.
});
// 이 줄은 위 콜백보다 먼저 실행될 수 있다.
```

## 템플릿 메서드와 콜백의 차이점

코드 구현의 관점에서 보면 콜백과 템플릿 메서드 패턴은 상당히 다릅니다. 콜백은 합성 관계를 기반으로 하여 한 객체를 다른 객체로 전달하는 객체 간 관계를 가지지만, 템플릿 메서드 패턴은 상속 관계를 기반으로 구현되어 하위 클래스가 상위 클래스의 알고리즘을 재정의합니다.

따라서 콜백은 템플릿 메서드 패턴과 비교했을 때, 다음과 같은 이점이 있습니다.

- 자바처럼 단일 상속만 지원하는 프로그래밍 언어에서는 템플릿 메서드의 하위 클래스는 다른 클래스를 상속받을 수 없습니다.
- 클래스를 미리 정의할 필요가 없이 익명 클래스나 람다를 사용하여 콜백 객체 생성이 가능합니다.
- 하나의 클래스에 템플릿 메서드가 여러 개 있으면, 하위 클래스가 그중 하나만 사용하더라도 클래스에 정의된 모든 추상 단계를 구현해야 합니다. 콜백은 사용할 메서드에만 콜백을 전달하면 됩니다.

반대로 템플릿 메서드 패턴은 다음과 같은 이점이 있습니다.

- 확장 지점이 여러 개일 때, 콜백은 전달할 인자가 늘어나지만 템플릿 메서드는 하위 클래스 하나에 관련 단계를 묶어 이름을 붙일 수 있습니다.
- 하위 클래스가 상위 클래스의 `protected` 멤버를 직접 사용할 수 있습니다.

정리하면, 확장 지점이 적고 호출할 때마다 동작이 달라지면 콜백이 어울리고, 확장 지점이 많아 하나의 변형을 클래스로 묶어 관리하는 편이 낫거나 이미 상속 구조가 있다면 템플릿 메서드가 어울립니다.

---

정리하면, 템플릿 메서드는 처리 순서는 같고 일부 단계만 다른 클래스들의 흐름을 상위 클래스에 고정하고, 달라지는 단계만 하위 클래스에 맡기는 패턴입니다. 확장 지점이 하나이거나 이미 다른 클래스를 상속해야 한다면 콜백이 더 어울리고, 골격 자체가 달라지거나 상속 계층이 깊어진다면 피하는 편이 낫습니다.

# 참고 자료

[**템플릿 메서드 패턴**](https://refactoring.guru/ko/design-patterns/template-method)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
