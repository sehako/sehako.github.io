---
title: 메멘토 패턴 (구조와 캡슐화를 지키는 구현 방식)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-29

permalink: "/object-oriented-design/memento-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 메멘토 패턴을 다룹니다. 텍스트 에디터의 실행 취소(Undo) 예시로 구조와 캡슐화를 지키는 세 가지 구현 방식을 살펴보고, 장단점과 백업이 잦거나 객체가 클 때 쓰는 시간 및 공간 최적화 방법을 정리하겠습니다.

# 메멘토 패턴이란

객체의 구현 세부 사항을 공개하지 않으면서 해당 객체의 이전 상태를 저장 및 복원할 수 있게 하는 행동 디자인 패턴입니다.

# 활용 상황

메멘토 패턴은 주로 다음 상황에서 사용됩니다.

- 실행 취소(Undo)나 롤백 기능을 구현해야 하는 경우

텍스트 에디터, 그래픽 편집기, 게임 세이브 포인트, 트랜잭션 롤백처럼 작업 내역이나 시스템 상태를 이전 시점으로 복원해야 할 때 사용합니다. 특정 시점의 상태를 불변 스냅숏으로 만들어 스택 등에 보관하면 원하는 시점으로 안전하게 되돌릴 수 있습니다.

- 캡슐화를 깨뜨리지 않으면서 상태를 백업하고 복원해야 하는 경우

`private` 필드의 Getter/Setter를 외부에 열어 두면 데이터 무결성이 쉽게 깨집니다. 오리지네이터가 직접 자신의 읽기 전용 스냅숏을 만들고 복원하게 하면 내부 구현을 노출하지 않고 상태를 다룰 수 있습니다.

- 상태 보관 책임과 비즈니스 로직 책임을 분리하고 싶은 경우

스냅숏 스택 관리, 백업 주기 제어, 메모리 한도 관리까지 원본 클래스가 맡으면 클래스가 비대해집니다. 상태의 보관과 탐색은 케어테이커에게 넘기고 원본 객체는 비즈니스 로직에만 집중하게 합니다.

# 구조

메멘토 패턴은 다음과 같은 구조를 가집니다.

- 오리지네이터(Originator)는 상태를 가진 원본 객체입니다. 자신의 현재 상태를 담은 메멘토를 만들고(`save()`), 메멘토로부터 상태를 복원합니다(`restore()`). 메멘토의 내부 데이터에 접근할 수 있는 유일한 객체입니다.
- 메멘토(Memento)는 특정 시점의 오리지네이터 상태를 담은 불변 스냅숏입니다. 케어테이커에게는 상태를 읽거나 바꿀 수 없는 껍데기(빈 인터페이스, 좁은 인터페이스)만 노출합니다.
- 케어테이커(Caretaker)는 메멘토를 스택 등에 보관하고, 복원이 필요할 때 오리지네이터에 되돌려 줍니다. 메멘토의 내용은 알지 못합니다.
- 클라이언트는 상태를 바꾸기 전에 케어테이커를 통해 메멘토를 저장하고, 실행 취소 시 가장 최근 메멘토로 복원합니다.

## 구현 방법

메멘토 패턴의 핵심은 케어테이커가 메멘토를 보관만 할 뿐 내부 상태는 읽거나 바꾸지 못하게 하는 것입니다. 이 캡슐화를 어떻게 지키느냐에 따라 구현 방식이 나뉩니다.

### 중첩 클래스 기반

Java나 C++처럼 중첩 클래스를 지원하는 언어에서 캡슐화를 가장 완벽하게 지킬 수 있는 방식입니다. 텍스트 에디터에서 글을 입력하고 실행 취소(Undo)로 이전 상태를 복원하는 예시입니다.

- 메멘토를 오리지네이터 안의 `private` 중첩 클래스로 선언하고, 외부에는 비어 있는 인터페이스만 노출합니다.
- 오리지네이터는 중첩 클래스의 `private` 필드에 접근할 수 있습니다.
- 케어테이커는 인터페이스 타입 참조만 들고 있어 내부 데이터에 접근할 수 없습니다.

`Editor`는 오리지네이터입니다. 외부에는 `Memento` 마커 인터페이스만 노출하고, 텍스트와 커서 위치를 담은 `EditorMemento`는 `private static` 중첩 클래스로 숨깁니다. `restore()`는 전달받은 메멘토가 `EditorMemento`인지 확인한 뒤 상태를 되돌립니다.

