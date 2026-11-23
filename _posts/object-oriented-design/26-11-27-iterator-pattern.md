---
title: 반복자 패턴 (구조, Fail-Fast, 스냅숏)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: false
date: 2026-11-27

permalink: "/object-oriented-design/iterator-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 반복자 패턴을 다룹니다. 동적 배열 기반 리스트의 순회를 반복자로 분리해 보고, 순회 중 컬렉션을 수정할 때 생기는 문제와 자바의 Fail-Fast 동작, 수정이 있어도 끝까지 순회하는 스냅숏 반복자, 반복자 패턴을 도입해야 하는 기준을 정리하겠습니다.

# 반복자 패턴이란

커서 패턴이라고도 하며, 컬렉션의 순회 작업을 컬렉션에서 분리한 뒤, 반복자를 통해 컬렉션의 책임과 반복자의 책임을 나누는 행동 디자인 패턴입니다.

자바에서는 `Iterator`를 통해 컬렉션을 순회할 수 있도록 하는 것이 바로 반복자 패턴이라고 볼 수 있습니다.

# 활용 상황

반복자 패턴은 주로 다음 상황에서 사용됩니다.

- 컬렉션 내부의 복잡한 데이터 구조를 숨긴 채 일관된 방식으로 순회하게 하고 싶은 경우

배열, 연결 리스트뿐 아니라 트리(전위/중위/후위)나 그래프(DFS/BFS)처럼 복잡한 자료구조도 내부 표현을 감춘 채 `hasNext()` / `next()` 호출만으로 같은 방식으로 탐색할 수 있습니다.

- 데이터 저장 책임과 순회 알고리즘 책임을 분리하여 코드 중복을 줄이고 싶은 경우

컬렉션 클래스에 순회 알고리즘을 모두 넣으면 컬렉션 코드가 비대해집니다. 순회 방식을 독립된 반복자 클래스로 분리하면 컬렉션은 본연의 책임에 집중할 수 있고, 애플리케이션 전반의 순회 코드 중복도 없앨 수 있습니다.

- 동일한 컬렉션에 대해 여러 순회 작업을 동시에 독립적으로 진행하고 싶은 경우

반복자 객체마다 자신의 커서(`cursor`)와 순회 상태를 따로 보관하므로, 하나의 컬렉션에 여러 반복자가 서로 간섭 없이 동시에 순회할 수 있습니다.

- 순회 대상 데이터 구조의 구체적인 유형(배열, 리스트, 트리 등)에 종속되지 않고 범용 순회 코드를 작성하고자 하는 경우

반복자를 공통 인터페이스(`Iterable`, `Iterator`)로 추상화하여, 클라이언트 코드가 구상 클래스를 알 필요 없이 동일한 순회 로직으로 다양한 데이터 구조를 일관되게 처리할 수 있습니다.

# 구조

반복자 패턴은 다음과 같은 구조를 가집니다.

- 반복자(Iterator) 인터페이스는 순회에 필요한 공통 메서드(`hasNext()`, `next()`)를 선언합니다.
- 구상 반복자(Concrete Iterator)는 컬렉션에 대한 참조와 현재 순회 위치(커서)를 보관하고 구체적인 순회 알고리즘을 구현합니다.
- 컬렉션(Collection) 인터페이스는 자신과 호환되는 반복자를 만들어 반환하는 팩토리 메서드(`iterator()`)를 선언합니다.
- 구상 컬렉션(Concrete Collection)은 실제 데이터를 보관하고, `iterator()` 호출 시 자신을 넘겨 구상 반복자를 생성합니다.
- 클라이언트는 구상 클래스가 아니라 반복자와 컬렉션 인터페이스에만 의존합니다.

## 구현 방법

동적 배열 기반 리스트를 순회하는 예시입니다. 아래 `Iterator`, `List`, `ArrayList`는 패턴 구조를 보이기 위해 직접 정의한 것으로, `java.util`의 것과 이름만 같습니다.

반복자 인터페이스는 `next()`가 현재 요소를 반환하면서 커서를 한 칸 옮기도록 정의합니다. 컬렉션 인터페이스는 반복자를 만드는 `iterator()`를 선언합니다.

{% include code-header.html %}
```java
public interface Iterator<E> {
    boolean hasNext();
    E next();
}

public interface List<E> {
    void add(E element);
    E get(int index);
    int size();

    // 반복자를 생성하는 팩토리 메서드
    Iterator<E> iterator();
}
```

구상 컬렉션은 데이터 보관만 맡고, 순회는 `iterator()`가 돌려주는 반복자에 맡깁니다.

