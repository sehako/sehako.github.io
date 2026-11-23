---
title: 책임 연쇄 패턴 (구조와 Spring Security 필터 체인)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-25

permalink: "/object-oriented-design/chain-of-responsibility-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 책임 연쇄 패턴을 다룹니다. 리스트 기반과 연결 리스트 기반 구현 방법을 살펴본 뒤, Spring Security의 필터 체인이 이 패턴을 어떻게 사용하는지 정리하겠습니다.

# 책임 연쇄 패턴이란

클라이언트 코드의 요청을 처리할 수 있는 핸들러들 간 사슬(체인)을 구성하여, 요청의 유형을 확인하여 자신이 처리하거나 다른 핸들러에게 요청을 넘기도록 하는 행동 디자인 패턴입니다.

# 활용 상황

책임 연쇄 패턴은 주로 다음 상황에서 사용됩니다.

- 다양한 방식으로 다양한 종류의 요청들을 처리할 것으로 예상되지만 정확한 요청 유형들과 순서를 미리 알 수 없는 경우

요청을 보내는 쪽이 처리할 객체를 직접 고르지 않고 체인에 맡기므로, 처리할 수 있는 핸들러를 만날 때까지 요청이 전달됩니다.

- 특정 순서로 여러 핸들러를 실행해야 하는 경우

핸들러를 체인에 연결한 순서대로 요청이 전달되므로, 실행 순서를 체인 구성만으로 정할 수 있습니다.

- 핸들러들의 집합과 순서가 런타임에 변경되어야 하는 경우

체인은 핸들러에 대한 참조(또는 리스트)로 구성되므로, 실행 중에도 핸들러를 추가, 제거하거나 순서를 바꿀 수 있습니다.

- 이미 존재하는 객체 간의 포함 관계를 요청 전달 경로로 활용하려는 경우

GUI에서 버튼에 도움말이 없으면 버튼을 담고 있는 패널, 패널에도 없으면 다이얼로그 순으로 도움말을 찾는 경우가 대표적입니다. 컴포넌트의 부모(컨테이너)를 다음 핸들러로 삼으면 별도의 체인 관리자 없이 컴포넌트 트리가 곧 체인이 됩니다.

# 구조

책임 연쇄 패턴은 다음과 같은 구조를 가집니다.

- 핸들러(Handler) 인터페이스는 모든 구상 핸들러가 공통으로 따르는 요청 처리 메서드를 선언합니다. 체인의 다음 핸들러를 설정하는 메서드를 함께 선언하기도 합니다.
- 기초 핸들러(Base Handler)는 선택적으로 두는 클래스로, 다음 핸들러에 대한 참조를 보관하고 모든 핸들러에 공통인 전달 코드를 둡니다.
- 구상 핸들러(Concrete Handler)는 요청을 처리하는 실제 코드를 가집니다. 요청을 받으면 직접 처리할지, 체인의 다음 핸들러로 넘길지 결정합니다.
- 클라이언트는 체인을 한 번 구성해 두거나 상황에 따라 동적으로 구성합니다. 요청은 체인의 첫 번째 핸들러뿐만 아니라 어느 핸들러에든 보낼 수 있습니다.

## 구현 방법

핸들러를 엮는 방식은 크게 리스트 기반과 연결 리스트 기반으로 나뉩니다. 두 예시 모두 한 핸들러가 요청을 처리하면 체인을 중단하는 고전적인 방식입니다.

### 리스트 기반

체인 클래스가 핸들러를 `List`로 관리하고 순회합니다. 핸들러는 다음 핸들러를 알 필요가 없고, 순서를 바꾸거나 핸들러를 추가하기 쉽습니다.

핸들러 인터페이스는 요청을 처리했으면 `true`, 처리하지 못했으면 `false`를 반환합니다.

{% include code-header.html %}
```java
public interface IHandler {
    boolean handle(Request request);
}
```

구상 핸들러는 자신이 처리할 수 있는 요청인지 확인하고, 처리할 수 없으면 `false`를 반환해 다음 핸들러로 넘어가게 합니다.

{% include code-header.html %}
```java
public class HandlerA implements IHandler {
    @Override
    public boolean handle(Request request) {
        if (request.getType() == RequestType.TYPE_A) {
            // 요청 처리 로직
            return true;
        }
        return false;
    }
}

public class HandlerB implements IHandler {
    @Override
    public boolean handle(Request request) {
        if (request.getType() == RequestType.TYPE_B) {
            // 요청 처리 로직
            return true;
        }
        return false;
    }
}
```

체인은 핸들러를 순서대로 호출하고, 처리한 핸들러가 나오면 순회를 멈춥니다.

{% include code-header.html %}
```java
public class HandlerChain {
    private final List<IHandler> handlers = new ArrayList<>();

    public void addHandler(IHandler handler) {
        handlers.add(handler);
    }

    public void handle(Request request) {
        for (IHandler handler : handlers) {
            if (handler.handle(request)) {
                return;
            }
        }
        // 처리한 핸들러가 없는 경우의 정책(로그, 예외 등)을 정해야 합니다.
    }
}
```

### 연결 리스트 기반

