---
title: Spring Security 아키텍처 이해하기

categories:
  - Spring
	- Spring Security

toc: true
toc_sticky: true
published: true

date: 2026-01-06
last_modified_at: 2025-01-06
---

스프링을 다룬다면 한 번 쯤 스프링 시큐리티를 사용해봤을 것이다. 나의 경우 인증과 인가를 처리하는 목적으로 스프링 시큐리티를 다뤄봤지만 내부적으로 어떤 과정을 거치는지는 잘 몰랐다. 따라서 문제가 발생했을 때 처리하는데 어려움을 겪었고, 이 기억은 내가 스프링 시큐리티를 선택하는 데 있어서 주저하게 만드는 이유가 되었다.

그러던 중, 스프링 시큐리티를 필요 이상으로 어렵게 받아들이고 있는 것은 아닐까 하는 생각이 들었다. 이에 공식 문서의 아키텍처를 중심으로 스프링 시큐리티가 어떻게 만들어져 있고, 인증과 인가가 어떤 흐름으로 처리되는지 정리해봤다.

# 아키텍처 설계

## Servlet Filter

스프링 시큐리티의 핵심 아키텍처는 서블릿 컨테이너 레벨에서 동작하는 `Filter`를 기반으로 한다. `Filter`는 서블릿의 최전방에서 요청을 인터셉트하여 적절한 처리를 하는 컴포넌트로, 스프링 시큐리티에 포함된 수많은 보안 기능은 이 `Filter` 체인을 통해 구현된다.

![image.png](/assets/images/spring-security-architecture-overview_01.png)

클라이언트가 요청을 보내면 서블릿 컨테이너는 URI를 기반으로 요청을 처리하는 필터 인스턴스와 서블릿(스프링에서는 `DispatcherServlet`)을 포함하는 `FilterChain`을 구성한다. 그림에서 알 수 있듯이 서블릿은 하나의 서블릿이 하나의 요청만 처리할 수 있지만, 필터는 여러 개로 이루어져 다음과 같은 목적으로 사용된다.

- 하위 필터 인스턴스 또는 서블릿의 호출 차단
- 하위 필터 인스턴스와 서블릿에서 사용하는 `HttpServletRequest` 또는 `HttpServletResponse` 수정

```java
public interface Filter {
	// 서블릿 컨테이너가 필터를 초기화 하는 과정에서 호출
  default void init(FilterConfig filterConfig) throws ServletException {}
	// 필터의 실질적인 작업을 수행하는 메소드
	// 요청, 응답, FilterChain 객체에 접근이 가능
	// 작업 완료 후 다음 필터 호출
  void doFilter(
		  ServletRequest request,
		  ServletResponse response,
		  FilterChain chain
  ) throws IOException, ServletException;
  // 서블릿 컨테이너가 필터를 제거할 때 호출
  default void destroy() {}
}

class ExampleFilter implements Filter {
	// ...
	@Override
	public void doFilter(
			ServletRequest request,
			ServletResponse response,
			FilterChain chain
	) throws IOException, ServletException {
		// 요청 전 처리
		chain.doFilter(request, response, chain); // 다음 필터로 요청을 넘기는 부분
		// 응답 이후 처리
	}
}
```

## DelegatingFilterProxy

스프링은 이런 서블릿 컨테이너에 `DelegatingFilterProxy`를 등록하여 서블릿 컨테이너의 생명주기와 스프링의 `ApplicationContext`를 연결한다. 실제 구현 코드를 간단히 살펴봤을 때, 해당 프록시 객체는 `GenericFilterBean`라는 추상 클래스를 상속하며, 이는 `Filter`를 구현하는 클래스인 것을 확인할 수 있었다.

```java
public abstract class GenericFilterBean implements Filter, ... { }
```

![image.png](/assets/images/spring-security-architecture-overview_02.png)

이렇게 서블릿 컨테이너로 등록하면 해당 객체를 통해 `ApplicationContext`에 등록된 스프링 시큐리티의 필터 빈들이 구동할 수 있게 되는 것이다. `DelegatingFilterProxy.doFilter()`의 의사 코드를 살펴보자.

```java
public void doFilter(
		ServletRequest request,
		ServletResponse response,
		FilterChain chain
) {
	// 스프링 빈으로 등록된 필터를 지연 로딩
	Filter delegate = getFilterBean(someBeanName);
	// 해당 빈 필터에 처리를 위임
	delegate.doFilter(request, response, chain);
}
```

왜 이렇게 만들었을까? 그 이유는 서블릿 컨테이너는 애플리케이션 초기화 과정에서 자체 메커니즘으로 `FilterChain`을 구성하며, 이때 스프링의 `ApplicationContext`가 관리하는 빈은 컨테이너가 직접 인식·생성할 수 없다.

따라서 `DelegatingFilterProxy`를 서블릿 컨테이너의 필터로 등록하고, 해당 객체를 통해서 스프링 빈으로 등록한 필터를 서블릿 필터로써 활용할 수 있는 것이다. 이것이 곧 서블릿 컨테이너의 생명주기와 `ApplicationContext`를 연결한다는 의미이다.

