---
title: 비지터 패턴 (구조와 이중 디스패치)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-28

permalink: "/object-oriented-design/visitor-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 비지터 패턴을 다룹니다. 리소스 파일에 텍스트 추출과 압축 같은 연산을 추가하는 예시로 구조와 구현 방법을 살펴보고, 자바에서 비지터 패턴이 단일 디스패치 두 번으로 이중 디스패치를 흉내 내는 원리, 그리고 비지터 패턴을 도입해야 하는 기준을 정리하겠습니다.

# 비지터 패턴이란

객체 구조나 데이터에서 작업이나 알고리즘을 완전히 분리하여, 기존 객체 클래스를 변경하지 않고도 새로운 연산을 자유롭게 추가할 수 있도록 하는 행동 디자인 패턴입니다.

# 활용 상황

비지터 패턴은 주로 다음 상황에서 사용됩니다.

- 기존 클래스를 수정하지 않고 새로운 연산을 추가하고자 하는 경우

파일 텍스트 추출, 압축, 인덱싱처럼 기능이 늘어날 때마다 모든 데이터 클래스를 열어 수정하면 OCP를 위반합니다. 데이터 클래스는 `accept()`만 열어 두고 연산은 비지터 클래스로 추가하면 기존 코드를 건드리지 않고 기능을 확장할 수 있습니다.

- 객체 구조는 안정적이지만 그 객체들에 대한 연산은 자주 추가·변경되는 경우

비지터 패턴은 연산 추가는 쉽지만 요소 추가는 어렵습니다. PDF, PPT, Word 같은 리소스 파일이나 프로그래밍 언어의 문법 노드(AST)처럼 클래스 종류가 거의 늘지 않는 구조에 적합합니다.

- 데이터 구조의 책임과 연산 알고리즘의 책임을 분리하고자 하는 경우

출력 포맷팅, 직렬화, 통계 계산 같은 로직이 데이터 클래스에 섞이면 클래스가 비대해지고 결합도가 높아집니다. 알고리즘을 비지터 클래스로 분리하면 데이터 클래스는 상태 보관에 집중할 수 있습니다.

- 서로 다른 타입이 섞인 객체 집합을 순회하며 타입별로 작업을 수행해야 하는 경우

복합체 패턴의 트리처럼 여러 구상 타입이 한 컬렉션에 섞여 있으면, 오버로딩이 정적 바인딩으로 처리되어 공통 추상 타입만으로는 타입별 메서드를 호출할 수 없습니다. 이중 디스패치를 사용하면 런타임 타입에 맞는 `visit(PdfFile)`, `visit(PPTFile)`이 호출됩니다.

# 구조

비지터 패턴은 다음과 같은 구조를 가집니다.

- 비지터(Visitor) 인터페이스는 구상 요소의 타입마다 오버로딩된 `visit()` 메서드를 선언합니다.
- 구상 비지터(Concrete Visitor)는 비지터 인터페이스를 구현하며, 연산 하나를 하나의 클래스로 캡슐화합니다. 요소 타입별로 다른 동작을 `visit()`에 구현합니다.
- 요소(Element) 인터페이스(또는 추상 클래스)는 비지터를 받아들이는 `accept(Visitor)` 메서드를 선언합니다.
- 구상 요소(Concrete Element)는 `accept()`에서 자신(`this`)을 넘기며 `visitor.visit(this)`를 호출합니다. 연산 자체는 알지 못하고 비지터에게 위임합니다.
- 클라이언트는 요소 집합과 구상 비지터를 선택하여, 각 요소의 `accept()`에 비지터를 전달합니다.

## 구현 방법

리소스 파일(PDF, PPT)에서 텍스트를 추출하거나 압축하는 예시입니다. 파일 종류는 고정되어 있고 연산은 계속 늘어날 수 있으므로, 연산을 비지터로 분리합니다.

비지터 인터페이스는 구상 요소 타입마다 `visit()`을 오버로딩하여 선언합니다. 구상 비지터는 연산 하나를 맡아 요소 타입별 동작을 구현합니다.

