---
title: Spring Security OAuth2 Client 알아보기

categories:
  - Spring Security

toc: true
toc_sticky: true
published: true

date: 2026-02-13
---

OAuth 2.0이 무엇인지 알아봤다. 이제 스프링 시큐리티 공식 문서를 통해서 OAuth 2.0 로그인 관련한 핵심 컴포넌트를 알아보도록 하자. 우선 스프링 시큐리티는 OAuth 2.0과 관련하여 다음 의존성들을 제공한다.

- OAuth2 Authorization Server
- OAuth2 Resource Server
- OAuth2 Client

이 중 OAuth 2.0 로그인을 구현하기 위해서는 OAuth2 Client 의존성을 사용하면 된다. 이에 대해서 자세히 알아보도록 하자.

# Spring Security OAuth2 Client 핵심 컴포넌트

## ClientRegistration

OAuth 2.0 이나 OpenID Connect 1.0 제공자에 등록된 클라이언트를 나타내는 컴포넌트다. 코드를 통해 해당 컴포넌트가 어떤 정보를 가지고 있는지 주석과 함께 알아보자.

```java
public final class ClientRegistration {

    // 이 ClientRegistration을 내부적으로 식별하기 위한 고유 ID (예: "google", "kakao")
    private String registrationId;
    // OAuth2.0 / OIDC Provider가 발급한 클라이언트 식별자
    private String clientId;
    // 클라이언트 인증에 사용되는 비밀키 (Confidential Client인 경우)
    private String clientSecret;
    // 토큰 엔드포인트 호출 시 클라이언트를 인증하는 방식
    // (client_secret_basic, client_secret_post, private_key_jwt, none 등)
    private ClientAuthenticationMethod clientAuthenticationMethod;
    // OAuth 2.0에서 사용하는 인가 방식
    // (authorization_code, client_credentials, jwt-bearer 등)
    private AuthorizationGrantType authorizationGrantType;
    // 인가 서버가 인증/동의 완료 후 사용자 에이전트를 리다이렉트할 URI
    private String redirectUri;
    // 인가 요청 시 클라이언트가 요청하는 권한 범위 (openid, profile, email 등)
    private Set<String> scopes;
    // OAuth2.0 / OIDC Provider의 엔드포인트 및 메타데이터 정보
    private ProviderDetails providerDetails;
    // 클라이언트를 설명하는 이름 (로그인 페이지 등 UI 표시용)
    private String clientName;

    public class ProviderDetails {
        // Authorization Code Flow에서 사용되는 Authorization Endpoint URI
        private String authorizationUri;
        // Access Token / Refresh Token 발급에 사용되는 Token Endpoint URI
        private String tokenUri;
        // 인증된 사용자 정보를 조회하기 위한 UserInfo Endpoint 설정
        private UserInfoEndpoint userInfoEndpoint;
        // ID Token 또는 UserInfo Response의 서명을 검증하기 위한 JWK Set URI
        private String jwkSetUri;
        // OIDC Provider 또는 Authorization Server의 Issuer 식별자
        private String issuerUri;
        // issuer-uri 기반 discovery를 통해 조회된 Provider 설정 메타데이터
        private Map<String, Object> configurationMetadata;

        public class UserInfoEndpoint {
            // 인증된 사용자의 claims/attributes를 조회하는 UserInfo Endpoint URI
            private String uri;
            // UserInfo Endpoint 호출 시 access token 전달 방식
            // (header, form, query)
            private AuthenticationMethod authenticationMethod;
            // UserInfo Response에서 사용자의 고유 식별자로 사용할 attribute 이름
            // (예: "sub", "id", "email")
            private String userNameAttributeName;
        }
    }

    public static final class ClientSettings {
        // true 이거나 clientAuthenticationMethod가 none이면 PKCE 사용 여부
        private boolean requireProofKey;
    }
}
```

`ClientRegistration`를 구성하기 위해서 다음 편의 메서드를 제공한다.

```java
ClientRegistration clientRegistration =
    ClientRegistrations
		    .fromIssuerLocation("https://idp.example.com/issuer")
		    .build();
```

이는 `issuer-uri`이 있는 경우에 스프링 시큐리티 자동 구성에서 사용하는 메서드로, OIDC 및 OAuth 2.0 규격에 따라서 정의된 엔드포인트 리스트에 대해서 최초로 정상 응답을 반환할 때까지 순차적으로 조회한다. 예를 들어 위 issuer-uri에 대한 엔드포인트 후보 리스트는 다음과 같다.

- idp.example.com/issuer/.well-known/openid-configuration
- https://idp.example.com/.well-known/openid-configuration/issuer
- https://idp.example.com/.well-known/oauth-authorization-server/issuer

이때 4xx 응답은 다음 엔드포인트에 대한 요청으로 진행하고, 그 외 응답은 즉시 예외를 발생시킨다.