## FilterChainProxy

요청에 따라서 적절한 `SecurityFilterChain`을 선택하여 실행하는 스프링 시큐리티의 특수한 `Filter` 구현체이다. 일반적으로 `DelegatingFilterProxy`가 요청을 위임하는 대상이 된다.

![image.png](/assets/images/spring-security-architecture-overview_03.png)

여기서 이미 `DelegatingFilterProxy`를 통해서 스프링 빈으로 등록된 필터를 호출할 수 있는데도, 왜 굳이 `FilterChainProxy`를 활용하는지 의문이 들었다. 그 이유에 대해서 찾아봤을 때 `DelegatingFilterProxy`는 스프링 시큐리티에서 제공하는 필터 프록시 객체가 아닌 스프링 웹에서 제공해주는 객체인 것을 알 수 있었다.

즉, 해당 객체의 책임은 스프링 시큐리티의 필터 뿐만 아니라 스프링 빈으로 등록되어 서블릿 컨테이너가 알 수 없는 필터를 서블릿 컨테이너 단계에서 활용하기 위한 역할을 가지고 있는 것이다. 이를 잘 활용한 것이 스프링 시큐리티일 뿐이었던 것이다.

이는 패키지에서도 엿볼 수 있다. `DelegatingFilterProxy`는 `org.springframework.web.filter` 패키지에 속한 객체이다. 하지만 `FilterChainProxy`는 `org.springframework.security.web` 패키지에 속한 것을 확인할 수 있었다.

정리하자면 `DelegatingFilterProxy`는 서블릿 컨테이너와 스프링 빈을 연결하기 위한 스프링 웹의 범용 필터 프록시이고, `FilterChainProxy`는 그 위에서 스프링 시큐리티가 반드시 수행해야 하는 공통 전처리와 보안 필터 체인 선택 및 실행을 책임지는 전용 진입점이 되는 것이다.

## SecurityFilterChain

`SecurityFilterChain`은 `FilterChainProxy`에서 현재 요청에 대해 어떤 스프링 시큐리티 필터 인스턴스를 호출해야 하는지 결정하는데 사용된다.

![image.png](/assets/images/spring-security-architecture-overview_04.png)

앞서서 이미 `DelegatingFilterProxy`에 직접 등록하는 대신에 `FilterChainProxy`를 활용하는 이유를 다루었었다. 이러한 구조로 인해 `FilterChainProxy`를 활용하면 다음과 같은 장점이 있다.

- 모든 스프링 시큐리티에 관한 시작점이 되므로 `FilterChainProxy`에 디버깅 포인트를 추가하여 시큐리티 관련 디버깅을 쉽게 할 수 있다.
- 스프링 시큐리티 사용의 핵심이기 때문에 사전 필수 작업들(메모리 누수 방지를 위한 `SecurityContext` 초기화, `HttpFirewall` 적용 등)을 수행할 수 있다.
- `SecurityFilterChain`을 호출해야 하는 시점을 결정하는데 더 많은 유연성을 제공한다. 서블릿 컨테이너에서는 필터 인스턴스가 URL만을 기반으로 호출되지만, `FilterChainProxy`는 `RequestMatcher` 인터페이스를 통해 `HttpServletRequest`의 모든 내용을 기반으로 호출 여부를 결정할 수 있다.

이러한 이유로 스프링 시큐리티는 `DelegatingFilterProxy` 위에 `FilterChainProxy`를 두어 보안 처리의 진입점으로 활용하는 것이다.

그리고 위 그림은 `HttpServletRequest`를 기반으로 여러 `SecurityFilterChain` 중에서 어떤 것이 사용되어야 하는지를 `FilterChainProxy`가 결정하는 과정이다. `/api/`로 시작하는 어떤 URL은 0번 필터 체인이 호출되고, 그 외 요청들은 n번 필터 체인이 호출되는 식이다. 이런 식으로 각 요청에 따라서 고유한 보안 필터 인스턴스를 구성할 수 있다. 그림에서도 보안 필터의 개수가 다른 것을 볼 수 있다.

## Security Filters

`SecurityFilterChain`과 함께 `FilterChainProxy`에 삽입되는 개별적인 필터들로, 보안 관련 설정이나 인증 및 인가 등을 수행하는 핵심적인 역할을 하게 된다. 필터는 특정한 순서대로 실행되어 적절한 시점에 호출되도록 보장한다.

예를 들어 권한 부여 이전에 인증을 수행하는 필터가 먼저 호출되어야 하는 식이다. 모든 필터의 실행 순서를 알 필요는 없지만 알면 좋은 경우도 있다. 따라서 필요하다면 `FilterOrderRegistration`을 참고하면 된다. 이에 따라서 내부 코드를 참고해보니 간단하게 `Map`으로 필수적인 필터들을 등록한 것으로 보인다.

