---
title: 커맨드 패턴 (구조와 Undo 구현)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-30

permalink: "/object-oriented-design/command-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 커맨드 패턴을 다룹니다. 텍스트 에디터의 실행 취소(Undo) 예시로 구조와 구현 방법을 살펴보고, 커맨드 패턴의 장단점과 도입 기준을 정리하겠습니다.

# 커맨드 패턴이란

요청(함수 호출, 명령) 자체를 독립적인 객체로 캡슐화하여 요청의 발신자와 실제 작업을 수행하는 수신자를 분리하는 행동 디자인 패턴입니다.

# 활용 상황

커맨드 패턴은 주로 다음 상황에서 사용됩니다.

- 요청을 매개변수화하여 호출자를 수정하지 않고 다양한 동작을 동적으로 연결하려는 경우

GUI 버튼, 단축키, 툴바처럼 클릭했을 때 실행할 작업이 런타임에 바뀌거나 여러 곳에서 재사용되어야 할 때 사용합니다. 발송자(Invoker)는 비즈니스 로직이나 수신자를 몰라도 `execute()`만 호출하면 되고, 클라이언트가 생성 시점에 알맞은 커맨드 객체를 주입하므로 발송자와 비즈니스 로직의 결합도가 낮아집니다.

- 요청을 즉시 처리하지 않고 큐에 적재해 지연 실행, 비동기 처리, 작업 스케줄링을 하려는 경우

모바일 게임 서버처럼 대규모 요청이 몰리는데 처리 스레드는 제한되어 있을 때 유용합니다. 클라이언트의 요청(이벤트와 데이터)을 커맨드 객체로 캡슐화해 메모리 큐에 쌓아 두고, 서버 스레드가 순차적으로 꺼내 처리하면 부하를 조절할 수 있습니다.

- 되돌릴 수 있는 작업(Undo/Redo)과 작업 내역 로깅(Audit Trail)을 지원해야 하는 경우

문서 편집기나 그래픽 도구처럼 실행 취소가 필요할 때 표준적으로 사용합니다. 커맨드 객체가 실행 전 상태의 백업이나 역연산 로직을 갖도록 설계하고 실행한 커맨드를 히스토리 스택에 저장해 두면, 실행 취소 요청 시 스택에서 꺼내 `undo()`를 호출해 이전 상태로 되돌릴 수 있습니다.

- 단위 명령을 조합해 일괄 실행하는 복합 명령(매크로 커맨드, 배치 작업)을 구성하려는 경우

파일 열기, 텍스트 변환, 저장, 닫기 같은 작은 커맨드를 리스트에 담아 순차 실행하는 매크로나 배치 스크립트를 만들 때, 개별 커맨드와 복합 커맨드를 같은 인터페이스로 묶어 조합할 수 있습니다.

# 구조

커맨드 패턴은 다음과 같은 구조를 가집니다.

- 커맨드(Command) 인터페이스는 요청을 실행하는 규격을 선언합니다. 주로 `execute()`이며 필요하면 `undo()`를 포함합니다. 발송자는 이 인터페이스만 알면 수신자나 작업 내용을 몰라도 같은 방식으로 요청을 실행할 수 있습니다.
- 구상 커맨드(Concrete Command)는 커맨드 인터페이스를 구현하며 요청 유형 하나(저장, 붙여넣기, 이동 등)를 정의합니다. 수신자에 대한 참조와 실행에 필요한 매개변수를 필드로 보관합니다. `execute()`가 호출되면 직접 로직을 수행하지 않고 수신자의 메서드를 호출해 작업을 위임합니다.
- 발송자(Invoker)는 커맨드 객체에 대한 참조를 유지하고 요청을 시작합니다. GUI 버튼, 단축키 리스너, 작업 큐 스케줄러가 예입니다. 이벤트가 발생하면 `command.execute()`를 호출하며, 어떤 수신자가 처리하는지와 매개변수는 알지 못합니다.
- 수신자(Receiver)는 실제 비즈니스 로직을 수행하는 객체입니다. 문서 모델, 계좌 서비스, 게임 캐릭터가 예입니다. 상태 변경과 연산은 수신자 내부에서 일어납니다.
- 클라이언트는 수신자를 생성하고, 필요한 매개변수와 함께 구상 커맨드를 만들어 발송자에 연결합니다. 객체를 조립하는 역할입니다.

## 구현 방법

커맨드 패턴의 핵심은 요청을 실행하는 방법(`execute()`)과 되돌리는 방법(`undo()`)을 커맨드 객체가 스스로 갖게 하는 것입니다. 발송자는 커맨드 인터페이스만 알고, 실제 작업은 수신자가 맡습니다. 이 역할 분리와 커맨드를 자료구조에 보관하는 특징이 모두 드러나는 텍스트 에디터의 실행 취소(Undo) 예시입니다.

`Command`는 모든 구상 커맨드가 따르는 인터페이스입니다. 실행과 취소만 정의합니다.

{% include code-header.html %}
```java
public interface Command {
    void execute();
    void undo();
}
```

`Document`는 수신자입니다. 텍스트를 실제로 수정하는 비즈니스 로직만 가지며 커맨드의 존재를 모릅니다.