조회가 성공하면 `ClientRegistration`의 `ProviderDetails`를 포함한 `authorization-uri,` `token-uri`, `jwk-set-uri`, `issuer-uri` 등 제공자 관련 메타데이터와 OAuth 2.0 로그인에 필요한 기본 설정 값들이 함께 구성된다.

만약 OpenID Connect만 사용한다면 `ClientRegistrations.fromOidcIssuerLocation()`으로 OpenID Connect 제공자의 설정 엔드포인트만 조회할 수 있다.

## ClientRegistrationRepository

`ClientRegistration`을 저장해두는 컴포넌트다. 하나의 애플리케이션에서 여러 OAuth 2.0 제공자를 동시에 지원할 수 있기 때문에 해당 컴포넌트에서 설정된 각각의 `ClientRegistration`을 저장한다. 기본적으로 `InMemoryClientRegistrationRepository`가 사용되며, 해당 구현체는 내부적으로 `Map<String, ClientRegistration>` 형식으로 `ClientRegistration`을 메모리에 보관한다.

```java
public final class InMemoryClientRegistrationRepository
		implements ClientRegistrationRepository, Iterable<ClientRegistration> {

	private final Map<String, ClientRegistration> registrations;
	// ...
}
```

참고로 스프링 부트 자동 구성 기능에서는 `ClientRegistration`을 설정된 애플리케이션 정보를 기반으로 구성하고 이를 `ClientRegistrationRepository`에 등록하는 동시에 `ClientRegistrationRepository`를 스프링 빈으로 등록하기도 한다. 따라서 필요하다면 의존성 주입하여 사용할 수도 있다.

## OAuth2AuthorizedClient

클라이언트가 리소스 오너(End-User)로부터 자원 접근을 인가하여 인가 토큰을 받은 것을 표현하는 컴포넌트다. 이 컴포넌트는 `OAuth2AccessToken` (설정에 따라서 `OAuth2RefreshToken`)을 `ClientRegistration`과 리소스 오너(`Principal`)와 하나로 묶어 관리한다. 이는 내부 구현 필드를 살펴보면 알 수 있다.

```java
public class OAuth2AuthorizedClient implements Serializable {

	private static final long serialVersionUID = 620L;
	private final ClientRegistration clientRegistration;
	private final String principalName;
	private final OAuth2AccessToken accessToken;
	private final OAuth2RefreshToken refreshToken;
	// ...
}
```

## OAuth2AuthorizedClientRepository / OAuth2AuthorizedClientService

`OAuth2AuthorizedClientRepository`는 웹 요청을 기준으로 `OAuth2AuthorizedClient` (이미 OAuth 2.0 인가가 끝난 상태를 나타냄)를 저장 및 조회하고, `OAuth2AuthorizedClientService`는 애플리케이션 수준에서 `OAuth2AuthorizedClient`를 관리할 수 있도록 하는 컴포넌트다. 이 둘은 완전히 분리되는 개념이 아니라 구현에 따라 `OAuth2AuthorizedClientRepository`에서 `OAuth2AuthorizedClientService`로 위임하기도 한다.

쉽게 말해 `OAuth2AuthorizedClientRepository`는 웹 요청 컨텍스트(`HttpServletRequest` / `Response`)에 종속된 방식 (예: 세션, 쿠키, 요청 속성 등)으로 관리하고, `OAuth2AuthorizedClientService`는 웹 요청과 무관한 저장소 (DB, 메모리 등)를 대상으로 관리한다고 보면 된다.

`OAuth2AuthorizedClientService`/`Repository`는 보통 개발자가 직접 호출할 일은 많지 않지만, `oauth2Login()` 흐름 내부에서 토큰을 저장/조회하는 데 사용된다. 특히 OAuth 제공자의 API를 호출하거나(예: Google Calendar), 리프레시 토큰 기반 재발급, 또는 스케일아웃 환경에서 토큰 저장소를 Redis/DB로 바꾸는 경우에는 이 컴포넌트들의 설정과 동작을 이해하는 것이 중요하다.

스프링 부트 자동 구성은 이 두 컴포넌트를 스프링 빈으로 등록하기 때문에 의존성 주입을 통해 애플리케이션 비즈니스 코드에서도 사용할 수 있다.

```java
@Controller
public class OAuth2ClientController {

    @Autowired
    private OAuth2AuthorizedClientService authorizedClientService;

    @GetMapping("/")
    public String index(Authentication authentication) {
        OAuth2AuthorizedClient authorizedClient =
            this.authorizedClientService
		            .loadAuthorizedClient("okta", authentication.getName());

        OAuth2AccessToken accessToken = authorizedClient.getAccessToken();

        // ...

        return "index";
    }
}
```

또한 이들에 대한 커스텀 구현체를 만들면, 스프링 부트 자동 구성은 기본 구현체 대신에 커스텀 컴포넌트를 스프링 빈으로 등록한다.