```java
final class FilterOrderRegistration {

	private static final int INITIAL_ORDER = 100;
	private static final int ORDER_STEP = 100;
	private final Map<String, Integer> filterToOrder = new HashMap<>();
	// ...
	FilterOrderRegistration() {
		Step order = new Step(INITIAL_ORDER, ORDER_STEP);
		put(DisableEncodeUrlFilter.class, order.next());
		put(ForceEagerSessionCreationFilter.class, order.next());
		// ...
	}

	void put(Class<? extends Filter> filter, int position) {
		this.filterToOrder.putIfAbsent(filter.getName(), position);
	}

	// ...
}
```

그리고 바로 이 부분이 개발자가 정의하는 부분이 된다. 정확히는 일반적인 서블릿 애플리케이션이라고 가정했을 때, `HttpSecurity` DSL을 활용하여 `SecurityFilterChain`을 정의하는 빈이 바로 인증 및 인가에 필요한 필터들을 정의하고, 최종적으로 하나의 `SecurityFilterChain`으로 구성되어 `FilterChainProxy`가 이를 참조하여 요청을 처리하게 된다.

```java
@Configuration
@EnableWebSecurity
public class SecurityConfig {

  @Bean
  public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
      return http
              .csrf(Customizer.withDefaults())
              .httpBasic(Customizer.withDefaults())
              .formLogin(Customizer.withDefaults())
              .authorizeHttpRequests((authorize) -> authorize
                    .anyRequest().authenticated()
              )
              .build();
  }
}
```

### 커스텀 필터 추가

때때로 커스텀 필터를 만들어야 할 때가 있다. 대표적으로 REST API에서 인증 / 인가를 위해 JWT 파싱 작업을 수행할 때 이 작업이 필요하다. 스프링 시큐리티는 이를 위해서 세 개의 메서드를 제공한다.

- `HttpSecurity.addFilterBefore(Filter, Class<?>)`
- `HttpSecurity.addFilterAfter(Filter, Class<?>)`
- `HttpSecurity.addFilterAt(Filter, Class<?>)`

사용 방법은 모두 필터 인스턴스와 기준이 되는 클래스를 넘기면 메서드에 따라서 해당 필터 클래스의 이전과 이후, 그리고 중복되어 실행되게 만들 수 있다. 이를 활용하여 다음과 같이 커스텀 필터를 정의하고 등록하면 된다.

```java
public class TenantFilter implements Filter {

  @Override
  public void doFilter(
	    ServletRequest servletRequest,
	    ServletResponse servletResponse,
	    FilterChain filterChain
	) throws IOException, ServletException {
    // ...
  }
}

@Bean
SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
	http
	    // ...
	    .addFilterAfter(
	        new TenantFilter(),
	        AnonymousAuthenticationFilter.class
	    );
	return http.build();
}
```

# 인증 (Authentication)

## SecurityContextHolder

스프링 시큐리티 인증 아키텍처의 핵심이 되는 객체로, 이름 그대로 `SecurityContext`를 보관하는 객체이다. 스프링 시큐리티는 여기에 어떤 방식으로 채워지는지 신경쓰지 않고, 값이 있으면 인증된 사용자로 간주되어 해당 값이 사용된다.

그리고 기본적으로 `SecurityContextHolder`는 `ThreadLocal`을 사용한다. 따라서 동일한 스레드 내의 메서드에서 항상 `SecurityContext`를 사용할 수 있다. 그리고 이렇게 구성되어 있기 때문에 `FilterChainProxy`에서 항상 `SecurityContextHolder`가 비워지도록 전처리 작업을 수행하는 이유이기도 하다. (하나의 스레드가 여러 사용자의 요청을 처리하기 때문)

![image.png](/assets/images/spring-security-architecture-overview_05.png)

여기서 인증된 사용자 정보는 다음과 같이 조회할 수 있다.

```java
SecurityContext context = SecurityContextHolder.getContext();
Authentication authentication = context.getAuthentication();
String username = authentication.getName();
Object principal = authentication.getPrincipal();
Collection<? extends GrantedAuthority> authorities = authentication.getAuthorities();
```

## Authentication

`Authentication`는 크게 두 가지 용도로 사용된다.

- `AuthenticationManager`에 사용자가 인증을 위해 제공한 자격 증명을 전달하는 입력값이다. 이 경우 `isAuthenticated()`는 `false`를 반환한다.
- 현재 인증된 사용자를 나타낸다. 인증된 사용자 정보는 `SecurityContext`에서 조회할 수 있다.

따라서 `Authentication`은 세 개의 대표적인 사용자 정보를 가지고 있다.

- `Principal`: 사용자 식별, 사용자 이름과 암호로 인증할 경우, 이는 대개 `UserDetails` 인스턴스이다.
- `Credentials`: 사용자가 입력한 암호로, 사용자 인증 후 대부분 삭제된다.
- `Authorities`: 사용자에게 부여된 권한으로, 역할과 범위가 그 예이다.

## **GrantedAuthority**

