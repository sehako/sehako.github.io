---
title: 프록시 패턴 (구조와 스프링 AOP 프록시)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-14

permalink: "/object-oriented-design/proxy-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 프록시를 다룹니다. 구조와 캐싱 예제를 살펴본 뒤, 스프링 AOP와 `@Transactional`이 프록시로 동작하는 방식과 그 제약을 정리하겠습니다.

# 프록시 패턴이란

프록시 패턴은 원본 객체에 대한 접근을 제어하거나, 원본 객체와 연관이 없는 어떤 기능을 추가할 때 사용하는 구조 디자인 패턴입니다.

이때 원본 객체를 서비스 객체라고 부르며, 프록시 패턴은 서비스 객체와 같은 인터페이스로 프록시 클래스를 구현합니다.

주로 다음 상황에서 사용합니다.

- 간헐적으로 사용되는 무거운 객체에 대한 가상 프록시를 제공하는 경우

앱 시작 시 객체를 생성하는 대신, 실제로 필요한 시점까지 초기화를 지연할 수 있습니다.

- 특정 클라이언트 코드만 객체를 사용할 수 있도록 접근 제어를 하려는 경우

클라이언트의 자격 증명에 따라서 서비스 객체 접근 가능 여부를 판단할 수 있습니다.

- 원격 서버에 있는 서비스 객체에 접근하는 경우

프록시가 네트워크 통신의 복잡성을 은닉하여 클라이언트가 원격 객체를 로컬 객체처럼 사용할 수 있도록 합니다.

- 어떤 동작에 대한 로깅을 처리하려는 경우

서비스 객체의 어떤 기능을 실행하기 전에 로그를 남길 수 있습니다.

- 클라이언트의 요청 결과를 캐싱하고 수명주기를 관리하려는 경우

무겁고 항상 같은 결과를 생성하는 경우 캐싱을 통해 연산 부담을 완화할 수 있습니다.

- 참조하는 클라이언트가 없을 때 객체를 해제해야 하는 경우

프록시에서 클라이언트를 점검하게 만들어, 참조하는 클라이언트가 없다면 서비스 객체를 정리해 서비스 자원을 절약할 수 있습니다.

# 구조

프록시 패턴은 다음과 같은 구조를 가집니다.

- 서비스 객체에 대한 인터페이스를 제공합니다.
- 프록시가 해당 인터페이스를 구현하고, 내부적으로 서비스 객체에 의존합니다.
- 클라이언트 코드는 인터페이스의 구현체로 프록시를 전달받습니다.

# 구현 예시

서드파티 유튜브 라이브러리의 비디오 다운로더가 있다고 가정합니다. 이 라이브러리는 요청 결과를 저장하지 않아서, 같은 비디오를 여러 번 요청하면 매번 유튜브와 통신해 다시 다운로드하는 비효율적인 구조라고 가정합니다. 이를 개선하기 위해 프록시를 도입해 보겠습니다.

라이브러리 코드는 수정할 수 없거나 `final`일 수 있으므로 서비스 클래스를 직접 고치지 않습니다. 대신 서비스 클래스와 같은 인터페이스를 구현하는 프록시에 캐싱 코드를 넣고, 실제 요청이 필요한 경우에만 서비스 객체에 위임합니다. 캐시는 `id`별로 관리하며, 캐시를 언제 무효화할지(`needReset`)는 정책에 따라 별도로 정해야 합니다.

서비스 객체와 프록시가 함께 구현할 인터페이스를 선언합니다.

{% include code-header.html %}
```java
// 원격 서비스의 인터페이스
public interface ThirdPartyYouTubeLib {
    List<Video> listVideos();
    VideoInfo getVideoInfo(String id);
    void downloadVideo(String id);
}
```

서비스 객체는 요청마다 유튜브에 접근합니다. 요청 속도는 네트워크 상태에 따라 달라지고, 같은 정보를 요청해도 매번 느립니다.

{% include code-header.html %}
```java
// 서비스 객체: 요청마다 유튜브와 통신한다.
public class ThirdPartyYouTubeClass implements ThirdPartyYouTubeLib {
    @Override
    public List<Video> listVideos() {
        // 유튜브에 API 요청을 보낸다.
    }

    @Override
    public VideoInfo getVideoInfo(String id) {
        // 비디오의 메타데이터를 가져온다.
    }

    @Override
    public void downloadVideo(String id) {
        // 유튜브에서 비디오 파일을 다운로드한다.
    }
}
```