특히 `OAuth2AuthorizedClientService` 기본 제공 구현체로는 기본적으로 자동 등록되는 `InMemoryOAuth2AuthorizedClientService`와 별도 설정을 통해서 사용할 수 있는 `JdbcOAuth2AuthorizedClientService`가 있다.

하지만 JDBC 구현체는 다음과 같은 스키마를 강제로 사용해야 한다.

```sql
CREATE TABLE oauth2_authorized_client (
  client_registration_id varchar(100) NOT NULL,
  principal_name varchar(200) NOT NULL,
  access_token_type varchar(100) NOT NULL,
  access_token_value blob NOT NULL,
  access_token_issued_at timestamp NOT NULL,
  access_token_expires_at timestamp NOT NULL,
  access_token_scopes varchar(1000) DEFAULT NULL,
  refresh_token_value blob DEFAULT NULL,
  refresh_token_issued_at timestamp DEFAULT NULL,
  created_at timestamp DEFAULT CURRENT_TIMESTAMP NOT NULL,
  PRIMARY KEY (client_registration_id, principal_name)
);
```

따라서 JPA 같은 편의 기능을 사용하고 싶을 뿐 아니라, 애플리케이션에 맞는 데이터베이스 스키마를 사용하고자 한다면 커스텀 `OAuth2AuthorizedClientService` 구현체를 고려해야 한다.

## OAuth2AuthorizedClientManager / OAuth2AuthorizedClientProvider

`OAuth2AuthorizedClientManager`는 `OAuth2AuthorizedClient`에 대해 다음과 같은 관리 기능을 제공한다.

- `OAuth2AuthorizedClientProvider`를 활용한 OAuth 2.0 클라이언트 인가 또는 재인가
- `OAuth2AuthorizedClientService` 또는 `OAuth2AuthorizedClientRepository` 를 활용하여 `OAuth2AuthorizedClient`의 저장 및 조회 책임을 위임
- OAuth 2.0 클라이언트가 성공적으로 인가를 받았을 경우 `OAuth2AuthorizationSuccessHandler`에 후속 처리를 위임
- OAuth 2.0 클라이언트가 인가를 실패했을 경우 `OAuth2AuthorizationFailureHandler`에 후속 처리를 위임

`OAuth2AuthorizedClientProvider`는 OAuth 2.0 클라이언트를 인가하거나 이미 인가된 클라이언트의 토큰을 재인가하기 위한 전략(Grant Type)을 구현한 컴포넌트이다.

`OAuth2AuthorizedClientManager`의 기본 구현체는 `DefaultOAuth2AuthorizedClientManager`이며, 이는 위임 기반 컴포지트 디자인 패턴으로 여러 OAuth 2.0 인가 방식을 지원할 수 있는 `OAuth2AuthorizedClientProvider`와 연관되어 있다.

```java
@Bean
public OAuth2AuthorizedClientManager authorizedClientManager(
		ClientRegistrationRepository clientRegistrationRepository,
		OAuth2AuthorizedClientRepository authorizedClientRepository) {

	OAuth2AuthorizedClientProvider authorizedClientProvider =
			OAuth2AuthorizedClientProviderBuilder.builder()
					.authorizationCode()
					.refreshToken()
					.clientCredentials()
					.build();

	DefaultOAuth2AuthorizedClientManager authorizedClientManager =
			new DefaultOAuth2AuthorizedClientManager(
					clientRegistrationRepository, authorizedClientRepository);
	authorizedClientManager.setAuthorizedClientProvider(authorizedClientProvider);

	return authorizedClientManager;
}
```

### DefaultOAuth2AuthorizedClientManager

**contextAttributesMapper**

`DefaultOAuth2AuthorizedClientManager`는 `Function<OAuth2AuthorizeRequest, Map<String, Object>>`타입의 `contextAttributesMapper`와 연관되어 있다. 이 매핑 도구는 `OAuth2AuthorizeRequest`의 속성을 `OAuth2AuthorizationContext`와 연결될 `Map`으로 변환하는 역할을 한다. 이를 통해 `OAuth2AuthorizedClientProvider`에 필수 속성을 제공해야 할 때 유용할 수 있다. 코드 예시는 다음과 같다.