사용자에게 부여된 상위 수준의 권한이다. 앞서 말한대로 역할과 스코프가 있으며, 이 권한들은 `Authentication.getAuthorities()`를 통해 조회할 수 있으며, 내부적으로는 문자열 기반 권한 식별자이다.

일반적으로 GrantedAuthority는 `ROLE_ADMIN`과 같은 역할(Role) 형태로 사용되며, 이후 웹 보안, 메서드 보안, 도메인 객체 보안 설정에서 인가 판단의 기준으로 활용된다.

Username/Password 기반 인증에서는 보통 `UserDetailsService`가 이러한 권한을 로딩한다.

GrantedAuthority는 애플리케이션 전역 권한을 표현하는 용도로 설계되었으며, 특정 도메인 객체(예: 특정 ID의 엔티티)에 대한 개별 권한을 나타내는 용도로는 적합하지 않다.

도메인 객체 단위의 세밀한 권한 제어는 스프링 시큐리티의 도메인 객체 보안 기능을 사용해야 한다.

## **AuthenticationManager**

스프링 시큐리티가 인증을 수행하는 방식을 정의하는 API이다. 이를 통해 반환된 인증 정보는 인증 필터에 의해`SecurityContextHolder`에 설정된다.

## **ProviderManager**

`AuthenticationManager`의 가장 일반적인 구현체이다. 이는 `AuthenticationProvider` 인스턴스 목록 중에서 해당하는 인증 방법으로 인증을 위임한다. 이 인스턴스 중에서 어느것도 인증에 성공하지 못하면 `ProviderNotFoundException` 예외와 함께 인증이 실패한다.

![image.png](/assets/images/spring-security-architecture-overview_06.png)

실제로 각 `AuthenticationProvider`는 처리 가능한 인증 타입이 무엇인지를 스스로 알고 있다. 이는 인터페이스 설계를 살펴보면 명확하게 알 수 있다.

```java
public interface AuthenticationProvider {
	@Nullable Authentication authenticate(Authentication authentication) throws AuthenticationException;
	boolean supports(Class<?> authentication);
}
```

그리고 이를 구현한 `AbstractUserDetailsAuthenticationProvider`를 살펴보면 다음과 같이 메서드가 오버라이딩 되어 있다.

```java
@Override
public boolean supports(Class<?> authentication) {
	return (UsernamePasswordAuthenticationToken.class.isAssignableFrom(authentication));
}
```

즉 해당되는 인증 필터에 따라서 적절한 `AuthenticationProvider`가 동작하도록 만들어져 있다. `UsernamePasswordAuthenticationFilter.attemptAuthentication()`을 살펴보자.

```java
@Override
public Authentication attemptAuthentication(
		HttpServletRequest request,
		HttpServletResponse response
) throws AuthenticationException {
	// ...
	UsernamePasswordAuthenticationToken authRequest
			= UsernamePasswordAuthenticationToken.unauthenticated(username, password);
	setDetails(request, authRequest);
	return this.getAuthenticationManager().authenticate(authRequest);
}
```

마지막에 `ProviderManager.authenticate()`가 호출되는 것을 볼 수 있다. 단계적으로 보자면

1. 인증 요청에 관한 필터가 동작하여 `ProviderManager`를 호출
2. `ProviderManager`는 `AuthenticationProvider`가 처리할 수 있는 인증 유형인지 검사
3. 있다면 해당 `AuthenticationProvider`가 인증을 수행, 없다면 `ProviderNotFoundException` 발생

이런 흐름으로 인증이 수행되는 것이다. 그리고 `ProviderManager`는 선택적인 상위 `AuthenticationManager`를 구성할 수 있도록 지원한다. 이는 곧 여러 인증 방식을 계층적으로 조합할 수 있도록 만들어 처리하지 못한 인증을 상위 `AuthenticationManager`에게 위임할 수 있다는 것이다.

![image.png](/assets/images/spring-security-architecture-overview_07.png)

이는 여러 `SecurityFilterChain` 인스턴스가 공통된 인증 방식(상위 `AuthenticationManager`)을 사용하지만, 서로 다른 메커니즘(여러 `ProviderManager` 인스턴스)을 사용하는 시나리오에서 흔히 발생한다.

![image.png](/assets/images/spring-security-architecture-overview_08.png)

이 부분은 나중에 직접 해보면서 좀 더 정리해야 할 것 같다. 마지막으로 `ProviderManager`는 인증 요청이 성공적으로 완료된 후 반환되는 `Authentication` 객체에서 민감한 자격 증명 정보를 모두 삭제하려고 한다.

## AuthenticationProvider

인증을 수행하는 객체이며, `ProviderManager`에 여러 `AuthenticationProvider` 인스턴스를 주입할 수 있다.

## AuthenticationEntryPoint

`AuthenticationEntryPoint`는 인증되지 않은 요청이 보호된 자원에 접근했을 때, 클라이언트에게 어떻게 인증을 요구할지를 결정하는 컴포넌트다.

## AbstractAuthenticationProcessingFilter