각 핸들러가 다음 핸들러(`successor`)를 참조합니다. 하위 클래스가 `successor.handle()` 호출을 빠뜨리면 체인이 끊기므로, 추상 클래스의 템플릿 메서드로 전달 흐름을 고정하고 하위 클래스는 처리 로직(`doHandle()`)만 구현하게 할 수 있습니다.

추상 핸들러는 `successor`를 보관하고, 처리하지 못했을 때 다음 핸들러로 넘기는 흐름을 `final` 메서드로 고정합니다.

{% include code-header.html %}
```java
public abstract class Handler {
    protected Handler successor = null;

    public void setSuccessor(Handler successor) {
        this.successor = successor;
    }

    // 템플릿 메서드: 처리하지 못했고 다음 핸들러가 있으면 요청을 넘긴다.
    public final void handle(Request request) {
        boolean handled = doHandle(request);
        if (!handled && successor != null) {
            successor.handle(request);
        }
    }

    protected abstract boolean doHandle(Request request);
}
```

구상 핸들러는 전달 흐름을 신경 쓰지 않고 처리 로직만 구현합니다. 처리하지 못하면 `false`만 반환하면 부모의 `handle()`이 다음 핸들러로 넘깁니다.

{% include code-header.html %}
```java
public class ConcreteHandlerA extends Handler {
    @Override
    protected boolean doHandle(Request request) {
        if (request.getAmount() <= 1000) {
            // 요청 처리 로직
            return true;
        }
        return false;
    }
}

public class ConcreteHandlerB extends Handler {
    @Override
    protected boolean doHandle(Request request) {
        if (request.getAmount() <= 10000) {
            // 요청 처리 로직
            return true;
        }
        return false;
    }
}
```

체인은 `head`와 `tail`로 연결 리스트를 관리하고, 요청은 `head`부터 시작합니다. 핸들러를 추가할 때는 `tail`의 `successor`로 연결합니다.

{% include code-header.html %}
```java
public class HandlerChain {
    private Handler head = null;
    private Handler tail = null;

    public void addHandler(Handler handler) {
        handler.setSuccessor(null);
        if (head == null) {
            head = handler;
            tail = handler;
            return;
        }
        tail.setSuccessor(handler);
        tail = handler;
    }

    public void handle(Request request) {
        if (head != null) {
            head.handle(request);
        }
    }
}
```

### 변형: 모든 핸들러를 통과하는 방식

처리한 핸들러가 나와도 멈추지 않고 모든 핸들러를 차례로 거치는 변형도 있습니다. 민감 단어 필터링처럼 전처리, 검증, 변환을 단계별로 적용할 때 쓰며, 리스트 기반 구조에서 반환값 확인과 `return`만 빼면 됩니다.

{% include code-header.html %}
```java
public interface Filter {
    void doFilter(Content content);
}

public class FilterChain {
    private final List<Filter> filters = new ArrayList<>();

    public void addFilter(Filter filter) {
        filters.add(filter);
    }

    public void execute(Content content) {
        for (Filter filter : filters) {
            filter.doFilter(content);
        }
    }
}
```

# 장단점

책임 연쇄 패턴의 장단점은 다음과 같습니다.

**장점**

- **요청 처리 순서의 유연한 제어**: 요청을 처리하는 핸들러들의 실행 순서를 손쉽게 제어할 수 있으며, 필요에 따라 런타임에 체인의 순서를 재구성하거나 핸들러를 동적으로 추가/제거할 수 있습니다.
- **단일 책임 원칙(SRP) 준수**: 작업을 요청(호출)하는 클래스와 실제로 작업을 처리하는 클래스들을 분리(디커플링)하여, 거대했던 검증 및 처리 작업을 각각의 독립된 핸들러 클래스로 제공할 수 있습니다.
- **개방-폐쇄 원칙(OCP) 준수**: 기존의 클라이언트 코드나 이미 구성된 다른 핸들러들을 수정하지 않고도, 새로운 핸들러 클래스를 만들어 체인에 손쉽게 끼워 넣을 수 있습니다.

**단점**

- **요청의 유실 가능성**: 클라이언트가 요청의 처리를 기대하지만, 요청에 맞는 핸들러가 없는 경우 요청이 처리되지 않고 무시당할 수 있습니다.
- **디버깅의 어려움**: 여러 핸들러를 거쳐가기 때문에 디버깅 시 어떤 핸들러에서 문제가 발생했는지 추적하기 어렵습니다.
- **순환 참조 위험**: 핸들러 체인을 순환 참조 구조로 연결하면 무한 루프 또는 스택 오버플로우가 발생할 위험이 있습니다.

# Spring Security와 책임 연쇄 패턴

Spring Security의 필터 체인은 책임 연쇄 패턴의 대표적인 사례입니다. CSRF 보호, 인증, 인가 같은 책임을 각각의 `Filter`가 맡고, `SecurityFilterChain`에 연결된 순서대로 요청이 전달됩니다. 필터는 `addFilterBefore()` 등으로 기존 코드를 수정하지 않고 추가할 수 있습니다.