{% include code-header.html %}
```java
public class Editor {
    private String text = "";
    private int cursorPosition = 0;

    // 외부에 노출되는 마커 인터페이스. 메서드가 없어 상태를 읽을 수 없다.
    public interface Memento {
    }

    // 실제 상태를 보관하는 구상 메멘토. 외부에서 접근할 수 없다.
    private static class EditorMemento implements Memento {
        private final String text;
        private final int cursorPosition;

        private EditorMemento(String text, int cursorPosition) {
            this.text = text;
            this.cursorPosition = cursorPosition;
        }
    }

    public void type(String words) {
        this.text += words;
        this.cursorPosition = this.text.length();
    }

    public Memento save() {
        return new EditorMemento(this.text, this.cursorPosition);
    }

    public void restore(Memento memento) {
        if (!(memento instanceof EditorMemento)) {
            throw new IllegalArgumentException("올바르지 않은 메멘토 객체입니다.");
        }
        EditorMemento editorMemento = (EditorMemento) memento;
        this.text = editorMemento.text;
        this.cursorPosition = editorMemento.cursorPosition;
    }
}
```

`History`는 케어테이커입니다. 메멘토를 스택에 쌓고 꺼내 주는 일만 합니다. `Editor.Memento` 타입만 다루므로 `text`나 `cursorPosition`을 열어 보거나 수정할 수 없습니다.

{% include code-header.html %}
```java
import java.util.ArrayDeque;
import java.util.Deque;

public class History {
    private final Deque<Editor.Memento> undoStack = new ArrayDeque<>();

    public void push(Editor.Memento memento) {
        undoStack.push(memento);
    }

    public Editor.Memento pop() {
        if (undoStack.isEmpty()) {
            return null;
        }
        return undoStack.pop();
    }

    public boolean isEmpty() {
        return undoStack.isEmpty();
    }
}
```

클라이언트는 상태를 바꾼 뒤 `save()`로 만든 메멘토를 `History`에 저장하고, 실행 취소할 때 `pop()`으로 꺼낸 메멘토를 `restore()`에 넘깁니다.

{% include code-header.html %}
```java
public class Main {
    public static void main(String[] args) {
        Editor editor = new Editor();
        History history = new History();

        editor.type("Hello");
        history.push(editor.save()); // 백업: "Hello"

        editor.type(", World!");
        history.push(editor.save()); // 백업: "Hello, World!"

        editor.type(" -- 잘못 입력한 텍스트");

        // 첫 번째 Undo: "Hello, World!"로 복원
        if (!history.isEmpty()) {
            editor.restore(history.pop());
        }

        // 두 번째 Undo: "Hello"로 복원
        if (!history.isEmpty()) {
            editor.restore(history.pop());
        }
    }
}
```

`Editor` 밖에는 `setText()` 같은 세터가 없고, `History`에서는 메멘토 내부 데이터를 조회하거나 조작하는 메서드가 보이지 않습니다. 실행 취소 이력 관리(스택 관리, 백업 횟수 제한 등)는 `History`가 전담하고 `Editor`는 편집과 스냅숏 생성·복원에만 집중합니다.

### 좁은/넓은 인터페이스 분리

중첩 클래스를 쓰기 어렵거나 클래스를 별도 파일로 분리해야 할 때 쓰는 GoF의 전통적인 방식입니다. 메멘토에 접근 수준이 다른 인터페이스를 둘 둡니다.

- 좁은 인터페이스: 케어테이커에게 노출합니다. 메서드가 비어 있거나 생성 시각, 이름 같은 메타데이터만 조회할 수 있습니다.
- 넓은 인터페이스: 오리지네이터만 사용합니다. 상태 복원에 필요한 실제 데이터를 꺼낼 수 있습니다.

케어테이커는 좁은 인터페이스 타입으로만 메멘토를 다루므로 상태를 조작할 수 없습니다. 클래스를 각각 별도 파일로 분리할 수 있고, 케어테이커가 저장 이름이나 시각 같은 메타데이터를 목록으로 보여 줘야 할 때 유연합니다.