프록시는 서비스 객체를 필드로 가지고, 같은 인터페이스를 구현합니다. 캐시에 결과가 없거나 초기화가 필요할 때만 서비스 객체에 위임합니다.

{% include code-header.html %}
```java
// 프록시: 요청 결과를 캐시하고, 필요한 경우에만 서비스 객체에 위임한다.
public class CachedYouTubeClass implements ThirdPartyYouTubeLib {
    private final ThirdPartyYouTubeLib service;
    private List<Video> listCache;
    private final Map<String, VideoInfo> videoCache = new HashMap<>();
    private final Set<String> downloaded = new HashSet<>();
    private boolean needReset;

    public CachedYouTubeClass(ThirdPartyYouTubeLib service) {
        this.service = service;
    }

    @Override
    public List<Video> listVideos() {
        if (listCache == null || needReset) {
            listCache = service.listVideos();
        }
        return listCache;
    }

    @Override
    public VideoInfo getVideoInfo(String id) {
        if (!videoCache.containsKey(id) || needReset) {
            videoCache.put(id, service.getVideoInfo(id));
        }
        return videoCache.get(id);
    }

    @Override
    public void downloadVideo(String id) {
        if (!downloaded.contains(id) || needReset) {
            service.downloadVideo(id);
            downloaded.add(id);
        }
    }
}
```

클라이언트는 인터페이스 타입만 사용하므로 서비스 객체 대신 프록시를 전달해도 코드를 바꿀 필요가 없습니다.

{% include code-header.html %}
```java
// 클라이언트: 서비스 객체인지 프록시인지 알지 못한다.
public class YouTubeManager {
    protected final ThirdPartyYouTubeLib service;

    public YouTubeManager(ThirdPartyYouTubeLib service) {
        this.service = service;
    }

    public void renderVideoPage(String id) {
        VideoInfo info = service.getVideoInfo(id);
        // 비디오 페이지를 렌더링한다.
    }

    public void renderListPanel() {
        List<Video> list = service.listVideos();
        // 비디오 섬네일 리스트를 렌더링한다.
    }

    public void reactOnUserInput(String id) {
        renderVideoPage(id);
        renderListPanel();
    }
}
```

프록시는 앱이 객체를 조립하는 시점에 설정합니다.

{% include code-header.html %}
```java
public class Application {
    public void init() {
        ThirdPartyYouTubeLib aYouTubeService = new ThirdPartyYouTubeClass();
        ThirdPartyYouTubeLib aYouTubeProxy = new CachedYouTubeClass(aYouTubeService);
        YouTubeManager manager = new YouTubeManager(aYouTubeProxy);
        manager.reactOnUserInput("video-id");
    }
}
```

# 장단점

프록시 패턴의 장단점은 다음과 같습니다.

**장점**

- 서비스 객체와 관련된 제어 및 관리를 클라이언트로부터 은닉할 수 있습니다.
- 서비스 객체의 생성과 수명주기를 프록시가 관리하므로, 서비스 객체가 아직 생성되지 않았거나 일시적으로 사용할 수 없는 경우에도 클라이언트는 프록시를 사용할 수 있습니다. (가상 프록시의 지연 초기화가 대표적입니다.)
- 서비스 객체나 클라이언트를 변경하지 않고 새로운 프록시를 도입할 수 있습니다.

**단점**

- 코드의 복잡성이 높아질 수 있습니다.
- 프록시가 수행하는 기능이 방대해질수록 서비스 객체의 응답에 대한 지연이 발생할 수 있습니다.

# 스프링과 프록시 패턴

스프링은 관점 지향 프로그래밍(AOP), JPA의 지연 로딩 등 다양한 기능에서 프록시 패턴을 활용합니다.

트랜잭션, 로깅, 보안 검사처럼 여러 클래스에 반복되는 부가 기능을 횡단 관심사라고 합니다. 이를 각 클래스에 직접 작성하면 비즈니스 로직과 섞이고 중복이 생기므로, 스프링 AOP는 대상 객체 앞에 프록시를 두고 호출을 가로채 부가 기능을 한 곳에서 적용합니다. 서비스 객체의 코드를 수정하지 않고 부가 동작을 더한다는 점에서 프록시 패턴의 구조를 그대로 따릅니다.

## 스프링 AOP의 프록시 생성 방식