고전적인 방식과 다른 점은 다음 핸들러로 넘길지를 체인이 아니라 필터가 직접 결정한다는 것입니다. 필터는 `doFilter()`를 호출해 요청을 넘기고, 요청을 거부할 때는 호출하지 않아 체인을 끊습니다. 또 `doFilter()` 호출 전후에 모두 로직을 둘 수 있어서, 뒤쪽 필터가 끝난 뒤 후처리를 하거나 뒤쪽 필터가 던진 예외를 잡을 수도 있습니다.

{% include code-header.html %}
```java
public void doFilter(ServletRequest request, ServletResponse response, FilterChain filterChain)
        throws IOException, ServletException {
    if (hasAccess(request)) {
        filterChain.doFilter(request, response);
        return;
    }
    throw new AccessDeniedException("Access denied");
}
```

이것이 가능한 이유는 체인 객체가 필터 목록과 현재 위치를 들고 있으면서 필터를 호출할 때 자기 자신을 넘기기 때문입니다. 아래는 `FilterChainProxy`의 `VirtualFilterChain`을 단순화한 코드입니다.

{% include code-header.html %}
```java
private static final class VirtualFilterChain implements FilterChain {
    private final FilterChain originalChain;
    private final List<Filter> additionalFilters;
    private final int size;
    private int currentPosition = 0;

    public void doFilter(ServletRequest request, ServletResponse response) {
        if (currentPosition == size) {
            originalChain.doFilter(request, response);
            return;
        }
        currentPosition++;
        Filter nextFilter = additionalFilters.get(currentPosition - 1);
        nextFilter.doFilter(request, response, this);
    }
}
```

필터가 `filterChain.doFilter()`를 호출하면 다음 필터의 `doFilter()`가 실행되고, 마지막 필터까지 호출이 쌓였다가 풀리면서 각 필터의 호출 이후 코드가 실행됩니다.

## 필터와 인터셉터의 구현 차이

스프링 MVC의 `HandlerInterceptor`도 요청이 여러 인터셉터를 차례로 거치므로 책임 연쇄 패턴으로 볼 수 있습니다. 다만 필터가 `doFilter()` 호출을 중첩해 전후 처리를 한 메서드에 묶는 것과 달리, `HandlerExecutionChain`이 인터셉터 목록을 반복문으로 돌며 호출 시점별 메서드(`preHandle`, `postHandle`, `afterCompletion`)를 따로 부릅니다.

{% include code-header.html %}
```java
// HandlerExecutionChain 단순화
boolean applyPreHandle(...) {
    for (int i = 0; i < interceptorList.size(); i++) {
        if (!interceptorList.get(i).preHandle(...)) {
            triggerAfterCompletion(...);  // 성공한 인터셉터까지만 호출
            return false;
        }
        interceptorIndex = i;
    }
    return true;
}

void applyPostHandle(...) {
    for (int i = interceptorList.size() - 1; i >= 0; i--) {  // 전체 역순
        interceptorList.get(i).postHandle(...);
    }
}

void triggerAfterCompletion(...) {
    for (int i = interceptorIndex; i >= 0; i--) {  // 성공한 것만 역순
        interceptorList.get(i).afterCompletion(...);
    }
}
```

| 구분 | 서블릿 필터, Spring Security | 스프링 인터셉터 |
| --- | --- | --- |
| 전달 방식 | `chain.doFilter(request, response)` 중첩 호출 | `HandlerExecutionChain`의 반복문 |
| 전후 처리 | 한 `doFilter()` 안에서 호출 앞뒤에 작성 | `preHandle`, `postHandle`, `afterCompletion`으로 분리 |
| 체인 중단 | `doFilter()`를 호출하지 않고 반환 | `preHandle()`이 `false` 반환 |
| 후처리 순서 | 호출 스택이 풀리며 자연스럽게 역순 | 반복문 인덱스를 거꾸로 돌려 역순 |
| 중단 시 후처리 | 호출한 쪽의 `finally` 등에서 직접 보장 | `postHandle`은 건너뛰고, `afterCompletion`만 성공한 인터셉터까지 역순 호출 |

---

정리하면, 책임 연쇄는 요청의 처리자를 미리 알 수 없거나 여러 핸들러를 순서대로 거치며 그 구성이 바뀔 때 요청의 전달을 체인으로 분리하는 패턴입니다. 모든 대상에게 알려야 한다면 옵서버가 낫고, 핸들러가 한두 개이고 구성이 고정이라면 조건 분기를 유지하는 편이 낫습니다.

# 참고 자료

[**책임 연쇄 패턴**](https://refactoring.guru/ko/design-patterns/chain-of-responsibility)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)

[**Spring Security Architecture**](https://docs.spring.io/spring-security/reference/servlet/architecture.html)

[**HandlerExecutionChain 소스**](https://github.com/spring-projects/spring-framework/blob/main/spring-webmvc/src/main/java/org/springframework/web/servlet/HandlerExecutionChain.java)

[**DispatcherServlet 소스**](https://github.com/spring-projects/spring-framework/blob/main/spring-webmvc/src/main/java/org/springframework/web/servlet/DispatcherServlet.java)