사용자 자격 증명을 인증하기 위한 기본 필터로 사용된다. 인증되지 않은 사용자가 보호된 리소스에 접근했을 때 `AuthenticationEntryPoint`가 먼저 동작해 자격 증명을 요청하고, `AbstractAuthenticationProcessingFilter`는 이 요청을 가로챈다. 이 과정을 묘사하면 다음과 같다.

```
[Authorization / Security Filter]
        ↓ (AccessDeniedException / AuthenticationException)
[ExceptionTranslationFilter]
        ↓
[AuthenticationEntryPoint]  ← redirect / 401 응답
        ↓
(클라이언트가 로그인 요청 전송)
        ↓
[AbstractAuthenticationProcessingFilter]
```

참고로 이 추상 클래스를 상속한 대표적인 필터가 바로 `UsernamePasswordAuthenticationFilter`이다.

![image.png](/assets/images/spring-security-architecture-overview_09.png)

이후 다음 절차를 수행한다. 코드와 함께 보기 위해서 `UsernamePasswordAuthenticationFilter`부터 시작하는 코드를 따라가면서 정리했다.

1. Authentication 생성

`HttpServletRequest`로부터 `Authentication` 객체를 생성한다. 이때 생성되는 `Authentication` 타입은 하위 필터에 의해 결정된다.

```java
public class UsernamePasswordAuthenticationFilter
		extends AbstractAuthenticationProcessingFilter {
	// ...
	@Override
	public Authentication attemptAuthentication(
			HttpServletRequest request,
			HttpServletResponse response
	) throws AuthenticationException {
		// ...
		// Authentication의 하위 구현체
		UsernamePasswordAuthenticationToken authRequest
				= UsernamePasswordAuthenticationToken.unauthenticated(username, password);
		// 웹 요청의 부가 정보를 넣는 메서드
		// 기본적으로 IP 주소와 세션 ID가 여기서 추가됨
		// 서비스에 따라 비정상 세션 감지, IP 차단 가능
		setDetails(request, authRequest);
	}
	// ...
}
```

2. AuthenticationManager 위임

생성된 `Authentication`을 `AuthenticationManager`에 전달해 실제 인증을 수행한다.

```java
public class UsernamePasswordAuthenticationFilter
		extends AbstractAuthenticationProcessingFilter {
	// ...
	@Override
	public Authentication attemptAuthentication(
    		HttpServletRequest request,
    		HttpServletResponse response
	) throws AuthenticationException {
		// ...
		return this.getAuthenticationManager().authenticate(authRequest);
	}
	// ...
}
```

3. 인증 성공 시

- `SessionAuthenticationStrategy` 실행
- 기존 인증 정보가 있다면 권한 병합
- 인증된 `Authentication`을 `SecurityContextHolder`에 저장
- 필요 시 `SecurityContextRepository`를 통해 컨텍스트 저장
- `RememberMeServices.loginSuccess()` 호출
- 인증 성공 이벤트 발행
- `AuthenticationSuccessHandler` 실행

```java
public abstract class AbstractAuthenticationProcessingFilter
		extends GenericFilterBean
		implements ApplicationEventPublisherAware, MessageSourceAware {
	// ...
	protected void successfulAuthentication(
			HttpServletRequest request,
			HttpServletResponse response,
			FilterChain chain,
			Authentication authResult
	) throws IOException, ServletException {
		SecurityContext context = this.securityContextHolderStrategy.createEmptyContext();
		context.setAuthentication(authResult);
		this.securityContextHolderStrategy.setContext(context);
		this.securityContextRepository.saveContext(context, request, response);
		// 로깅
		this.rememberMeServices.loginSuccess(request, response, authResult);
		if (this.eventPublisher != null) {
			this.eventPublisher
					.publishEvent(new InteractiveAuthenticationSuccessEvent(
									authResult,
									this.getClass())
					);
		}
		this.successHandler.onAuthenticationSuccess(request, response, authResult);
	}
	// ...
}
```

4. 인증 실패 시

- `SecurityContextHolder` 초기화
- `RememberMeServices.loginFail()` 호출
- `AuthenticationFailureHandler` 실행

```java
public abstract class AbstractAuthenticationProcessingFilter
		extends GenericFilterBean
		implements ApplicationEventPublisherAware, MessageSourceAware {
	// ...
	protected void unsuccessfulAuthentication(
			HttpServletRequest request,
			HttpServletResponse response,
			AuthenticationException failed
	) throws IOException, ServletException {
		this.securityContextHolderStrategy.clearContext();
		// 로깅
		this.rememberMeServices.loginFail(request, response);
		this.failureHandler.onAuthenticationFailure(request, response, failed);
	}
	// ...
}
```

# 인가 (Authorization)

인가(Authorization)는 이미 인증된 사용자가 특정 요청을 수행할 수 있는지 판단하는 과정이다. 참고로 스프링 시큐리티 6부터 인가 구조는 기존의 `AccessDecisionManager` / `AccessDecisionVoter` 기반 모델에서 `AuthorizationManager` 중심 구조로 재설계되었다. 이 변경의 목적은 인가 로직을 더 명확하고, 조합 가능하며, 테스트하기 쉬운 구조로 만들기 위함이다.