{% include code-header.html %}
```java
public class ArrayList<E> implements List<E> {

    private Object[] elements = new Object[10];
    private int size = 0;

    @Override
    public void add(E element) {
        if (size >= elements.length) {
            elements = java.util.Arrays.copyOf(elements, elements.length * 2);
        }
        elements[size++] = element;
    }

    @Override
    @SuppressWarnings("unchecked")
    public E get(int index) {
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("인덱스 범위 초과: " + index);
        }
        return (E) elements[index];
    }

    @Override
    public int size() {
        return size;
    }

    @Override
    public Iterator<E> iterator() {
        return new ArrayIterator<>(this);
    }
}
```

구상 반복자는 컬렉션 참조와 커서를 자신이 가집니다. 커서가 반복자마다 따로 있으므로 같은 컬렉션에서 여러 반복자를 만들어도 서로의 순회 상태에 영향을 주지 않습니다.

{% include code-header.html %}
```java
public class ArrayIterator<E> implements Iterator<E> {

    private final List<E> list;
    private int cursor = 0;

    public ArrayIterator(List<E> list) {
        this.list = list;
    }

    @Override
    public boolean hasNext() {
        return cursor < list.size();
    }

    @Override
    public E next() {
        if (!hasNext()) {
            throw new java.util.NoSuchElementException("더 이상 순회할 요소가 없다.");
        }
        return list.get(cursor++);
    }
}
```

클라이언트는 `Iterator` 인터페이스만 알면 되므로 컬렉션의 내부 구조와 무관하게 같은 코드로 순회합니다.

{% include code-header.html %}
```java
public class Main {

    public static <T> int count(Iterator<T> iterator) {
        int count = 0;
        while (iterator.hasNext()) {
            iterator.next();
            count++;
        }
        return count;
    }

    public static void main(String[] args) {
        List<String> names = new ArrayList<>();
        names.add("디자인 패턴");
        names.add("반복자 패턴");
        names.add("객체지향");

        int total = count(names.iterator()); // 3

        // 반복자마다 커서가 독립적이다.
        Iterator<String> iter1 = names.iterator();
        Iterator<String> iter2 = names.iterator();
        iter1.next();                 // 디자인 패턴
        String first = iter2.next();  // 디자인 패턴 (iter1의 영향을 받지 않음)
        String second = iter1.next(); // 반복자 패턴
    }
}
```

# 장단점

**장점**

- **단일 책임 원칙(SRP) 준수**: 순회 알고리즘을 별도의 클래스로 추출하여 컬렉션이 가졌던 반복에 대한 책임을 분리합니다.
- **개방-폐쇄 원칙(OCP) 준수**: 기존 컬렉션 코드나 클라이언트를 수정하지 않고도 새로운 순회 알고리즘을 담은 반복자 클래스를 확장할 수 있습니다.
- **독립적인 동시 순회 지원**: 순회 진행 상태가 개별 반복자 객체 내부에 저장되기 때문에 여러 순회 작업을 지연하거나 병렬로 실행할 수 있습니다.

**단점**

- **오버 엔지니어링**: 단순 배열이나 기본 리스트처럼 인덱스로도 충분히 가능한 구조에서는 코드의 복잡도만 증가할 수 있습니다.
- **성능 오버헤드**: 메서드를 통해 반복을 수행하므로 직접 인덱스로 접근하는 것과 비교했을 때 성능이 미세하게 떨어질 수 있습니다.

# 순회 중 컬렉션 수정과 Fail-Fast

순회 도중 컬렉션의 `add()`나 `remove()`를 직접 호출하면 커서 위치가 어긋나 결과를 예측할 수 없습니다. `[a, b, c, d]`에서 커서가 `b`를 가리킬 때를 예로 들면 다음과 같습니다.

- **삭제 시 요소 누락**: `remove("a")`로 요소가 앞으로 당겨지면 커서 자리에 `c`가 와서 `b`를 건너뜁니다.
- **추가 시 요소 중복**: `add(0, "x")`로 요소가 뒤로 밀리면 커서 자리에 이미 처리한 `a`가 다시 옵니다.

순회 중 수정을 잠그는 방식은 순회가 언제 끝나는지 컬렉션이 알 수 없어서(`break`로 중간에 빠져나오는 경우가 흔합니다) 잠금이 풀리지 않을 위험이 있습니다. 그래서 자바 컬렉션은 변경을 감지하면 즉시 `ConcurrentModificationException`을 던지는 Fail-Fast 방식을 택했습니다.

