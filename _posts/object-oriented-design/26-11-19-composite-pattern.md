---
title: 복합체 패턴 (구조와 재귀적 합성)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-19

permalink: "/object-oriented-design/composite-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 복합체를 다룹니다. 구조와 파일 시스템 크기 계산 예제를 통해 재귀적 합성으로 단일 객체와 복합 객체를 같은 방식으로 다루는 방법을 정리하겠습니다.

# 복합체 패턴이란

데이터(객체)들을 트리 구조의 부분-전체 계층 구조로 구성하여, 단일 객체인 리프와 복합 객체를 클라이언트가 구분 없이 동일한 방식으로 다룰 수 있도록 하는 구조 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 트리와 같은 객체 구조를 구현해야 하는 경우

파일 시스템의 디렉터리를 생각하면, 디렉터리 내부에 또 다른 하위 디렉터리나 파일을 계속해서 추가할 수 있습니다.

이를 복합체 패턴을 통해 구현하면, 디렉터리가 파일과 하위 디렉터리를 공통 인터페이스 타입으로 보관하는 '재귀적 합성'을 사용하여 깊이의 제한 없이 객체 트리를 손쉽게 구성할 수 있습니다.

- 클라이언트 코드가 단순 요소들과 복합 요소들을 모두 균일하게 처리하도록 하고자 하는 경우

마찬가지로 파일 시스템을 생각해 보면, 사용자는 대상이 단일 파일이든 여러 파일이 묶인 디렉터리든 상관없이 동일하게 '크기 확인'이나 '삭제' 요청을 보냅니다.

복합체 패턴을 적용하면 클라이언트가 대상의 구체적인 타입(파일인지 디렉터리인지)을 확인하는 if-else 분기문 없이, 공통 메서드 하나만 호출하여 전체 트리를 동일한 방식으로 처리할 수 있습니다.

# 구조

복합체 패턴은 다음과 같은 구조를 가집니다.

- 단일 객체와 복합 객체 모두가 구현해야 하는 공통 작업에 대한 컴포넌트 인터페이스를 제공합니다.
- 하위 요소를 가지지 않는 트리 구조의 리프인 단일 객체는 실제 작업을 정의합니다.
- 하위 요소들을 담을 수 있는 컨테이너 역할을 하는 복합 객체는 하위 요소에 대한 순회를 통해 결과를 만들고 반환합니다.
- 클라이언트는 컴포넌트 인터페이스를 통해 작업을 수행합니다.

# 구현 예시

파일 시스템에서 파일과 디렉터리의 크기를 구하는 예시입니다. 단일 파일이든 여러 파일이 담긴 디렉터리든 클라이언트는 같은 방식으로 크기를 요청합니다.

컴포넌트 인터페이스는 단일 객체와 복합 객체 모두가 구현하는 공통 작업을 선언합니다.

{% include code-header.html %}
```java
// 컴포넌트: 파일과 디렉터리가 함께 구현하는 공통 작업
public interface FileSystemNode {
    long getSize();
}
```

리프는 하위 요소를 가지지 않으며 실제 작업을 정의합니다. 파일은 자신의 크기를 그대로 반환합니다.

{% include code-header.html %}
```java
// 리프: 하위 요소가 없고 실제 크기를 가진다.
public class FileNode implements FileSystemNode {
    private final String name;
    private final long size;

    public FileNode(String name, long size) {
        this.name = name;
        this.size = size;
    }

    @Override
    public long getSize() {
        return size;
    }
}
```

복합체는 자식을 담는 컨테이너입니다. 자식은 파일일 수도, 다른 디렉터리일 수도 있으므로 공통 인터페이스 타입으로 보관합니다. 크기 계산은 자식들에게 위임하고 결과를 합산합니다. 자식이 디렉터리면 같은 위임이 그 아래로 이어지므로 트리 전체가 재귀적으로 순회됩니다.

{% include code-header.html %}
```java
// 복합체: 자식에게 작업을 위임하고 결과를 합산한다.
public class DirectoryNode implements FileSystemNode {
    private final String name;
    private final List<FileSystemNode> children = new ArrayList<>();

    public DirectoryNode(String name) {
        this.name = name;
    }

    public void add(FileSystemNode child) {
        children.add(child);
    }

    public void remove(FileSystemNode child) {
        children.remove(child);
    }

    @Override
    public long getSize() {
        long total = 0;
        for (FileSystemNode child : children) {
            total += child.getSize();
        }
        return total;
    }
}
```

클라이언트는 대상이 파일인지 디렉터리인지 확인하지 않고 `getSize()`만 호출합니다.

{% include code-header.html %}
```java
// 클라이언트 코드: 컴포넌트 인터페이스만 사용한다.
public class Application {
    public static void main(String[] args) {
        DirectoryNode root = new DirectoryNode("root");
        DirectoryNode docs = new DirectoryNode("docs");

        docs.add(new FileNode("report.txt", 100));
        docs.add(new FileNode("memo.txt", 50));
        root.add(docs);
        root.add(new FileNode("image.png", 200));

        FileSystemNode target = root;
        System.out.println(target.getSize()); // 350
    }
}
```

# 장단점

복합체 패턴의 장단점은 다음과 같습니다.

**장점**

- 다형성과 재귀를 사용하여 복잡한 트리 구조와 편리하게 작업이 가능합니다.
- 객체 트리와 작동하는 기존 코드를 훼손하지 않고 앱에 새로운 요소 유형들을 도입할 수 있기 때문에 개방-폐쇄 원칙을 준수할 수 있습니다.

**단점**

- 기능이 너무 다른 클래스를 대상으로 복합체 패턴을 구현하고자 하면 공통 인터페이스를 제공하기 어렵습니다.
- 컴포넌트 인터페이스를 과도하게 일반화하는 경우 코드를 이해하기 어렵게 만들 수 있습니다.

---

지금까지 살펴본 내용을 기준으로 복합체 패턴을 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 디렉터리와 파일처럼 트리 형태의 부분-전체 계층 구조를 구현해야 한다 | 쓴다 | 복합체가 자식을 공통 인터페이스 타입으로 보관하는 재귀적 합성으로 깊이 제한 없이 객체 트리를 구성할 수 있다 |
| 클라이언트가 단일 객체와 복합 객체를 구분하지 않고 같은 방식으로 처리해야 한다 | 쓴다 | 타입을 확인하는 if-else 분기 없이 공통 메서드 하나로 전체 트리를 처리할 수 있다 |
| 트리와 작동하는 기존 코드를 고치지 않고 새로운 요소 유형을 추가하고 싶다 | 쓴다 | 새 요소가 컴포넌트 인터페이스만 구현하면 되므로 개방-폐쇄 원칙을 지킬 수 있다 |
| 묶으려는 클래스들의 기능이 서로 너무 다르다 | 피한다 | 모두가 만족하는 공통 인터페이스를 제공하기 어렵다 |
| 컴포넌트 인터페이스에 여러 요소의 기능을 계속 일반화해서 넣고 있다 | 공통 작업만 남긴다 | 인터페이스를 과도하게 일반화하면 코드를 이해하기 어려워진다 |

# 참고 자료

[**복합체 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/composite)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