때문에 포스팅에서는 `AuthorizationManager` 부분만 자세히 다루도록 하겠다. 그리고 스프링 시큐리티는 웹 요청에 대한 인가 뿐 아니라, `@PreAuthorize`와 `@PostAuthorize` 어노테이션으로 애플리케이션의 메서드 자체에 인가 처리를 해줄 수도 있다. 하지만 Spring MVC 애플리케이션이나 REST API 개발을 주로 해왔기 때문에 이 포스팅에서는 서블릿 애플리케이션에서의 요청에 대한 인가 처리만 다룬다.

## GrantedAuthority

스프링 시큐리티에서 모든 `Authentication` 구현체는 `GrantedAuthority`를 담는 컬렉션 객체를 가지고 있다. 이 객체는 `AuthenticationManager`에 의해 `Authentication` 객체에 삽입되고, 이후 `AuthorizationManager` 인스턴스가 권한 부여 결정을 내릴 때 읽어들인다. 해당 객체의 인터페이스를 한 번 살펴보자.

```java
public interface GrantedAuthority extends Serializable {
	@Nullable String getAuthority();
}
```

문자열을 반환하는 하나의 메서드만 존재하는 것을 볼 수 있다. 이 메서드는 `AuthorizationManager` 구현체에 의해 호출되어, 인가 판단을 위한 권한 정보를 읽는데 사용된다. 권한 정보를 `String`을 반환함으로써 대부분의 `AuthorizationManager` 구현체는 단순한 문자열 비교를 통해 권한을 처리할 수 있다.

대부분 문자열 기반 권한을 사용하며, 복잡한 권한 모델은 커스텀 `AuthorizationManager`를 통해 별도로 해석하는 방식이 일반적이다.

스프링 시큐리티는 `SimpleGrantedAuthority`라는 하나의 `GrantedAuthority` 구현체를 제공한다. 이 구현체는 사용자가 지정한 문자열을 `GrantedAuthority`로 변환할 수 있다. 보안 아키텍처에 포함된 모든 `AuthenticationProvider` 인스턴스는 `SimpleGrantedAuthority`를 사용하여 `Authentication` 객체를 설정한다.

그리고 기본적으로 역할 기반 부여 규칙에는 ROLE\_ 접두사가 포함된다. 따라서 `SecurityFilterChain`을 구성할 때 USER 역할에 인가를 설정하는 경우, `SimpleGrantedAuthority`의 권한은 ROLE_USER라는 값으로 설정되어야 한다.

이러한 권한에 사용할 접두사는 `GrantedAuthorityDefaults`로 다음과 같이 커스텀 할 수 있다.

```java
@Bean
static GrantedAuthorityDefaults grantedAuthorityDefaults() {
	return new GrantedAuthorityDefaults("MYPREFIX_");
}
```

여기서 `@Configuration` 클래스를 초기화하기 전에 스프링이 `GrantedAuthorityDefaults`를 등록하도록 보장하려면 정적 메서드를 사용해야 한다.

## AuthorizationManager

인가를 처리하는 핵심 주체이다. 다음과 같은 인터페이스 설계를 가지고 있다.

```java
@FunctionalInterface
public interface AuthorizationManager<T extends @Nullable Object> {
	// 인가 판단 결과를 바탕으로 흐름 제어
	default void verify(Supplier<? extends @Nullable Authentication> authentication, T object) {
		AuthorizationResult result = authorize(authentication, object);
		if (result != null && !result.isGranted()) {
			throw new AuthorizationDeniedException("Access Denied", result);
		}
	}
	// 인가 판단 수행
	@Nullable AuthorizationResult authorize(Supplier<? extends @Nullable Authentication> authentication, T object);
}
```

그리고 스프링 시큐리티는 요청에 따라서 적절한 `AuthorizationManager` 구현체에 판단을 위임하는 구조를 가지고 있다.

![image.png](/assets/images/spring-security-architecture-overview_10.png)

웹 요청의 경우 `RequestMatcherDelegatingAuthorizationManager`가 `RequestMatcher`를 기반으로 요청을 분기하여 인가를 담당한다.

## 웹 요청 인가 과정 살펴보기

![image.png](/assets/images/spring-security-architecture-overview_11.png)

1. `AuthorizationFilter`는 `SecurityContextHolder`로부터 `Authentication`을 가져오는 `Supplier`를 생성하여 `Supplier<Authentication>`과 `HttpServletRequest`를 `AuthorizationManager`에게 전달한다.