`Memento`가 좁은 인터페이스이고, 상태를 담은 `ConcreteMemento` 자체가 넓은 인터페이스에 해당합니다. 오리지네이터인 `TextEditor`는 `restore()`에서 받은 `Memento`를 `ConcreteMemento`로 캐스팅해 상태를 꺼냅니다.

{% include code-header.html %}
```java
// 좁은 인터페이스: 케어테이커에게 노출
public interface Memento {
}

// 구상 메멘토: 상태를 담고 있으며 오리지네이터만 캐스팅해서 사용
public class ConcreteMemento implements Memento {
    final String state;

    ConcreteMemento(String state) {
        this.state = state;
    }
}

public class TextEditor { // 오리지네이터
    private String text = "";

    public void type(String words) {
        this.text += words;
    }

    public Memento save() {
        return new ConcreteMemento(text);
    }

    public void restore(Memento memento) {
        if (!(memento instanceof ConcreteMemento)) {
            throw new IllegalArgumentException("지원하지 않는 메멘토 구현체입니다.");
        }
        this.text = ((ConcreteMemento) memento).state;
    }
}
```

케어테이커는 `Memento` 타입만 다루므로 보관하고 되돌려 줄 수만 있습니다.

{% include code-header.html %}
```java
Deque<Memento> history = new ArrayDeque<>();

history.push(editor.save());
history.push(editor.save());

editor.restore(history.pop());
```

다만 외부에서 `(ConcreteMemento) memento`로 강제 캐스팅하면 상태를 읽을 수 있어서, 캡슐화는 중첩 클래스 방식보다 약합니다.

### 패키지 전용 접근 제어자 기반

Java 실무에서 가장 단순하게 쓰는 방식입니다.

- 메멘토 클래스는 `public`으로 열어 케어테이커가 타입으로 보관할 수 있게 합니다.
- 메멘토의 생성자와 상태 필드는 접근 제어자를 붙이지 않아 package-private으로 둡니다.
- 오리지네이터와 메멘토는 같은 패키지에 두고, 케어테이커와 클라이언트는 다른 패키지에 둡니다.
- 다른 패키지의 케어테이커는 메멘토를 생성하거나 상태에 접근하면 컴파일 에러가 납니다.

클래스를 각각 별도 파일로 분리할 수 있고, 인터페이스와 다운캐스팅 없이 구현할 수 있어 단순합니다.

{% include code-header.html %}
```java
package com.example.editor;

public class Snapshot { // 메멘토
    final String text;

    Snapshot(String text) {
        this.text = text;
    }
}

public class TextEditor { // 오리지네이터
    private String text = "";

    public void type(String words) {
        this.text += words;
    }

    public Snapshot save() {
        return new Snapshot(text);
    }

    public void restore(Snapshot snapshot) {
        this.text = snapshot.text;
    }
}
```

`SnapshotHolder`는 다른 패키지에 있는 케어테이커입니다. `Snapshot` 타입으로 보관하고 되돌려 줄 수만 있습니다.

{% include code-header.html %}
```java
package com.example.history;

import com.example.editor.Snapshot;

public class SnapshotHolder { // 케어테이커
    private final Deque<Snapshot> snapshots = new ArrayDeque<>();

    public void push(Snapshot snapshot) {
        // snapshot.text 접근 시 컴파일 에러
        snapshots.push(snapshot);
    }

    public Snapshot pop() {
        return snapshots.pop();
    }
}
```

클라이언트는 `history` 패키지에서 `editor.save()`로 만든 스냅숏을 보관하고, 실행 취소 때 `editor.restore(holder.pop())`으로 복원합니다.

다만 같은 패키지에 다른 개발자가 클래스를 추가하면 그 클래스도 `Snapshot`의 상태에 접근할 수 있어서, 패키지 구성을 규칙으로 지켜야 하고 중첩 클래스 방식보다 캡슐화가 약합니다.

# 장단점

메멘토 패턴의 장단점은 다음과 같습니다.

**장점**

- **캡슐화 원칙 준수**: 객체 내부의 세부 구현이나 private 필드를 외부에 노출하지 않고(불필요한 public Setter 없이) 상태를 백업하고 복원할 수 있습니다.
- **오리지네이터의 책임 단순화(SRP)**: 오리지네이터는 비즈니스 로직에만 집중하고, 스냅숏을 스택 등에 저장하고 꺼내주는 관리 책임은 케어테이커에게 위임됩니다.
- **상태의 불변성 보장**: 메멘토를 수정할 수 없는 읽기 전용 객체로 설계하면 저장된 스냅숏이 외부 코드에 의해 오염될 위험이 없습니다.