{% include code-header.html %}
```java
public interface Visitor {
    void visit(PdfFile pdfFile);
    void visit(PPTFile pptFile);
}

// 텍스트 추출 연산
public class Extractor implements Visitor {
    @Override
    public void visit(PdfFile pdfFile) {
        // PDF 텍스트 추출
    }

    @Override
    public void visit(PPTFile pptFile) {
        // PPT 텍스트 추출
    }
}

// 압축 연산
public class Compressor implements Visitor {
    @Override
    public void visit(PdfFile pdfFile) {
        // PDF 압축
    }

    @Override
    public void visit(PPTFile pptFile) {
        // PPT 압축
    }
}
```

요소 추상 클래스는 비지터를 받아들이는 `accept()`만 선언합니다. 구상 요소는 `accept()`에서 `visitor.visit(this)`를 호출합니다.

`this`의 컴파일 시점 타입이 `PdfFile`, `PPTFile`처럼 구상 타입으로 확정되므로 알맞은 `visit()` 오버로드가 선택됩니다. 이 호출을 추상 클래스 한 곳에 두면 `this`가 `ResourceFile`로 인식되어 컴파일 오류가 나기 때문에, 구상 클래스마다 같은 코드를 반복해야 합니다.

{% include code-header.html %}
```java
public abstract class ResourceFile {
    protected String filePath;

    public ResourceFile(String filePath) {
        this.filePath = filePath;
    }

    public String getFilePath() {
        return filePath;
    }

    public abstract void accept(Visitor visitor);
}

public class PdfFile extends ResourceFile {
    public PdfFile(String filePath) {
        super(filePath);
    }

    @Override
    public void accept(Visitor visitor) {
        visitor.visit(this); // this는 PdfFile
    }
}

public class PPTFile extends ResourceFile {
    public PPTFile(String filePath) {
        super(filePath);
    }

    @Override
    public void accept(Visitor visitor) {
        visitor.visit(this); // this는 PPTFile
    }
}
```

클라이언트는 컬렉션의 요소 타입이 `ResourceFile`이어도 `accept()`에 비지터만 바꿔 전달하면 다른 연산을 수행합니다.

{% include code-header.html %}
```java
public class ToolApplication {
    public static void main(String[] args) {
        List<ResourceFile> resourceFiles = new ArrayList<>();
        resourceFiles.add(new PdfFile("spring_design_pattern.pdf"));
        resourceFiles.add(new PPTFile("presentation.ppt"));

        Visitor extractor = new Extractor();
        for (ResourceFile resourceFile : resourceFiles) {
            resourceFile.accept(extractor);
        }

        Visitor compressor = new Compressor();
        for (ResourceFile resourceFile : resourceFiles) {
            resourceFile.accept(compressor);
        }
    }
}
```

`accept()`에서 `visitor.visit(this)`로 이어지는 두 번의 호출은 단일 디스패치 언어에서 이중 디스패치(Double Dispatch)를 흉내 내는 방식입니다. 원리는 아래 `단일 디스패치와 이중 디스패치`에서 다룹니다.

`WordCounter`, `Encryptor` 같은 새 연산은 `Visitor` 구현 클래스만 추가하면 되며, 기존 파일 클래스는 수정하지 않습니다.

# 장단점

비지터 패턴의 장단점은 다음과 같습니다.

**장점**

- **개방-폐쇄 원칙(OCP) 준수**: 요소 클래스를 수정하지 않고 새로운 비지터 클래스(예: `WordCountVisitor`, `EncryptVisitor`)만 추가하면 새로운 연산을 얼마든지 확장할 수 있습니다.
- **단일 책임 원칙(SRP) 준수**: 데이터 구조 클래스는 본연의 역할에 집중하고, 복잡한 알고리즘은 각각의 비지터 클래스로 분리됩니다.

**단점**

- **요소 추가에 취약**: 새로운 요소 클래스(예: `ExcelFile`)가 추가되면 모든 비지터 인터페이스와 그 구현체를 일일이 수정해야 합니다.
- **캡슐화 훼손**: 비지터가 작업을 수행하려면 요소 내부의 세부 데이터에 접근해야 하므로, 내부 필드의 getter가 외부에 노출되기 쉽습니다.
- **높은 코드 복잡도**: 이중 디스패치 구조 때문에 코드 흐름을 따라가기 어렵고 디버깅이 까다롭습니다.