컬렉션은 구조적 변경 때마다 `modCount`를 증가시키고, 반복자는 생성 시점의 값을 `expectedModCount`로 복사해 두었다가 `next()`마다 비교합니다.

{% include code-header.html %}
```java
private void checkForComodification() {
    if (modCount != expectedModCount) {
        throw new ConcurrentModificationException();
    }
}
```

이 검사는 최선의 노력 수준입니다. `ArrayList`의 `hasNext()`는 `cursor != size`만 확인하므로, 끝에서 두 번째 요소를 삭제하면 예외 없이 순회가 끝날 수 있습니다.

순회 중 삭제는 반복자의 `remove()`로 해야 합니다. 삭제 후 커서를 `lastRet`로 되돌리고 `expectedModCount = modCount`로 동기화해서 이후 검사를 통과합니다. `next()` 한 번에 한 번만 호출할 수 있고, 연달아 호출하면 `IllegalStateException`이 발생합니다.

같은 컬렉션의 반복자 `iter1`, `iter2` 중 `iter1.remove()`를 호출하면 `iter2`의 `expectedModCount`는 갱신되지 않아서, 이후 `iter2.next()`는 `ConcurrentModificationException`을 던집니다.


# 스냅숏 반복자

Fail-Fast가 변경을 감지해 순회를 중단시키는 방어적 접근이라면, 스냅숏 반복자는 순회 중 원본이 바뀌어도 예외 없이 끝까지 순회하게 하는 접근입니다.

반복자를 생성한 시점의 컬렉션 상태를 기준으로 순회하므로, 이후 원본에서 요소를 추가하거나 삭제해도 영향을 받지 않습니다. `[3, 8, 2]`에서 `iter1`을 만든 뒤 원본에서 `2`를 삭제해도 `iter1`은 `3, 8, 2`를 모두 순회합니다.

스냅숏을 구현하는 방법은 크게 두 가지입니다.

## 전체 복사본 방식

반복자를 만들 때 컬렉션의 요소를 반복자 내부에 복사해 두고 그 복사본을 순회합니다.

{% include code-header.html %}
```java
public class SnapshotArrayIterator<E> implements Iterator<E> {

    private int cursor = 0;
    private final ArrayList<E> snapshot;

    public SnapshotArrayIterator(ArrayList<E> arrayList) {
        this.snapshot = new ArrayList<>();
        this.snapshot.addAll(arrayList); // 생성 시점에 전체 요소를 얕은 복사
    }

    @Override
    public boolean hasNext() {
        return cursor < snapshot.size();
    }

    @Override
    public E next() {
        return snapshot.get(cursor++);
    }
}
```

구현이 단순하지만 반복자를 만들 때마다 복사본을 새로 할당합니다. 반복자를 자주 만들거나 컬렉션이 크면 메모리와 생성 시간 부담이 커집니다. 자바의 복사는 참조만 복사하는 얕은 복사라서 요소 객체까지 중복 생성되지는 않습니다.

## 타임스탬프 방식

요소마다 추가 시각(`addTimestamp`)과 삭제 시각(`delTimestamp`)을 기록해 스냅숏에 속하는 요소인지 판별합니다.

- 추가할 때는 `addTimestamp`에 현재 시각을, `delTimestamp`에 `Long.MAX_VALUE`를 기록합니다.
- 삭제할 때는 메모리에서 지우지 않고 `delTimestamp`만 현재 시각으로 바꿉니다.
- 반복자를 만들 때는 현재 시각을 `snapshotTimestamp`로 보관합니다.

반복자는 `addTimestamp < snapshotTimestamp < delTimestamp`인 요소만 순회합니다. 반복자 생성 전에 추가되었고, 생성 시점에는 아직 삭제되지 않은 요소라는 뜻입니다.

시각은 `System.currentTimeMillis()` 대신 증가하는 논리 시계로 가정합니다. 밀리초 시각을 쓰면 같은 시각에 일어난 추가·삭제·반복자 생성이 `<` 비교에서 누락될 수 있기 때문입니다. 반복자는 생성 시점의 시각을 `snapshotTimestamp`로 받아 보관합니다.

