---
title: Spring Security를 이용한 OAuth 2.0 로그인 구현

categories:
  - Spring Security

toc: true
toc_sticky: true
published: true

date: 2026-03-02
---

스프링 시큐리티를 통해서 OAuth 2.0 로그인을 구현할 것이다. 구글과 카카오 로그인을 구현할 것이며, 구글 로그인은 OpenID Connect로, 카카오 로그인은 일반적인 OAuth 2.0 프로토콜로 구현할 것이다. 그리고 인가 정책은 Authorization Code 방식으로 구현할 것이다.

OAuth 2.0을 위한 사전 준비는 다른 참고 자료를 통해 살펴보는 것을 추천하고 여기에서는 스프링 시큐리티의 OAuth 2.0 로그인 자체에만 집중하도록 할 것이다. 백엔드 주도 방식을 먼저 구현하고, 여기에 BFF 패턴을 추가해 SPA에서도 백엔드 주도 방식을 사용할 수 있도록 만들어볼 것이다.

# 백엔드 주도 방식

## 애플리케이션 설정

**build.gradle**

스프링 시큐리티 의존성에 이어서 앞서 언급한 OAuth2 Client를 의존성으로 추가하도록 하자.

```groovy
implementation 'org.springframework.boot:spring-boot-starter-oauth2-client'
```

**application.yml**

구글과 카카오 OAuth 2.0 관련 클라이언트 정보를 명시했다.

```yaml
spring:
  application:
    name: spring-security
  datasource:
    url: jdbc:mysql://localhost:3306/security?useSSL=false&serverTimezone=UTC
    username: root
    password: 1234
    driver-class-name: com.mysql.cj.jdbc.Driver

  security:
    oauth2:
      client:
        registration:
          google:
            client-id: ${GOOGLE_CLIENT_ID}
            client-secret: ${GOOGLE_CLIENT_SECRET}
            scope: openid, profile, email
          kakao:
            client-id: ${KAKAO_CLIENT_ID}
            client-secret: ${KAKAO_CLIENT_SECRET}
            client-authentication-method: client_secret_post
            authorization-grant-type: authorization_code
            redirect-uri: "{baseUrl}/login/oauth2/code/{registrationId}"
            scope: profile_nickname, # account_email
            client-name: Kakao
        provider:
          kakao:
            authorization-uri: https://kauth.kakao.com/oauth/authorize
            token-uri: https://kauth.kakao.com/oauth/token
            user-info-uri: https://kapi.kakao.com/v2/user/me
            user-name-attribute: id
```

카카오는 이메일을 조회하기 위해서는 비즈 앱을 등록해야 하기 때문에 간단하게 닉네임을 조회해서 애플리케이션의 ID로 쓰도록 하겠다.

> 백엔드 주도 방식은 OAuth 2.0 인가 이후의 리다이렉트가 백엔드 서버로 이어져야 한다. 스프링 시큐리티는 내부적으로 `{baseUrl}/login/oauth2/code/{registrationId}`로 리다이렉트하면 OAuth 2.0 로그인 이후의 과정을 알아서 처리해준다.
>
> 따라서 구글 OAuth 2.0 로그인을 구현하고자 한다면 구글 클라우드 OAuth 2.0 설정 부분에서 승인된 리다이렉션 URI를 `{baseUrl}/login/oauth2/code/google`로 설정해야 한다.

## 회원 가입 및 로그인

OAuth 2.0에서는 리소스 서버로부터 전달된 식별정보를 기반으로 데이터베이스를 조회하고, 없다면 새로운 사용자를 데이터베이스에 저장하는 방식으로 동시에 회원 가입과 로그인이 존재한다고 생각한다. 이를 위해서 레포지토리와 서비스를 다음과 같이 만들었다.

### 레포지토리

```java
@Repository
public interface UserRepository extends JpaRepository<User, Long> {
    Optional<User> findByUsername(String username);
}
```

### 서비스

```java
@Service
@Transactional
@RequiredArgsConstructor
public class UserService {

  private final UserRepository userRepository;

  public User findOrCreateOauth2User(String username) {
    return userRepository.findByUsername(username)
        .orElseGet(() -> userRepository.save(createOauth2User(username)));
  }

  private User createOauth2User(String username) {
      return User.builder()
          .username(username)
          .password(null)
          .role("USER")
          .build();
  }
}
```

## User 및 UserService 설정