```java
@Bean
public OAuth2AuthorizedClientManager authorizedClientManager(
		ClientRegistrationRepository clientRegistrationRepository,
		OAuth2AuthorizedClientRepository authorizedClientRepository) {

	OAuth2AuthorizedClientProvider authorizedClientProvider =
			OAuth2AuthorizedClientProviderBuilder.builder()
					.authorizationCode()
					.refreshToken()
					.build();

	DefaultOAuth2AuthorizedClientManager authorizedClientManager =
			new DefaultOAuth2AuthorizedClientManager(
					clientRegistrationRepository, authorizedClientRepository);
	authorizedClientManager.setAuthorizedClientProvider(authorizedClientProvider);

	// Assuming the attributes are supplied as `HttpServletRequest` parameters,
	// map the `HttpServletRequest` parameters to `OAuth2AuthorizationContext.getAttributes()`
	authorizedClientManager.setContextAttributesMapper(contextAttributesMapper());

	return authorizedClientManager;
}

private Function<OAuth2AuthorizeRequest, Map<String, Object>> contextAttributesMapper() {
	return authorizeRequest -> {
		Map<String, Object> contextAttributes = Collections.emptyMap();
		HttpServletRequest servletRequest = authorizeRequest.getAttribute(HttpServletRequest.class.getName());
		String param1 = servletRequest.getParameter("param1");
		String param2 = servletRequest.getParameter("param2");
		if (StringUtils.hasText(param1) && StringUtils.hasText(param2)) {
			contextAttributes = new HashMap<>();
			contextAttributes.put("param1", param1);
			contextAttributes.put("param2", param2);
		}
		return contextAttributes;
	};
}
```

### AuthorizedClientServiceOAuth2AuthorizedClientManager

`DefaultOAuth2AuthorizedClientManager`는 `HttpServletRequest` 컨텍스트 내에서 사용하도록 설계되었다. 따라서 웹 요청이 아닌 작업에는 `AuthorizedClientServiceOAuth2AuthorizedClientManager`를 대신 사용하도록 한다.

예를 들어 사용자 상호작용 없이 백그라운드에서 실행되어야 하는 client_credentials 권한 부여 유형의 경우에 이 컴포넌트를 사용할 수 있다. 다음 코드는 그러한 상황에 대해서 `AuthorizedClientServiceOAuth2AuthorizedClientManager`를 구성하는 예시 코드다.

```java
@Bean
public OAuth2AuthorizedClientManager authorizedClientManager(
		ClientRegistrationRepository clientRegistrationRepository,
		OAuth2AuthorizedClientService authorizedClientService) {

	OAuth2AuthorizedClientProvider authorizedClientProvider =
			OAuth2AuthorizedClientProviderBuilder.builder()
					.clientCredentials()
					.build();

	AuthorizedClientServiceOAuth2AuthorizedClientManager authorizedClientManager =
			new AuthorizedClientServiceOAuth2AuthorizedClientManager(
					clientRegistrationRepository, authorizedClientService);
	authorizedClientManager.setAuthorizedClientProvider(authorizedClientProvider);

	return authorizedClientManager;
}
```

## OAuth2AuthorizationRequest

OAuth 2.0 인가 과정에서 인가 요청의 중간 상태는 `OAuth2AuthorizationRequest` 객체에 담긴다. 그리고 이 객체의 저장/조회/제거는 `AuthorizationRequestRepository`가 담당한다.

기본 구현체는 HTTP 세션 기반의 `HttpSessionOAuth2AuthorizationRequestRepository`이다. 하지만 상황에 따라 HTTP 세션을 사용할 수 없거나(또는 세션 의존을 제거하려는 경우) 인가 요청 저장 방식을 커스터마이징해야 한다. 대상 인터페이스는 다음과 같다.

```java
public interface AuthorizationRequestRepository<T extends OAuth2AuthorizationRequest> {
	T loadAuthorizationRequest(HttpServletRequest request);
	void saveAuthorizationRequest(T authorizationRequest, HttpServletRequest request, HttpServletResponse response);
	T removeAuthorizationRequest(HttpServletRequest request, HttpServletResponse response);

}
```

예를 들어 이를 구현한 `CustomOAuth2AuthorizationRequestRepository`가 있다면 다음과 같이 스프링 빈으로 등록한 다음에 필터 체인에 명시하면 된다.

```java
@Bean
public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
  http
		.oauth2Login(oauth2 -> oauth2
		  .authorizationEndpoint(authEndpoint -> authEndpoint
//        .baseUri("OAUTH_BASE_URL")
		    .authorizationRequestRepository(authorizationRequestRepository())
		  )
		);

  return http.build();
}

@Bean
public AuthorizationRequestRepository<OAuth2AuthorizationRequest> authorizationRequestRepository() {
	return new CustomOAuth2AuthorizationRequestRepository();
}
```

## OAuth2UserService

사용자 정보 엔드포인트에 요청하여 `Principal` 생성을 담당하는 컴포넌트다. OAuth 2.0에서는 `DefaultOAuth2UserService`가 기본 구현체로 사용되고, OIDC에서는 `OidcUserService`가 기본 구현체로 사용된다. `OidcUserService`는 ID 토큰을 처리하고, 사용자 정보가 필요하면 내부적으로 `DefaultOAuth2UserService`를 위임해서 사용자 정보를 요청하는 구조다.

이 두 구현체는 각각 `OAuth2User`와 이를 한 번 더 상속한 `OidcUser`를 반환한다. 일반적으로 서비스 구현체를 상속하는 커스텀 컴포넌트를 만들고 사용자 정보 엔드포인트에 요청하는 것은 기본 구현체에 위임한 다음, 애플리케이션에 적합한 커스텀 `OAuth2User` 객체를 반환하는 식으로 만든다.