{% include code-header.html %}
```java
public class Entry<E> {
    final E item;
    final long addTimestamp;
    long delTimestamp = Long.MAX_VALUE; // 삭제 전에는 최댓값

    Entry(E item, long addTimestamp) {
        this.item = item;
        this.addTimestamp = addTimestamp;
    }
}

public class TimestampSnapshotIterator<E> implements Iterator<E> {

    private final List<Entry<E>> entries;
    private final long snapshotTimestamp;
    private int cursor = 0;

    public TimestampSnapshotIterator(List<Entry<E>> entries, long snapshotTimestamp) {
        this.entries = entries;
        this.snapshotTimestamp = snapshotTimestamp;
    }

    @Override
    public boolean hasNext() {
        skipOutOfSnapshot();
        return cursor < entries.size();
    }

    @Override
    public E next() {
        if (!hasNext()) {
            throw new NoSuchElementException("더 이상 순회할 요소가 없다.");
        }
        return entries.get(cursor++).item;
    }

    // 스냅숏에 속하지 않는 요소는 건너뛴다.
    private void skipOutOfSnapshot() {
        while (cursor < entries.size()) {
            Entry<E> entry = entries.get(cursor);
            if (entry.addTimestamp < snapshotTimestamp && snapshotTimestamp < entry.delTimestamp) {
                return;
            }
            cursor++;
        }
    }
}
```

`[3, 8, 2]`가 시각 1, 2, 3에 추가된 상태에서 시각 4에 반복자를 만들었다고 합시다. 이후 시각 5에 `2`를 삭제하고 시각 6에 `7`을 추가해도 `iter1`은 `3, 8, 2`를 순회합니다. `2`는 `4 < 5`라서 삭제 전 요소이고, `7`은 `6 < 4`가 거짓이라서 스냅숏 이후 요소입니다. 이후 만든 반복자는 `3, 8, 7`을 순회합니다.

## 방식 비교

| 구분 | Fail-Fast 반복자 | 스냅숏 반복자(복사본) | 스냅숏 반복자(타임스탬프) |
| --- | --- | --- | --- |
| 순회 중 원본 수정 | `ConcurrentModificationException` 발생 | 예외 없이 생성 시점 기준으로 순회 | 예외 없이 생성 시점 기준으로 순회 |
| 반복자 생성 비용 | `O(1)` (참조만 유지) | `O(N)` (요소 복사) | `O(1)` (시각만 기록) |
| 추가 메모리 | 거의 없음 (커서 등) | 반복자 생성 횟수에 비례 | 요소별 타임스탬프와 삭제 표시된 요소 |

---

지금까지 살펴본 내용을 기준으로 반복자 패턴을 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 트리나 그래프처럼 내부 구조가 복잡한 컬렉션을 클라이언트가 같은 방식으로 순회해야 한다 | 쓴다 | `hasNext()` / `next()`만 노출하면 내부 표현을 숨긴 채 전위·중위·후위, DFS·BFS 같은 순회 방식을 반복자로 바꿔 끼울 수 있다 |
| 같은 컬렉션을 여러 곳에서 동시에 독립적으로 순회해야 한다 | 쓴다 | 커서가 반복자마다 따로 있어서 한 순회가 다른 순회에 영향을 주지 않는다 |
| 컬렉션에 순회 방식이 계속 추가되어 컬렉션 클래스가 비대해진다 | 쓴다 | 순회 알고리즘을 반복자 클래스로 분리하면 컬렉션 코드를 바꾸지 않고 새 반복자를 추가할 수 있다(OCP) |
| 배열이나 기본 리스트처럼 인덱스나 향상된 `for`문으로 충분하다 | 피한다 | 반복자를 직접 만들면 클래스만 늘고 얻는 것이 없다. 자바 컬렉션은 이미 `Iterable`을 제공한다 |
| 순회 도중 컬렉션이 수정될 수 있다 | 수정 방식을 검토한다 | 직접 `add()`/`remove()`를 호출하면 요소 누락이나 `ConcurrentModificationException`이 생기므로 `iterator.remove()`를 쓰거나 순회와 수정을 분리한다 |
| 성능이 민감한 반복문에서 인덱스 접근이 가능하다 | 인덱스 접근을 검토한다 | 반복자는 메서드 호출을 거치므로 직접 인덱스 접근보다 미세하게 느리다 |

정리하면, 반복자는 컬렉션의 내부 구조를 숨긴 채 일관된 방식으로, 그리고 여러 순회를 독립적으로 진행해야 할 때 컬렉션과 순회 책임을 분리하는 패턴입니다. 단순 배열이나 리스트라면 언어가 제공하는 반복 구문으로 충분하고, 순회 중 수정이 필요하다면 반복자의 `remove()`를 쓰거나 수정 방식을 먼저 따져 봐야 합니다.

# 참고 자료

[**반복자 패턴**](https://refactoring.guru/ko/design-patterns/iterator)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