스프링 시큐리티는 OAuth 2.0과 OpenID Connect 방식에 따라서 `DefaultOAuth2UserService`와 `OidcUserService`를 지원한다. 해당 컴포넌트들은 리소스 서버나 유저 엔드포인트로부터 사용자 정보를 조회하여 각각 `OAuth2User`와 `OidcUser` 객체를 반환한다. 따라서 이 들을 구현하는 커스텀 구현체를 먼저 만들어야 한다.

### OAuth 2.0

**OAuth2User**

```java
@Getter
public class CustomOAuth2User implements OAuth2User, UserDetails {

  private final Long id;
  private final String username;
  private final String role;
  private final OAuth2User oAuth2User;

  public CustomOAuth2User(User user, OAuth2User oAuth2User) {
    this.id = user.getId();
    this.username = user.getUsername();
    this.role = user.getRole();
    this.oAuth2User = oAuth2User;
  }

  @Override
  public Map<String, Object> getAttributes() {
    return oAuth2User.getAttributes();
  }

  @Override
  public String getName() {
    return oAuth2User.getName();
  }

  @Override
  public Collection<? extends GrantedAuthority> getAuthorities() {
    return List.of(new SimpleGrantedAuthority(role));
  }

  @Override
  public String getUsername() {
    return username;
  }
}
```

**OAuthAttributeExtractor**

현재는 구글 OAuth 2.0 로그인을 OpenID Connect로 처리하고 있지만, 만약 이를 단순히 OAuth 2.0으로 사용한다면 카카오와 사용자 정보 반환 형식이 다르다. 따라서 제공자마다 다르게 반환된 사용자 정보 응답 형식에 맞게 속성을 추출하는 클래스를 사용해야 한다.

```java
public final class OAuthAttributeExtractor {

  private OAuthAttributeExtractor() {
  }

  public static String extractEmail(
		  String provider,
		  Map<String, Object> attributes
  ) {

	  return switch (provider) {
	    case "google" -> (String) attributes.get("email");
	    case "kakao" -> {
	      Map<String, Object> account =
	              (Map<String, Object>) attributes.get("kakao_account");
	      Map<String, Object> profile = (Map<String, Object>) account.get("profile");
	      yield profile.get("nickname").toString();
	    }
	    default -> throw new IllegalArgumentException();
	  };
  }
}
```

**OAuth2UserService**

```java
@Service
@RequiredArgsConstructor
public class CustomOAuth2UserService extends DefaultOAuth2UserService {

  private final UserService userService;

  @Override
  public OAuth2User loadUser(OAuth2UserRequest userRequest) throws OAuth2AuthenticationException {
    OAuth2User oAuth2User = super.loadUser(userRequest);

    String registrationId = userRequest.getClientRegistration().getRegistrationId();
    Map<String, Object> attributes = oAuth2User.getAttributes();

    User user = findOrSave(registrationId, attributes);

    return new CustomOAuth2User(user, oAuth2User);
  }

  private User findOrSave(String registrationId, Map<String, Object> attributes) {
    String email = OAuthAttributeExtractor.extractEmail(registrationId, attributes);

    return userService.findOrCreateOauth2User(email);
  }
}
```

### OpenID Connect

**OidcUser**

```java
@Getter
public class CustomOidcUser implements OidcUser, UserDetails {

  private final Long id;
  private final String username;
  private final String role;
  private final OidcUser oidcUser;

  public CustomOidcUser(
		  User user,
		  OidcUser oidcUser
  ) {
    this.id = user.getId();
    this.username = user.getUsername();
    this.role = user.getRole();
    this.oidcUser = oidcUser;
  }

  @Override
  public Map<String, Object> getClaims() {
    return oidcUser.getClaims();
  }

  @Override
  public OidcIdToken getIdToken() {
    return oidcUser.getIdToken();
  }

  @Override
  public OidcUserInfo getUserInfo() {
    return oidcUser.getUserInfo();
  }

  @Override
  public String getName() {
    return oidcUser.getName();
  }

  @Override
  public <A> A getAttribute(String name) {
    return OidcUser.super.getAttribute(name);
  }

  @Override
  public Map<String, Object> getAttributes() {
    return oidcUser.getAttributes();
  }

  @Override
  public Collection<? extends GrantedAuthority> getAuthorities() {
    return List.of(new SimpleGrantedAuthority(role));
  }
}
```

**OidcUserService**