```java
public class CustomOAuth2UserService extends DefaultOAuth2UserService {

  @Override
  public OAuth2User loadUser(OAuth2UserRequest userRequest) throws OAuth2AuthenticationException {
    OAuth2User oAuth2User = super.loadUser(userRequest);

    // ...

    return new CustomOAuth2User(oAuth2User);
  }
}
```

```java
public class CustomOidcUserService extends OidcUserService {

  @Override
  public OidcUser loadUser(OidcUserRequest userRequest) throws OAuth2AuthenticationException {
    OidcUser oidcUser = super.loadUser(userRequest);

    // ...

    return new CustomOidcUser(oidcUser);
  }
}
```

커스텀 `OAuth2UserService`는 OAuth 2.0이나 OIDC 여부에 따라서 시큐리티 필터 체인에 등록하면 된다.

```java
private final CustomOAuth2UserService customOAuth2UserService;
private final CustomOidcUserService customOidcUserService;

@Bean
public SecurityFilterChain securityFilterChain(HttpSecurity http) throws Exception {
  return http
      .oauth2Login(oauth2 -> oauth2
          .userInfoEndpoint(userInfo -> userInfo
              .userService(customOAuth2UserService) // OAuth 2.0
              .oidcUserService(customOidcUserService) // OIDC
          )
      )
      .build();
}
```

## 컴포넌트 담당 분야 정리

앞선 모든 설명을 간략하게 정리하면 다음과 같다.

- OAuth 2.0 클라이언트 등록 정보(Provider별 설정) 관리: `ClientRegistrationRepository`
- OAuth 2.0 로그인(Authorization Code) 인가 요청의 중간 상태 저장/조회: `AuthorizationRequestRepository`
- 토큰 획득/재인가(Refresh, Client Credentials 등) 오케스트레이션: `OAuth2AuthorizedClientManager` (+ `OAuth2AuthorizedClientProvider`)
- 인가된 클라이언트(Principal + Access/Refresh Token) 저장/조회: `OAuth2AuthorizedClientRepository` / `OAuth2AuthorizedClientService`

## OAuth 2.0 인가 흐름 정리

스프링 시큐리티 컴포넌트를 이용해서 OAuth 2.0 인가 흐름에 대한 전체적인 단계를 컴포넌트 관점과 웹 요청 URL 관점 두 개로 나눠서 살펴보도록 하자.

### 컴포넌트 관점

**인가 성공**

1. 리소스 오너로부터 인가를 받으면 `DefaultOAuth2AuthorizedClientManager`는 `OAuth2AuthorizationSuccessHandler`로 후속 처리를 위임한다.
2. `OAuth2AuthorizationSuccessHandler`는 (기본적으로) `OAuth2AuthorizedClientRepository`를 통해 `OAuth2AuthorizedClient`를 저장한다.

**재인가 실패 (갱신 실패)**

1. 재인가 (갱신)작업이 실패하면 `RemoveAuthorizedClientOAuth2AuthorizationFailureHandler`가 실행된다.
2. 해당 핸들러를 통해서 이전에 저장된 `OAuth2AuthorizedClient`가 `OAuth2AuthorizedClientRepository`에서 제거된다.

> `setAuthorizationSuccessHandler(OAuth2AuthorizationSuccessHandler)`와 `setAuthorizationFailureHandler(OAuth2AuthorizationFailureHandler)`를 통해 동작을 커스터마이징 할 수 있다.

### 웹 요청 URL 관점

앞서 소개한 컴포넌트들이 실제 `oauth2Login()` 흐름에서 어떻게 연결되는지, **웹 요청 URL 기준으로** 큰 흐름만 간단히 정리해보자.

**인가 요청 시작**

1. 사용자가 `/oauth2/authorization/{registrationId}` 로 접근한다.
2. 스프링 시큐리티는 `{registrationId}`에 해당하는 `ClientRegistration`을 `ClientRegistrationRepository`에서 조회한다.
3. 조회된 `ClientRegistration`을 기반으로 `OAuth2AuthorizationRequest`(인가 요청 정보)를 생성한다.
4. 생성된 `OAuth2AuthorizationRequest`는 `AuthorizationRequestRepository`에 저장된다.

   (기본 구현체는 세션 기반 `HttpSessionOAuth2AuthorizationRequestRepository`)

5. 이후 OAuth 2.0 제공자의 설정된 URI로 리다이렉트된다.

**콜백 처리 (인가 코드 → 토큰 교환)**