```java
public class AuthorizationFilter extends GenericFilterBean {
	// ...
	@Override
	public void doFilter(
			ServletRequest servletRequest,
			ServletResponse servletResponse,
			FilterChain chain
	) throws ServletException, IOException {
		// ...
		try {
			AuthorizationResult result = this.authorizationManager
																		.authorize(this::getAuthentication, request);
			// ...
		}
	}

	private Authentication getAuthentication() {
		Authentication authentication = this.securityContextHolderStrategy
																				.getContext().getAuthentication();
		// ...
		return authentication;
	}

	private static class NoopAuthorizationEventPublisher implements AuthorizationEventPublisher {

		@Override
		public <T> void publishAuthorizationEvent(Supplier<Authentication> authentication, T object,
				@Nullable AuthorizationResult result) {
		}
	}
}
```

2. `AuthorizationManager`는 `authorizeHttpRequests`에 정의된 패턴과 요청을 매칭한 후, 해당하는 인가 규칙을 실행한다. `RequestMatcherDelegatingAuthorizationManager.authorize()`를 살펴보자.

```java
private static final AuthorizationDecision DENY
																= new AuthorizationDecision(false);

@Override
public @Nullable AuthorizationResult authorize(
		Supplier<? extends @Nullable Authentication> authentication,
		HttpServletRequest request
) {
	for (RequestMatcherEntry<AuthorizationManager<? super RequestAuthorizationContext>> mapping : this.mappings) {

		RequestMatcher matcher = mapping.getRequestMatcher();
		MatchResult matchResult = matcher.matcher(request);
		// 인가 성공
		if (matchResult.isMatch()) {
			AuthorizationManager<? super RequestAuthorizationContext> manager = mapping.getEntry();

			return manager.authorize(authentication,
					new RequestAuthorizationContext(request, matchResult.getVariables()));
		}
	}

	// 인가 실패
	return DENY;
}
```

3. 접근이 허용되면 `AuthorizationGrantedEvent`(기본적으로 No-op이라 발행되지 않고 설정된 경우에만 발행)가 발행되고, `AuthorizationFilter`는 필터 체인을 계속 진행하여 애플리케이션이 정상적으로 처리되도록 한다.

```java
public class AuthorizationFilter extends GenericFilterBean {
	// ...
	@Override
	public void doFilter(
			ServletRequest servletRequest,
			ServletResponse servletResponse,
			FilterChain chain
	) throws ServletException, IOException {
		// ...
		try {
			// ...
			this.eventPublisher
						.publishAuthorizationEvent(this::getAuthentication, request, result);
			chain.doFilter(request, response);
		}
	}
	// ...
}
```

4. 인가가 거부되면 `AuthorizationDeniedEvent`가 발행되고, `AccessDeniedException`이 발생한다. 이 경우 `ExceptionTranslationFilter`가 `AccessDeniedException`을 처리한다.

```java
public class AuthorizationFilter extends GenericFilterBean {
	// ...
	@Override
	public void doFilter(
			ServletRequest servletRequest,
			ServletResponse servletResponse,
			FilterChain chain
	) throws ServletException, IOException {
		// ...
		try {
			// ...
			if (result != null && !result.isGranted()) {
				throw new AuthorizationDeniedException("Access Denied", result);
			}
			// ...
		}
	}
	// ...
}
```

## HttpSecurity DSL을 통한 인가 설정

스프링 시큐리티에서는 인가 역시 인증과 마찬가지로 `HttpSecurity` DSL을 통해서 다음과 같이 간단하게 구현할 수 있다.

```java
@Bean
SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
	return http
	    .authorizeHttpRequests((authorize) -> authorize
	        .anyRequest().authenticated()
	    )
	    .build();
}
```

스프링 시큐리티는 기본적으로 모든 요청이 인증된 상태인 것을 요구하므로 위와 같은 인가 처리를 최소한의 설정으로 권장한다고 한다.

참고로 `AuthorizationFilter`는 기본적으로 스프링 시큐리티의 모든 필터 중에서 가장 마지막에 위치해있다. 따라서 `AuthorizationFilter` 앞 순서에 배치된 필터들은 모두 인가를 요구하지 않아야 한다는 것을 유념해야 한다.

### requestMatchers

`AuthorizationManagerRequestMatcherRegistry`의 메서드로, 인가 규칙이 적용될 엔드포인트(`RequestMatcher`)를 정의할 때 사용한다.

```java
@Bean
public SecurityFilterChain web(HttpSecurity http) throws Exception {
	http
	    .authorizeHttpRequests((authorize) -> authorize
			    .requestMatchers("/endpoint").hasAuthority("USER")
	        .anyRequest().authenticated()
	      )
	    // ...

	return http.build();
}
```

> **hasRole() vs hasAuthority()**
>
> 인가 규칙을 정의할 때 두 메서드가 나와서 혼란스러웠다. 두 메서드 모두 사용자가 어떤 `GrantedAuthority`를 가지고 있는지 검사한다. 하지만 `hasRole()`은 앞에 접두사로 `ROLE_`을 붙여서 검사한다는 차이점이 있다.

또한 `requestMatchers()`는 Ant-style 경로 패턴 매칭 문법을 지원한다. 예를 들어 `/board`라는 주소의 하위 주소는 모두 허용하려면 다음과 같이 설정하면 된다.