```java
@Slf4j
@Service
@RequiredArgsConstructor
public class CustomOidcUserService extends OidcUserService {

  private final UserService userService;

  @Override
  public OidcUser loadUser(OidcUserRequest userRequest) throws OAuth2AuthenticationException {
    OidcUser oidcUser = super.loadUser(userRequest);

    OidcIdToken idToken = oidcUser.getIdToken();

    String email = idToken.getEmail();

    User user = userService.findOrCreateOauth2User(email);

    return new CustomOidcUser(user, oidcUser);
  }
}
```

여기서 OAuth 2.0과 OIDC의 차이가 드러난다. 앞선 OAuth 2.0 로그인을 다룰 때에는 각 OAuth 2.0 제공자마다 응답 형식이 다르기 때문에 이에 맞춰서 직접 객체를 캐스팅 하여서 사용자 식별값에 접근해야 했다.

하지만 OIDC는 사용자 식별 값이 ID 토큰에 있고, 스프링 시큐리티는 이에 관해서 `OidcUser`가 ID 토큰을 가지도록 설계되어 있기 때문에 제공자가 누군지 상관하지 않고 `OidcIdToken.getEmail()`로 사용자 식별 값에 접근이 가능하다.

### 공통 UserDetails 객체 만들기

애플리케이션에서 `UserDetails`를 활용하여 사용자를 식별할 수 있지만 상황에 따라서 별도의 식별 정보를 이용해야 할 수도 있다. 이 경우에는 `@AuthenticationPrincipal` 로 `UserDetails` 객체를 컨트롤러 메서드에서 주입받는 것 보다는 별도의 인터페이스를 정의하여 모든 `UserDetails`의 하위 객체에 구현 사항으로 명시할 수도 있다.

앞서 인증 정보에서 사용자의 ID 번호를 통해 식별을 하고자 한다면 `OAuth2User`객체와 `OidcUser` 객체를 다음 인터페이스로 묶어서 처리할 수 있다.

```java
public interface AuthPrincipal {
  Long getId();

  String getUsername();

  String getRole();
}
```

이를 다음과 같이 공통적으로 명시한다.

```java
public class CustomOidcUser
		implements OidcUser, AuthPrincipal {...}
```

```java
public class CustomOAuth2User
		implements OAuth2User, AuthPrincipal {...}
```

이러면 `@AuthenticationPrincipal`을 다음과 같이 사용하면 된다.

```java
@Slf4j
@Controller
public class ExampleController {

  @GetMapping
  public String example(
	    @AuthenticationPrincipal AuthPrincipal user,
  ) {...}
}
```

## SecurityFilterChain 설정

```java
@Configuration
@EnableWebSecurity
@RequiredArgsConstructor
public class SecurityConfiguration {

  private final CustomOAuth2UserService customOAuth2UserService;
  private final CustomOidcUserService customOidcUserService;

  @Bean
  public SecurityFilterChain securityFilterChain(HttpSecurity http) throws Exception {
	  return http
        .authorizeHttpRequests(auth -> auth
            .requestMatchers(PathRequest.toStaticResources().atCommonLocations())
            .permitAll()
            .requestMatchers("/oauth2/**").permitAll()
            .anyRequest().authenticated()
        )
        .oauth2Login(oauth2 -> oauth2
            .defaultSuccessUrl("/", true)
            // 커스텀 OAuth2와 OpenID Connect 서비스 매핑
            .userInfoEndpoint(userInfo -> userInfo
		            .userService(customOAuth2UserService)
		            .oidcUserService(customOidcUserService)
            ).permitAll()
        )
        .build();
  }
}
```

JWT 관련 필터 기능은 앞선 포스팅을 참고하도록 하자.

# SPA 및 모바일 클라이언트를 위한 백엔드 주도 토큰 발급

백엔드 주도 방식을 SPA나 앱 등에서 활용할 수 있게 만들려면 `OAuth2AuthorizationSuccessHandler`를 등록하여 SPA로 리다이렉트 시키는 동시에 애플리케이션 자체적으로 발급한 JWT를 발급하면 된다. 리액트나 Vue.js 같은 웹 프론트엔드로 리다이렉트 시키는 `OAuth2AuthorizationSuccessHandler`를 등록하도록 하자.

## JWT

### 애플리케이션 설정

**build.gradle**

```groovy
implementation 'io.jsonwebtoken:jjwt-api:0.13.0'
runtimeOnly 'io.jsonwebtoken:jjwt-impl:0.13.0'
runtimeOnly 'io.jsonwebtoken:jjwt-jackson:0.13.0'
```

**application.yml**

```yaml
jwt:
  secret: ${JWT_SECRET}
  access-token-expiration: 86400000
  refresh-token-expiration: 604800000
```