{% include code-header.html %}
```java
public class Document {
    private final StringBuilder text = new StringBuilder();

    public void insert(int position, String str) {
        text.insert(position, str);
    }

    public void delete(int position, int length) {
        text.delete(position, position + length);
    }

    public String getText() {
        return text.toString();
    }
}
```

`InsertTextCommand`는 구상 커맨드입니다. 수신자에 대한 참조와 삽입 위치, 삽입할 텍스트를 필드로 캡슐화합니다. 삽입의 역연산은 삽입한 길이만큼 지우는 것이므로 `undo()`는 이 정보만으로 이전 상태를 되돌립니다.

{% include code-header.html %}
```java
public class InsertTextCommand implements Command {
    private final Document document;
    private final int insertPosition;
    private final String textToInsert;

    public InsertTextCommand(Document document, int insertPosition, String textToInsert) {
        this.document = document;
        this.insertPosition = insertPosition;
        this.textToInsert = textToInsert;
    }

    @Override
    public void execute() {
        document.insert(insertPosition, textToInsert);
    }

    @Override
    public void undo() {
        document.delete(insertPosition, textToInsert.length());
    }
}
```

`EditorInvoker`는 발송자입니다. 커맨드를 실행하고 이력 스택에 쌓으며, 실행 취소 요청이 오면 가장 최근 커맨드를 꺼내 `undo()`를 호출합니다. `Document`나 `InsertTextCommand`를 직접 참조하지 않고 `Command` 타입만 다룹니다.

{% include code-header.html %}
```java
import java.util.ArrayDeque;
import java.util.Deque;

public class EditorInvoker {
    private final Deque<Command> undoStack = new ArrayDeque<>();

    public void executeCommand(Command command) {
        command.execute();
        undoStack.push(command);
    }

    public void undo() {
        if (!undoStack.isEmpty()) {
            undoStack.pop().undo();
        }
    }
}
```

클라이언트는 수신자와 커맨드를 만들어 발송자에게 넘깁니다.

{% include code-header.html %}
```java
public class Client {
    public static void main(String[] args) {
        Document doc = new Document();
        EditorInvoker invoker = new EditorInvoker();

        invoker.executeCommand(new InsertTextCommand(doc, 0, "Hello")); // 문서: "Hello"
        invoker.executeCommand(new InsertTextCommand(doc, 5, " World!")); // 문서: "Hello World!"

        invoker.undo(); // 문서: "Hello"
        invoker.undo(); // 문서: ""
    }
}
```

`EditorInvoker`의 `undoStack`을 큐로 바꾸고 `execute()`를 바로 호출하지 않고 나중에 꺼내 실행하면, 같은 구조로 요청 큐잉과 지연 실행을 구현할 수 있습니다.

같은 에디터의 실행 취소를 메멘토 패턴은 상태 스냅숏으로, 커맨드 패턴은 역연산으로 구현합니다. 두 방식은 함께 쓰기도 하는데, 역연산을 구하기 어려운 커맨드가 실행 전 상태의 메멘토를 보관하는 식입니다.

# 장단점

커맨드 패턴의 장단점은 다음과 같습니다.

**장점**

- **단일 책임 원칙(SRP) 준수**: 요청을 호출하는 발송자(Invoker)와 실제 비즈니스 로직을 수행하는 수신자(Receiver)를 분리하여 각각 하나의 책임에 집중시킬 수 있습니다.
- **개방-폐쇄 원칙(OCP) 준수**: 기존 발송자나 클라이언트 코드를 수정하지 않고도 새로운 구상 커맨드 클래스를 추가할 수 있습니다.
- **지연 실행과 큐 적재**: 명령을 데이터가 담긴 독립 객체로 캡슐화하므로 즉시 실행하지 않고 큐에 적재해 비동기로 처리하거나 실행 시점을 미룰 수 있습니다.
- **실행 취소(Undo)와 다시 실행(Redo)의 용이성**: 커맨드 객체 안에 역연산 로직을 두거나 이전 상태의 스냅숏을 보관해 실행 취소와 재실행을 체계적으로 구현할 수 있습니다.
- **복합 명령(Macro Command) 구성**: 단순한 커맨드 여러 개를 묶어 순차적으로 일괄 실행하는 복합 커맨드로 조합하기 쉽습니다.

**단점**

- **클래스 개수 증가와 복잡도 상승**: 발송자와 수신자 사이에 `Command`라는 추상화 계층이 추가되고 명령의 종류마다 커맨드 클래스를 정의해야 하므로 클래스 수가 늘고 코드가 복잡해집니다.
- **단순한 작업에는 과한 설계**: 비동기 큐잉이나 Undo/Redo가 필요 없는 단순한 호출 관계에 적용하면 간접 호출 비용과 개발 공수만 늘어납니다.

---

정리하면, 커맨드는 요청을 객체로 캡슐화해 발송자와 수신자를 분리하고, 요청을 큐에 쌓거나 실행 취소하거나 묶어서 실행해야 할 때 쓰는 패턴입니다. 단순한 호출 관계라면 직접 호출을 유지하는 편이 낫고, 역연산을 구하기 어려운 작업의 실행 취소가 필요하다면 메멘토를 함께 검토합니다.

# 참고 자료

[**커맨드 패턴**](https://refactoring.guru/ko/design-patterns/command)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