스프링 AOP는 프록시 생성을 위해 JDK Dynamic Proxy와 CGLIB를 사용합니다.

- JDK Dynamic Proxy: 인터페이스를 구현하는 프록시 객체를 동적으로 생성합니다.
- CGLIB: 실제 클래스를 상속하는 프록시 객체를 동적으로 생성하고, 메서드 오버라이딩을 통해 호출을 가로챕니다.

스프링 프레임워크는 기본적으로 대상 객체가 인터페이스를 하나 이상 구현하면 JDK Dynamic Proxy를 사용하고, 구현하는 인터페이스가 없으면 CGLIB를 사용합니다. `proxy-target-class`를 `true`로 설정하면 인터페이스 구현 여부와 관계없이 CGLIB로 클래스 기반 프록시를 생성합니다.

반면 스프링 부트의 자동 구성은 기본적으로 CGLIB 프록시를 사용하도록 AOP를 설정합니다. 다음 설정으로 JDK Dynamic Proxy를 사용하도록 변경할 수 있습니다. 다만 인터페이스를 구현하지 않은 빈은 설정과 관계없이 CGLIB로 프록시가 생성됩니다.

{% include code-header.html %}
```yaml
spring:
  aop:
    proxy-target-class: false
```

### CGLIB 주의점

CGLIB는 상속을 기반으로 프록시를 생성하기 때문에 다음과 같은 제약이 존재합니다.

- `final` 클래스는 상속할 수 없으므로 CGLIB 프록시를 생성할 수 없습니다.
- `final` 메서드는 오버라이딩할 수 없으므로 AOP를 적용할 수 없습니다.
- `private` 메서드 역시 오버라이딩할 수 없으므로 AOP를 적용할 수 없습니다.
- 다른 패키지에 있는 부모 클래스의 package-private 메서드는 보이지 않아 사실상 `private`과 같으므로 AOP를 적용할 수 없습니다.

## @Transactional

스프링의 `@Transactional`은 프록시 패턴을 활용하는 대표적인 사례입니다.

{% include code-header.html %}
```java
@Service
public class Example {

    @Transactional
    public void method() {
    }
}
```

스프링은 트랜잭션 적용 대상 객체에 프록시를 생성하고, 프록시를 통해 메서드 호출을 가로채 트랜잭션을 관리합니다.

`@Transactional`은 주로 `public` 메서드에 사용합니다. 스프링 6.0부터는 클래스 기반(CGLIB) 프록시에서 `protected`나 package-visible 메서드에도 기본적으로 적용됩니다. 반면 인터페이스 기반 프록시는 인터페이스에 정의된 `public` 메서드만 대상이 되고, `private` 메서드는 오버라이딩할 수 없으므로 적용되지 않습니다.

스프링 빈의 클래스를 출력해 보면 원본 클래스가 아니라 생성된 프록시 클래스가 나오는 것을 확인할 수 있습니다.

{% include code-header.html %}
```java
System.out.println(example.getClass());
// 스프링 6.0 이상: class hello.Example$$SpringCGLIB$$0
// 스프링 5.x: class hello.Example$$EnhancerBySpringCGLIB$$3a1b2c4d (해시 값은 예시)
```

### 동작 과정

{% include code-header.html %}
```text
   요청
    ↓
Example 프록시
    ↓
TransactionInterceptor
    │
    ├─ 트랜잭션 시작 또는 기존 트랜잭션 참여
    │
    ↓
실제 Example.method()
    │
    ↓
TransactionInterceptor
    │
    ├─ 정상 종료 → Commit
    └─ 예외 발생 → 롤백 규칙에 따라 Commit 또는 Rollback
```

롤백 규칙의 기본값은 `RuntimeException`과 `Error`가 발생하면 롤백하고, 체크 예외(`Exception`)는 롤백하지 않고 커밋하는 것입니다.

### 프록시 기반 AOP의 제약

AOP는 프록시를 경유하는 메서드 호출만 가로챌 수 있습니다. 따라서 실제 객체 내부에서 자신의 다른 메서드를 호출하는 경우(Self-invocation)에는 AOP가 적용되지 않습니다.

{% include code-header.html %}
```java
@Service
public class Example {

    public void method1() {
        method2();
    }

    @Transactional
    public void method2() {
        // DB 저장
    }
}
```

외부에서 `method1()`을 호출하면 다음과 같이 실행됩니다.