1. OAuth 2.0 제공자에서 인증/동의가 완료되면 사용자는 `/login/oauth2/code/{registrationId}` 로 리다이렉트된다.
2. 스프링 시큐리티는 `AuthorizationRequestRepository`에서 앞서 저장해둔 `OAuth2AuthorizationRequest`를 조회/제거하여, callback 요청이 “이전 인가 요청”과 이어지는지 검증한다. (state 검증, redirectUri 매칭 등)
3. 콜백으로 전달받은 `code` 파라미터를 사용해 토큰 엔드포인트로 요청하여 액세스 토큰(필요 시 리프레시 토큰 포함)을 교환한다.
4. OIDC 제공자인 경우 ID Token(JWT) 검증이 수행되며, 설정에 따라 사용자 정보 엔드포인트로 사용자 정보를 추가 조회할 수 있다. (`OAuth2UserService`가 사용되는 지점)
5. 최종적으로 인증이 완료되면 `OAuth2AuthorizedClientRepository`/`OAuth2AuthorizedClientService`를 통해 `OAuth2AuthorizedClient`가 저장된다.
6. 인증된 사용자 정보는 `Authentication`으로 생성되어 `SecurityContext`에 저장되며, 이후 요청부터는 애플리케이션에서 인증된 사용자로 처리된다.

# Spring Security OAuth2 Client 핵심 설정

스프링 부트는 Spring Security OAuth2 Client의존성에 대해서 애플리케이션 설정을 통해 `ClientRegistration`을 자동으로 구성해준다. 이를 위해서 `spring.security.oauth2.client`로 시작하는 다음 하위 속성들이 존재한다.

**Registration**

| 설정                         | ClientRegistration 필드명  | 설명                                          |
| ---------------------------- | -------------------------- | --------------------------------------------- |
| registrationId               | registrationId             | 클라이언트를 식별하는 ID (예: google, github) |
| client-id                    | clientId                   | 발급받은 Client ID                            |
| client-secret                | clientSecret               | 발급받은 Client Secret                        |
| client-authentication-method | clientAuthenticationMethod | 클라이언트 인증 방식 (basic, post 등)         |
| authorization-grant-type     | authorizationGrantType     | 권한 부여 방식 (authorization_code 등)        |
| redirect-uri                 | redirectUri                | 인증 후 리다이렉트될 URI                      |
| scope                        | scopes                     | 요청할 권한 범위                              |
| client-name                  | clientName                 | 클라이언트의 이름 (UI 표시용 등)              |

**Registration.ProviderDetails**

| 설정 정보           | ClientRegistration 필드명        | 설명                                 |
| ------------------- | -------------------------------- | ------------------------------------ |
| authorization-uri   | providerDetails.authorizationUri | 인증 서버의 권한 부여 엔드포인트     |
| token-uri           | providerDetails.tokenUri         | 액세스 토큰 발급 엔드포인트          |
| jwk-set-uri         | providerDetails.jwkSetUri        | 토큰 검증을 위한 공개키(JWK) 경로    |
| issuer-uri          | providerDetails.issuerUri        | 인증 서버의 발행자(Issuer) 식별 주소 |
| user-info-uri       | userInfoEndpoint.uri             | 사용자 정보를 가져오는 엔드포인트    |
| user-name-attribute | userNameAttributeName            | 사용자 이름으로 사용할 속성 키값     |

## CommonOAuth2Provider

또한 많이 사용되는 OAuth 2.0 제공자는 `CommonOAuth2Provider`를 통해 사전에 미리 정의되어 있다. 구글, 깃허브, 페이스북, X(트위터), Okta가 사전에 정의된 OAuth 2.0 제공자에 속한다. 이러한 경우에는 `client-id`와 `client-secret`만 알고 있으면 된다. 구글 OAuth 2.0 로그인을 위해서는 애플리케이션에서 다음과 같이 설정만 해주면 된다.

```yaml
spring:
  security:
    oauth2:
      client:
        registration:
          google:
            client-id: google-client-id
            client-secret: google-client-secret
            scope: email, profile
```

참고로 `registrationId`는 이미 yaml 속성에 `google`이라고 명시되어 있기 때문에 별도로 명시해줄 이유는 없다. 만약에 해당 속성에 대해서 별도로 사용하고자 한다면 `provider` 속성을 명시해주기만 하면 된다.

```yaml
spring:
  security:
    oauth2:
      client:
        registration:
          google-login:
            provider: google
            client-id: google-client-id
            client-secret: google-client-secret
            scope: email, profile
```

### 커스텀 제공자 정의

일부 OAuth 2.0 제공자(예: Okta)는 멀티 테넌시 구조를 가지며, 테넌트마다 서로 다른 인증 엔드포인트를 사용하기도 한다. 또한 Spring Security가 기본 제공하지 않는 OAuth 2.0 제공자도 존재한다. 이러한 경우 `spring.security.oauth2.client.provider.{providerId}` 설정을 통해 OAuth 2.0 제공자에 대한 정보를 직접 구성해야 한다.