### JwtProvider

`JwtProvider`는 크게 다를 것 없지만 JWT 파싱 이후에 OAuth2, OIDC 구분 없이 하나의 객체로 간주하기 위해서 `UserDetails`와 `AuthPrincipal` 모두 구현하는 임의의 `JwtUser`를 구현했다.

```java
@Getter
@RequiredArgsConstructor
public class JwtUser implements UserDetails, AuthPrincipal {

  private final Long id;
  private final String username;
  private final String role;

  @Override
  public Collection<? extends GrantedAuthority> getAuthorities() {
    return List.of(new SimpleGrantedAuthority(role));
  }

  @Override
  public @Nullable String getPassword() {
    return null;
  }
}
```

이를 파싱 이후에 반환하도록 코드를 약간 수정했다. 전체 코드는 다음과 같다.

```java
@Getter
@Component
public class JwtProvider {

  private static final String TOKEN_PREFIX = "Bearer ";
  private static final String ID_CLAIM = "id";
  private static final String USERNAME_CLAIM = "username";
  private static final String ROLE_CLAIM = "role";

  private final SecretKey secretKey;
  private final Long accessTokenExpirationMs;
  private final Long refreshTokenExpirationMs;

  public JwtProvider(
		  @Value("${jwt.secret}") String secret,
		  @Value("${jwt.access-token-expiration}") Long accessTokenExpirationMs,
		  @Value("${jwt.refresh-token-expiration}") Long refreshTokenExpirationMs
  ) {
    this.secretKey = Keys.hmacShaKeyFor(Decoders.BASE64.decode(secret));
    this.accessTokenExpirationMs = accessTokenExpirationMs;
    this.refreshTokenExpirationMs = refreshTokenExpirationMs;
  }

  public TokenInformation generateToken(Authentication authentication) {
	  return new TokenInformation(
		    createToken(authentication, accessTokenExpirationMs),
		    createToken(authentication, refreshTokenExpirationMs)
	  );
  }

  private String createToken(Authentication authentication, long expirationMs) {
	  if (!(authentication.getPrincipal() instanceof AuthPrincipal userDetails)) {
      throw new RuntimeException("User is not authenticated");
	  }

	  Date issuedAt = new Date();

	  return Jwts.builder()
        .claim(ID_CLAIM, userDetails.getId())
        .claim(USERNAME_CLAIM, userDetails.getUsername())
        .claim(ROLE_CLAIM, userDetails.getRole())
        .expiration(new Date(issuedAt.getTime() + expirationMs))
        .issuedAt(issuedAt)
        .signWith(secretKey)
        .compact();
  }

  public UserDetails getUserDetails(String token) {
	  Claims claims = parseToken(token);

	  User user = createAuthenticatedUser(claims);

	  return new JwtUser(
		    user.getId(),
		    user.getUsername(),
		    user.getRole()
	  );
  }

  private Claims parseToken(String token) {
	  try {
	      return Jwts.parser()
		        .verifyWith(secretKey)
		        .build()
		        .parseSignedClaims(getTokenWithoutPrefix(token))
		        .getPayload();
	  } catch (UnsupportedJwtException | IllegalArgumentException e) {
      throw new RuntimeException("Invalid JWT token", e);
	  }
  }

  private String getTokenWithoutPrefix(String token) {
    return token.substring(TOKEN_PREFIX.length());
  }

  private User createAuthenticatedUser(Claims claims) {
    return User.builder()
        .id(claims.get(ID_CLAIM, Long.class))
        .username(claims.get(USERNAME_CLAIM, String.class))
        .role(claims.get(ROLE_CLAIM, String.class))
        .build();
  }
}
```

## OAuth2 핸들러 추가

OAuth 성공과 실패를 핸들링하는 SPA 주소로 보내도록 각각 다음과 같이 설정했다. 리다이렉트 주소는 간단하게 `http://localhost:3000`으로 보낸다고 가정했다.

### SuccessHandler

성공 관련 핸들러는 리프레시 토큰 관련 처리는 무조건 쿠키로 보내기 때문에 관련 코드는 생략하고 간단하게 액세스 토큰만 보낸다고 가정했다.

**쿠키 기반 토큰 전송**

리프레시 토큰과 액세스 토큰 모두를 쿠키에 담아서 보내는 방식이다.