# 단일 디스패치와 이중 디스패치

디스패치(Dispatch)는 메서드를 호출할 때 실제로 실행할 메서드를 결정하는 과정입니다. 결정에 쓸 수 있는 기준은 메서드를 호출받는 수신 객체의 타입과 인자의 타입, 두 가지입니다.

| 구분 | 단일 디스패치 | 이중 디스패치 |
| --- | --- | --- |
| 결정 기준 | 수신 객체의 런타임 타입 | 수신 객체와 인자 각각의 런타임 타입 |
| 인자 타입 | 컴파일 시점의 선언 타입으로 고정 (정적 바인딩) | 런타임의 실제 타입을 인식 |
| 지원 언어 | Java, C++, C# 등 대부분의 객체지향 언어 | Common Lisp, Julia 등 다중 디스패치 언어 |

## Java에서 오버로딩만으로 안 되는 이유

Java는 단일 디스패치 언어입니다. 오버라이딩은 런타임에 실제 객체 타입으로 메서드를 고르지만(동적 바인딩), 오버로딩은 컴파일 시점의 선언 타입으로 메서드를 확정합니다(정적 바인딩).

{% include code-header.html %}
```java
List<ResourceFile> resourceFiles = new ArrayList<>();
resourceFiles.add(new PdfFile("a.pdf"));

Extractor extractor = new Extractor();
for (ResourceFile resourceFile : resourceFiles) {
    extractor.visit(resourceFile); // 컴파일 오류
}
```

리스트 안의 실제 객체는 `PdfFile`이지만 변수 `resourceFile`의 선언 타입은 `ResourceFile`입니다. `Extractor`에는 `visit(PdfFile)`, `visit(PPTFile)`만 있고 `visit(ResourceFile)`은 없으므로 컴파일러가 호출할 메서드를 찾지 못합니다.

Java가 이중 디스패치를 지원했다면 `resourceFile`의 런타임 타입(`PdfFile`)을 보고 `visit(PdfFile)`을 호출해 주었을 것이고, 비지터 패턴이 필요하지 않았을 것입니다.

따라서 이중 디스패치 흉내는 비지터 패턴의 필수 조건이 아니라 단일 디스패치 언어의 제약을 넘기 위한 구현 방식입니다. Common Lisp나 Julia 같은 다중 디스패치 언어에서는 오버로드된 메서드가 인자의 런타임 타입으로 바로 선택되므로 `accept()`를 거치는 구조가 필요 없습니다.

## 단일 디스패치 두 번으로 우회하기

비지터 패턴은 단일 디스패치를 연속으로 두 번 호출해서 이중 디스패치를 흉내 냅니다.

{% include code-header.html %}
```text
클라이언트 → ResourceFile.accept(visitor) → Visitor.visit(this)
              (1차: 동적 바인딩)               (2차: this의 정적 타입으로 오버로드 확정)
```

1. `resourceFile.accept(extractor)`는 오버라이딩이므로 실제 인스턴스(`PdfFile`)의 `accept()`가 실행됩니다.
2. `PdfFile.accept()` 안의 `this`는 컴파일러에게도 `PdfFile` 타입이므로, `visitor.visit(this)`는 `visit(PdfFile)`로 확정됩니다.

결국 `accept()`에서 실제 타입을 찾아내고, `this`로 그 타입을 컴파일러에게 알려 주는 우회 방법이 비지터 패턴의 본질입니다.

---

정리하면, 비지터는 요소의 종류는 고정되어 있고 그에 대한 연산이 계속 늘어날 때 연산을 요소에서 분리하는 패턴입니다. 요소 클래스가 자주 추가된다면 피하고, 연산이 하나뿐이거나 요소가 적고 변경이 드물다면 요소의 메서드나 조건 분기를 유지하는 편이 낫습니다.

# 참고 자료

[**비지터 패턴**](https://refactoring.guru/ko/design-patterns/visitor)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