```yaml
spring:
  security:
    oauth2:
      client:
        registration:
          okta:
            client-id: okta-client-id
            client-secret: okta-client-secret
        provider:
          okta:
            authorization-uri: https://your-subdomain.oktapreview.com/oauth2/v1/authorize
            token-uri: https://your-subdomain.oktapreview.com/oauth2/v1/token
            user-info-uri: https://your-subdomain.oktapreview.com/oauth2/v1/userinfo
            user-name-attribute: sub
            jwk-set-uri: https://your-subdomain.oktapreview.com/oauth2/v1/keys
```

이와 같이 제공자를 직접 정의하면 Spring Security의 `CommonOAuth2Provider`에 포함되지 않은 OAuth 2.0 제공자이거나, 멀티 테넌시 구조로 인해 엔드포인트가 고정되지 않은 제공자를 명시된 속성 값으로 구성할 수 있다.

> **테넌트, 태넌시, 멀티 테넌시**
>
> 테넌트란 특정 사용자나 조직을 뜻하며, 테넌시는 테넌트가 IT 자원을 사용하는 운영 구조를 말한다.
>
> 멀티 테넌시는 여러 사용자나 조직이 동일한 IT 자원을 사용하면서도 데이터, 설정, 보안 측면에서 논리적으로 격리되어 있는 것을 말한다.

## OAuth2Login DSL

OAuth 2.0 로그인에 대한 설정은 시큐리티 필터 체인에 있는 DSL로 구성할 수 있다. 코드 예시를 보도록 하자.

```java
@Configuration
@EnableWebSecurity
public class OAuth2LoginSecurityConfig {
	@Bean
	public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
		http
			// OAuth 2.0 / OIDC 로그인 전체 동작 활성화 및 설정
			.oauth2Login((oauth2) -> oauth2
				.clientRegistrationRepository(this.clientRegistrationRepository())
				.authorizedClientRepository(this.authorizedClientRepository())
				.authorizedClientService(this.authorizedClientService())
				// OAuth2 로그인 시작/실패 시 보여줄 커스텀 로그인 페이지 URL 지정
				.loginPage("/login")
				// 인가 엔드포인트 동작 커스터마이징
				.authorizationEndpoint((authorization) -> authorization
					// 인가 요청을 시작하는 엔드포인트 설정 (기본: /oauth2/authorization/{registrationId})
					.baseUri(this.authorizationRequestBaseUri())
					// OAuth 2.0 상태(AuthorizationRequest) 저장을 위한 레포지토리 설정
					.authorizationRequestRepository(this.authorizationRequestRepository())
					// 인가 요청을 생성/수정하는 Resolver 지정 (동적 파라미터 추가, 멀티테넌트 등)
					.authorizationRequestResolver(this.authorizationRequestResolver())
				)
				// 인가 응답을 수신하는 콜백(리다이렉트 URI) 엔드포인트 동작 커스터마이징
				.redirectionEndpoint((redirection) -> redirection
					// 기본 URI 설정 (기본: /login/oauth2/code/*)
					.baseUri(this.authorizationResponseBaseUri())
				)
				// 토큰 엔드포인트 동작 커스터마이징
				.tokenEndpoint((token) -> token
					// 토큰 교환 요청/응답 처리 클라이언트 지정 (커스텀 RestOperations, 파라미터 확장 등)
					.accessTokenResponseClient(this.accessTokenResponseClient())
				)
				// 사용자 정보 엔드포인트 동작 커스터마이징
				.userInfoEndpoint((userInfo) -> userInfo
					// 기본으로 생성된 GrantedAuthority(SCOPE_*, OIDC_USER 등)를 애플리케이션 권한으로 매핑
					.userAuthoritiesMapper(this.userAuthoritiesMapper())
					// OAuth2User를 로딩하는 OAuth2UserService 지정
					.userService(this.oauth2UserService())
					// OidcUser(id_token/userinfo 포함)를 로딩하는 OidcUserService 지정
					.oidcUserService(this.oidcUserService())
				)
			);
		return http.build();
	}
}

```

# OAuth2ClientAutoConfiguration

OAuth2 Client를 위한 자동 구성에 사용되는 컴포넌트이다. 이는 정해진 설정 정보를 읽어서 다음 작업을 수행한다.

- `ClientRegistration`이 등록된 `ClientRegistrationRepository`를 빈으로 등록
- `http.oauth2Login()` 구성이 적용될 때 OAuth 2.0 Login이 동작할 수 있도록 `SecurityFilterChain`에 필요한 기본 구성을 제공한다.

만약 이러한 자동 구성을 덮어쓰고 싶다면 다음 방법을 사용하면 된다.

## ClientRegistrationRepository 빈 등록