```java
@Component
@RequiredArgsConstructor
public class OAuth2SuccessHandler extends SimpleUrlAuthenticationSuccessHandler {

  private static final int MILLISECONDS_IN_SECOND = 1000;
  private static final String SAME_SITE_POLICY = "Lax";

  private final JwtProvider jwtProvider;

  @Override
  public void onAuthenticationSuccess(
		  HttpServletRequest request,
		  HttpServletResponse response,
		  Authentication authentication
  ) throws IOException {
    TokenInformation tokenInformation = jwtProvider.generateToken(authentication);

    ResponseCookie cookie = ResponseCookie.from("accessToken", tokenInformation.accessToken())
        .path("/")
        .httpOnly(true)
        // .secure(true) // HTTPS 사용 시 해당 설정 사용
        // .domain("localhost") // 필요 시 도메인 설정
        .maxAge(convertToSeconds())
        .sameSite(SAME_SITE_POLICY)
        .build();

    String redirectUrl = UriComponentsBuilder
		    .fromUriString("http://localhost:3000")
        .toUriString();

    response.addHeader(HttpHeaders.SET_COOKIE, cookie.toString());

    clearAuthenticationAttributes(request); // 인증이 성공했으므로 세션 정리

    getRedirectStrategy().sendRedirect(request, response, redirectUrl);
  }

  private int convertToSeconds() {
    return jwtProvider.getAccessTokenExpirationMs().intValue() / MILLISECONDS_IN_SECOND;
  }
}
```

보안이 좋은 방식이므로 권장된다고 하지만 백엔드 쪽에서 CSRF 공격에 유의해야 하므로 세션 쿠키인 JSESSIONID관련 옵션을 다음과 같이 설정해줘야 한다고 한다.

```yaml
server:
  servlet:
    session:
      cookie:
        same-site: lax
```

**쿼리 파라미터를 통한 액세스 토큰 전달**

```java
@Component
@RequiredArgsConstructor
public class OAuth2SuccessHandler extends SimpleUrlAuthenticationSuccessHandler {

  private final JwtProvider jwtProvider;

  @Override
  public void onAuthenticationSuccess(
		  HttpServletRequest request,
		  HttpServletResponse response,
		  Authentication authentication
  ) throws IOException {
	  TokenInformation tokenInformation = jwtProvider.generateToken(authentication);

	  String redirectUrl = UriComponentsBuilder
			  .fromUriString("http://localhost:3000")
        .queryParam("accessToken", tokenInformation.accessToken())
        .toUriString();

	  getRedirectStrategy().sendRedirect(request, response, redirectUrl);
  }
}
```

리프레시 토큰을 쿠키에 담아서 보내고, 액세스 토큰을 쿼리 파라미터로 보내는 방법이다. 이렇게 URL 파라미터로 넘기면 브라우저 기록에 남아서 보안에 취약할 수 있다. 다만 모바일 앱은 쿠키를 보낼 방법이 없기 때문에 딥링크와 쿼리 파라미터를 조합한 방식을 사용한다고 한다.

### FailureHandler

```java
@Component
public class OAuth2FailureHandler extends SimpleUrlAuthenticationFailureHandler {

  @Override
  public void onAuthenticationFailure(
		  HttpServletRequest request,
		  HttpServletResponse response,
		  AuthenticationException exception
  ) throws IOException, ServletException {
    String errorMessage = URLEncoder.encode(exception.getMessage(), StandardCharsets.UTF_8);

    String redirectUrl = UriComponentsBuilder
        .fromUriString("http://localhost:3000/login")
        .queryParam("error", errorMessage)
        .toUriString();

    getRedirectStrategy().sendRedirect(request, response, redirectUrl);
  }
}
```

### SecurityFilterChain에 등록

각각의 핸들러를 `oauth2Login()`의 `successHandler()`와 `failureHandler()`를 이용해 추가한다. 그리고 SPA로 리다이렉트 되는 REST API 이므로 정적 리소스 관련 인가 설정과 `defaultSuccessUrl()`를 굳이 명시할 이유가 없기 때문에 두 설정을 지우면서 REST API에 맞는 설정을 일부 추가하였다.