```java
@Bean
public SecurityFilterChain web(HttpSecurity http) throws Exception {
	http
	    .authorizeHttpRequests((authorize) -> authorize
			    .requestMatchers("/board/**").permitAll()
	        .anyRequest().authenticated()
	    )
	    // ...

	return http.build();
}
```

### 인가 규칙

위 코드에서 `hasAuthority()`는 해당 엔드포인트에 접근하는 인증 주체가 어떤 자격을 가지고 있는지 정의하는 부분이다. 이처럼 `HttpSecurity` DSL은 여러 인가 규칙을 내장하고 있다.

- `permitAll()`: 인가가 필요 없는 공개 엔드포인트이다. 세션에서 `Authentication`을 조회하지 않는다.
- `denyAll()`: 어떤 경우에도 요청을 허용하지 않는다. 이 경우 역시 `Authentication`은 조회되지 않는다.
- `hasAuthority()`: `Authentication` 객체가 지정된 값과 일치하는 `GrantedAuthority`를 가지고 있어야 한다.
- `hasRole()`: `hasAuthority`의 단축 표현으로, 기본 접두사인 `ROLE_`(또는 설정된 기본 prefix)를 자동으로 붙여서 검사한다.
- `hasAnyAuthority()`: `Authentication`이 주어진 값들 중 하나라도 일치하는 `GrantedAuthority`를 가지고 있으면 허용된다.
- `hasAnyRole()`: `hasAnyAuthority`의 단축 표현으로,각 값에 `ROLE_`(또는 기본 prefix)를 붙여 검사한다.
- `hasAllAuthorities()`: Authentication이 주어진 모든 `GrantedAuthority`를 가지고 있어야 한다.
- `hasAllRoles()`: `hasAllAuthorities`의 단축 표현으로, 각 값에 `ROLE_`(또는 기본 prefix)를 붙여 검사한다.
- `access()`: 커스텀 `AuthorizationManager`를 사용하여 접근 여부를 판단한다.

Any는 조건이 하나라도 만족하면 허용되고, All은 모든 조건이 만족해야 한다는 것을 의미한다.

### 활용 예시

위 두 내용을 종합하면 `authorizeHttpRequests()`는 `AuthorizationManagerRequestMatcherRegistry.requestMatchers()`를 통해 인가 규칙이 적용될 요청 범위를 정의하고, 이후 인가 규칙 메서드를 통해 해당 엔드포인트에 적용할 규칙을 지정하는 것이다. 다음은 이를 활용한 서블릿 기반 애플리케이션에서의 인가 설정 예시다.

```java
@Bean
public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
	return http
			  // ...
	      .authorizeHttpRequests(auth -> auth
	              .requestMatchers(PathRequest.toStaticResources().atCommonLocations())
	              .permitAll() // 정적 리소스는 무조건 통과
	              .requestMatchers("/login", "/register", "/board/**")
	              .permitAll() // 로그인 페이지와 /board의 하위 경로는 누구나 접근 가능
	              .requestMatchers("/admin").hasRole("ADMIN") // ROLE_ADMIN만 접근 가능
	              .anyRequest().authenticated() // 그 외 모든 요청은 인증 필요
	      )
	      .build();
}
```

마지막으로 `anyRequest().authenticated()`는 항상 마지막에 위치해야 한다. 그 이유는 `RequestMatcherDelegatingAuthorizationManager`의 구현 특성상 리스트를 순차적으로 순회하다가 가장 먼저 매칭되는 규칙을 적용하기 때문이다. 이는 첫 번째로 매칭되는 규칙이 적용되는 구조이기 때문에, 더 일반적인 규칙이 앞에 오면 이후 규칙은 평가되지 않는다.

이렇게 설정하면 각 인가 규칙은 내부적으로 `RequestMatcher`와 `AuthorizationManager`의 조합인 `RequestMatcherEntry`로 생성돼 등록되며, 이후 요청 시 `RequestMatcherDelegatingAuthorizationManager`에서 순차적으로 평가된다.

---

이제 스프링 시큐리티가 어떤 구조로 되어있는지 공부했으니 활용할 차례이다. 구현 순서는 애플리케이션 자체 로그인을 MVC 애플리케이션과 REST API + JWT로 구현하는 것이 첫 번째이고, 두 번째로는 OAuth2를 MVC 애플리케이션과 REST API + JWT 차례대로 구현하는 것이다.

# 참고 자료

[**실전 스프링 부트 [솜나트 무시브]**](https://product.kyobobook.co.kr/detail/S000208713876)

[**Spring Security/Servlet Applications/Architecture**](https://docs.spring.io/spring-security/reference/servlet/architecture.html)

[**Spring Security/Servlet Applications/Authentication/Authentication Architecture**](https://docs.spring.io/spring-security/reference/servlet/authentication/architecture.html)

[**Spring Security/Servlet Applications/Authorization/Authorize HTTP Requests**](https://docs.spring.io/spring-security/reference/servlet/authorization/authorize-http-requests.html)