```java
@Configuration
public class OAuth2LoginConfig {

	@Bean
	public ClientRegistrationRepository clientRegistrationRepository() {
		return new InMemoryClientRegistrationRepository(this.googleClientRegistration());
	}

	private ClientRegistration googleClientRegistration() {
		return ClientRegistration.withRegistrationId("google")
			.clientId("google-client-id")
			.clientSecret("google-client-secret")
			.clientAuthenticationMethod(ClientAuthenticationMethod.CLIENT_SECRET_BASIC)
			.authorizationGrantType(AuthorizationGrantType.AUTHORIZATION_CODE)
			.redirectUri("{baseUrl}/login/oauth2/code/{registrationId}")
			.scope("openid", "profile", "email", "address", "phone")
			.authorizationUri("https://accounts.google.com/o/oauth2/v2/auth")
			.tokenUri("https://www.googleapis.com/oauth2/v4/token")
			.userInfoUri("https://www.googleapis.com/oauth2/v3/userinfo")
			.userNameAttributeName(IdTokenClaimNames.SUB)
			.jwkSetUri("https://www.googleapis.com/oauth2/v3/certs")
			.clientName("Google")
			.build();
	}
}
```

## SecurityFilterChain 빈 등록

```java
@Configuration
@EnableWebSecurity
public class OAuth2LoginSecurityConfig {

	@Bean
	public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
		http
			.authorizeHttpRequests((authorize) -> authorize
				.anyRequest().authenticated()
			)
			.oauth2Login(withDefaults());
		return http.build();
	}
}
```

이 두 개를 종합하여 스프링 부트 자동 구성을 완전히 덮어쓴다면 다음과 같이 설정하면 된다.

```java
@Configuration
public class OAuth2LoginConfig {

	@Bean
	public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
		http
			.authorizeHttpRequests((authorize) -> authorize
				.anyRequest().authenticated()
			)
			.oauth2Login(withDefaults());
		return http.build();
	}

	@Bean
	public ClientRegistrationRepository clientRegistrationRepository() {
		return new InMemoryClientRegistrationRepository(this.googleClientRegistration());
	}

	private ClientRegistration googleClientRegistration() {
		return ClientRegistration.withRegistrationId("google")
			.clientId("google-client-id")
			.clientSecret("google-client-secret")
			.clientAuthenticationMethod(ClientAuthenticationMethod.CLIENT_SECRET_BASIC)
			.authorizationGrantType(AuthorizationGrantType.AUTHORIZATION_CODE)
			.redirectUri("{baseUrl}/login/oauth2/code/{registrationId}")
			.scope("openid", "profile", "email", "address", "phone")
			.authorizationUri("https://accounts.google.com/o/oauth2/v2/auth")
			.tokenUri("https://www.googleapis.com/oauth2/v4/token")
			.userInfoUri("https://www.googleapis.com/oauth2/v3/userinfo")
			.userNameAttributeName(IdTokenClaimNames.SUB)
			.jwkSetUri("https://www.googleapis.com/oauth2/v3/certs")
			.clientName("Google")
			.build();
	}
}
```

## 순수 자바 코드를 통한 설정

때때로 스프링 부트가 아닌 레거시 프로젝트에서 `CommonOAuth2Provider`를 설정하고 싶다면 다음과 같이 설정할 수도 있다.

```java
@Configuration
@EnableWebSecurity
public class OAuth2LoginConfig {

	@Bean
	public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
		http
			.authorizeHttpRequests((authorize) -> authorize
				.anyRequest().authenticated()
			)
			.oauth2Login(withDefaults());
		return http.build();
	}

	@Bean
	public ClientRegistrationRepository clientRegistrationRepository() {
		return new InMemoryClientRegistrationRepository(this.googleClientRegistration());
	}

	@Bean
	public OAuth2AuthorizedClientService authorizedClientService(
			ClientRegistrationRepository clientRegistrationRepository) {
		return new InMemoryOAuth2AuthorizedClientService(clientRegistrationRepository);
	}

	@Bean
	public OAuth2AuthorizedClientRepository authorizedClientRepository(
			OAuth2AuthorizedClientService authorizedClientService) {
		return new AuthenticatedPrincipalOAuth2AuthorizedClientRepository(authorizedClientService);
	}

	private ClientRegistration googleClientRegistration() {
		return CommonOAuth2Provider.GOOGLE.getBuilder("google")
			.clientId("google-client-id")
			.clientSecret("google-client-secret")
			.build();
	}
}
```

---

# 참고 자료

[**Spring Authorization Server**](https://spring.io/projects/spring-authorization-server)

[**Spring Security - OAuth2**](https://docs.spring.io/spring-security/reference/servlet/oauth2/index.html)

[**Spring Security - Core Configuration**](https://docs.spring.io/spring-security/reference/servlet/oauth2/login/core.html)

[**Spring Security - Advanced Configuration**](https://docs.spring.io/spring-security/reference/servlet/oauth2/login/advanced.html)

[**Spring Security - Core Interfaces and Classes**](https://docs.spring.io/spring-security/reference/servlet/oauth2/client/core.html)

[**Spring Security - Authorization Grant Support**](https://docs.spring.io/spring-security/reference/servlet/oauth2/client/authorization-grants.html)

[**멀티테넌시란?**](https://www.redhat.com/ko/topics/cloud-computing/what-is-multitenancy)