```java
@Configuration
@EnableWebSecurity
@RequiredArgsConstructor
public class SecurityConfiguration {

  private final CustomOAuth2UserService customOAuth2UserService;
  private final CustomOidcUserService customOidcUserService;
  private final OAuth2SuccessHandler oAuth2SuccessHandler;
  private final OAuth2FailureHandler oAuth2FailureHandler;

  @Bean
  public SecurityFilterChain securityFilterChain(HttpSecurity http) throws Exception {
    return http
			    // HTTP 헤더를 통한 액세스 토큰 전달 시 사용 (파라미터로 액세스 토큰을 전달)
          // .csrf(AbstractHttpConfigurer::disable)
          // 쿠키 기반 인증 시 사용
					.csrf(csrf -> csrf.
							// HTTP Only가 false인 CSRF 토큰을 쿠키 형태로 프론트엔드에 넘겨줌
							csrfTokenRepository(CookieCsrfTokenRepository.withHttpOnlyFalse())
		          // 필요하면 특정 엔드포인트 CSRF 제외(선택)
		          // .ignoringRequestMatchers("/oauth2/**", "/login/oauth2/**")
					)
          .sessionManagement(session -> session.
              // OAuth2 로그인(Authorization Code) 리다이렉트 왕복 과정에서
					    // state/nonce 등 인가 요청 정보를 저장해야 하므로 세션이 필요할 수 있다.
					    // 따라서 필요한 경우에만 세션을 생성하도록 IF_REQUIRED로 설정한다.
              sessionCreationPolicy(SessionCreationPolicy.IF_REQUIRED)
          )
          .cors(httpSecurityCorsConfigurer -> httpSecurityCorsConfigurer
              .configurationSource(corsConfigurationSource())
          )
          .authorizeHttpRequests(auth -> auth
              .requestMatchers("/oauth2/**", "/login/oauth2/**").permitAll()
              .anyRequest().authenticated()
          )
          .oauth2Login(oauth2 -> oauth2
		          .userInfoEndpoint(userInfo -> userInfo
		              .userService(customOAuth2UserService)
		              .oidcUserService(customOidcUserService)
		          )
		          .successHandler(oAuth2SuccessHandler)
		          .failureHandler(oAuth2FailureHandler)
		          .permitAll()
          )
          .build();
  }

  @Bean
  public CorsConfigurationSource corsConfigurationSource() {
	  CorsConfiguration configuration = new CorsConfiguration();
	  configuration.setAllowedOriginPatterns(Collections.singletonList("http://localhost:3000"));
	  configuration.setAllowedMethods(List.of("GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS"));
	  configuration.setAllowedHeaders(List.of("*"));
	  configuration.setAllowCredentials(true);
	  configuration.setMaxAge(3600L);

	  UrlBasedCorsConfigurationSource source = new UrlBasedCorsConfigurationSource();
	  source.registerCorsConfiguration("/**", configuration);
	  return source;
  }
}
```

참고로 쿠키 기반 인증 방식을 사용하는 경우 프론트엔드에서 쿠키 전송을 위해 axios 라이브러리 기준으로 다음과 같이 설정해줘야 한다고 한다.

```jsx
axios.defaults.withCredentials = true;
```

이 설정은 브라우저가 요청에 쿠키를 포함하도록 하는 옵션이며, CORS 설정에서도 `allowCredentials(true)`가 필요하다. 그리고 CSRF가 활성화된 경우, 프론트엔드는 서버가 내려준 `XSRF-TOKEN` 쿠키 값을 `X-XSRF-TOKEN` 헤더로 함께 전송해야 한다(axios는 기본 설정으로 이를 자동 처리한다).

참고로 액세스 토큰 쿠키는 XSS에 노출되면 치명적이므로 `HttpOnly=true`로 설정한다. 반면 CSRF 방어를 위한 `XSRF-TOKEN`은 프론트가 읽어서 `X-XSRF-TOKEN` 헤더로 보내야 하므로 `HttpOnly=false`로 내려줘야 하는 것이다.

---

쿠키 기반 인증 방식은 CSRF 보안 처리가 관건인 것 같다. 최대한 정확하게 작성하려고 했지만 실제 서비스에서 유효할지는 조금 조심스럽기도 하다. 아무튼 이렇게 스프링 시큐리티를 이용한 자체적인 로그인 시나리오부터 OAuth 2.0 로그인 구현까지 해봤다. 그동안 공부하자고 생각만 했던 것인데 이렇게 공부해서 직접 구현해보니 속이 후련하다.

# 참고 자료

[**Spring Security - Core Configuration**](https://docs.spring.io/spring-security/reference/servlet/oauth2/login/core.html)

[**Spring Security - Advanced Configuration**](https://docs.spring.io/spring-security/reference/servlet/oauth2/login/advanced.html#oauth2login-advanced-map-authorities-oauth2userservice)

[**SameSite 와 CSRF**](https://velog.io/@wpark/SameSite-%EC%99%80-CSRF)