{% include code-header.html %}
```text
   요청
    ↓
Example 프록시
    ↓
실제 Example.method1()
    ↓
this.method2()
    ↓
프록시를 거치지 않으므로 @Transactional 미적용
```

`method1()`은 실제 객체 내부에서 직접 호출되므로 `TransactionInterceptor`를 거치지 않습니다.

스프링 공식 문서는 다음 해결 방법을 제시합니다.

- 자기 호출이 일어나지 않도록 코드를 리팩터링합니다. 공식 문서가 가장 덜 침습적인 방법으로 꼽습니다. 예를 들어 `method2()`를 별도 빈으로 분리해 외부 호출로 바꿉니다.
- 자기 자신을 주입받아(self injection) `this` 대신 주입받은 프록시로 호출합니다.
- `AopContext.currentProxy()`로 현재 프록시를 꺼내 호출합니다. 코드가 스프링 AOP에 종속되고 프록시를 노출하도록 설정해야 하므로 공식 문서는 최후의 수단으로만 권합니다.
- AspectJ 모드(컴파일 타임·로드 타임 위빙)를 사용합니다. 프록시 없이 바이트코드에 부가 기능을 적용하므로 자기 호출 문제가 없습니다.

---

지금까지 살펴본 내용을 기준으로 프록시를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황 | 판단 | 근거 |
| --- | --- | --- |
| 서비스 클래스를 수정할 수 없는데 캐싱, 로깅 같은 부가 동작이 필요하다 | 쓴다 | 같은 인터페이스를 구현한 프록시에 넣으면 서비스 객체와 클라이언트를 바꾸지 않고 도입할 수 있다 |
| 무거운 객체를 간헐적으로 사용한다 | 쓴다 | 가상 프록시가 실제로 필요한 시점까지 초기화를 미룬다 |
| 클라이언트의 자격 증명에 따라 접근을 막아야 한다 | 쓴다 | 프록시가 서비스 객체 호출 전에 접근 가능 여부를 판단한다 |
| 원격 서버의 객체를 로컬 객체처럼 써야 한다 | 쓴다 | 프록시가 네트워크 통신의 복잡성을 은닉한다 |
| 트랜잭션, 로깅처럼 같은 부가 기능을 여러 클래스에 적용해야 한다 | 쓴다(스프링 AOP) | 프록시가 호출을 가로채 부가 기능을 한 곳에서 적용하므로 대상 클래스의 코드를 수정하거나 중복 작성하지 않는다. 다만 프록시를 거치는 외부 호출에만 적용된다 |
| 서비스 객체의 인터페이스와 클라이언트가 기대하는 인터페이스가 다르다 | 어댑터를 검토한다 | 프록시는 서비스 객체와 같은 인터페이스를 유지해야 한다 |
| 클라이언트가 기능을 골라 여러 개 겹쳐 쓰게 해야 한다 | 데코레이터를 검토한다 | 프록시는 접근 제어가 목적이고 조합은 클라이언트가 정하는 구조가 아니다 |
| 서비스 객체를 직접 수정할 수 있고 부가 동작이 단순하다 | 피한다 | 클래스가 늘고 호출 한 단계가 추가되어 복잡성과 지연만 늘어난다 |

정리하면, 프록시는 서비스 객체를 수정하지 않고 접근 제어, 캐싱, 로깅 같은 부가 동작을 더할 때 쓰는 패턴입니다. 스프링에서는 AOP와 `@Transactional`이 이 구조로 동작하므로, 프록시를 거치지 않는 자기 호출에는 적용되지 않는다는 점을 기억해야 합니다.

# 참고 자료

[**프록시 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/proxy)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)

[**Spring Framework Reference - Proxying Mechanisms**](https://docs.spring.io/spring-framework/reference/core/aop/proxying.html)

[**Spring Framework Reference - Using @Transactional**](https://docs.spring.io/spring-framework/reference/data-access/transaction/declarative/annotations.html)

[**TransactionInterceptor.java**](https://github.com/spring-projects/spring-framework/blob/main/spring-tx/src/main/java/org/springframework/transaction/interceptor/TransactionInterceptor.java)

[**TransactionAspectSupport.java**](https://github.com/spring-projects/spring-framework/blob/main/spring-tx/src/main/java/org/springframework/transaction/interceptor/TransactionAspectSupport.java)

[**CglibAopProxy.java**](https://github.com/spring-projects/spring-framework/blob/main/spring-aop/src/main/java/org/springframework/aop/framework/CglibAopProxy.java)