**단점**

- **메모리 낭비 위험**: 원본 객체가 크거나 스냅숏 생성이 매우 빈번하면(예: 글자를 한 자 입력할 때마다 백업) 스택에 객체가 누적되어 메모리 사용량이 급증할 수 있습니다.
- **생성 및 복원 시간 오버헤드**: 대규모 객체를 깊은 복사하거나 직렬화해야 하면 상당한 CPU 비용과 지연이 발생합니다.
- **케어테이커의 수명 주기 관리 부담**: 더 이상 필요하지 않은 스냅숏을 비우지 않으면 GC 대상이 되지 않아 메모리 누수가 생기므로, 최대 백업 개수 제한(예: 최근 20개만 유지) 같은 관리 정책이 필요합니다.

# 시간 및 공간 최적화

백업할 객체가 크거나 백업이 잦으면 메멘토 패턴은 시간과 메모리 비용이 커집니다. 객체의 데이터 크기를 N, 보관하는 스냅숏 개수를 K라고 하면 전체를 복사하는 기본 방식의 비용은 다음과 같습니다.

| 구분       | 복잡도      | 원인                        |
| -------- | -------- | ------------------------- |
| 백업 생성 시간 | O(N)     | 객체의 전체 상태를 새로 복제한다.       |
| 상태 복원 시간 | O(N)     | 스냅숏 데이터를 원본 객체로 복사해 교체한다. |
| 공간       | O(K × N) | 스냅숏마다 크기 N짜리 독립 복사본이 쌓인다. |

예를 들어 100MB 문서에서 글자를 입력할 때마다 스냅숏을 만들면, 10번만 백업해도 약 1GB의 메모리가 쓰이고 입력마다 지연이 생깁니다.

## 변경된 부분만 기록하기

텍스트 에디터처럼 직전 작업을 순차적으로 되돌리기만 하고 중간 시점으로 건너뛸 필요가 없을 때 사용합니다.

- 문서 전체를 복사하는 대신 직전에 입력한 텍스트의 길이만 스냅숏에 기록합니다.
- 실행 취소할 때는 현재 텍스트의 끝에서 그 길이만큼 잘라냅니다.

스냅숏마다 정수 하나만 저장하므로 공간은 O(K), 복원 시간은 O(1)로 줄어듭니다. 임의의 시점으로 복원해야 하는 경우에는 쓸 수 없습니다.

## 전체 백업과 증분 백업 조합

임의의 시점으로 복원해야 하거나, 데이터가 크고 자주 백업해야 하는 시스템에서 사용합니다.

- 전체 백업: 모든 데이터를 통째로 저장합니다. 낮은 빈도(하루 또는 며칠 간격)로 실행합니다.
- 증분 백업: 매 수정마다 변경 내용(diff, 작업 로그)만 기록합니다. 높은 빈도로 실행합니다.

복원할 시점에 전체 백업이 있으면 바로 복원합니다. 없으면 그 시점과 가장 가까운 이전 전체 백업을 복원한 뒤, 거기서부터 목표 시점까지의 증분 백업을 순서대로 다시 적용합니다. 전체를 복사하는 횟수가 줄어 시간과 공간 소모가 크게 줄어듭니다.

## 스냅숏 개수 제한

스냅숏을 무한정 쌓으면 GC 대상이 되지 않아 메모리 누수가 생깁니다. 케어테이커에 최대 보관 개수(예: 최근 20~50개)를 두고, 한도를 넘으면 가장 오래된 스냅숏부터 버리면 공간의 상한이 O(최대 개수 × N)으로 고정됩니다.

---

정리하면, 메멘토는 객체의 이전 상태를 캡슐화를 깨지 않고 저장하고 복원해야 할 때 쓰는 패턴입니다. 객체가 크거나 백업이 잦다면 변경분 기록이나 증분 백업, 보관 개수 제한을 함께 검토하고, 복원할 필요가 없다면 도입하지 않는 편이 낫습니다.

# 참고 자료

[**메멘토 패턴**](https://refactoring.guru/ko/design-patterns/memento)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
