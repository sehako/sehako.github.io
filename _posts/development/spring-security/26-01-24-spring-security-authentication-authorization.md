---
title: Spring Security를 이용한 인증 / 인가 처리

categories:
  - Spring Security

toc: true
toc_sticky: true
published: true

date: 2026-01-24
last_modified_at: 2026-01-24
---

앞선 포스팅에서 스프링 시큐리티의 아키텍처에 대해 살펴보았다. 구조는 다소 복잡해 보이지만, 핵심은 인증이 완료되면 `SecurityContextHolder`에 `Authentication` 객체가 설정된다는 것이다.

이 개념을 바탕으로, 스프링 시큐리티를 이용해 인증과 인가를 설정하는 방법을 Form 로그인 부터 OAuth2 까지 구현해볼 것이다. 개발 환경은 다음과 같다.

- Java 21
- Spring boot 4.0.0
- JPA
- MySQL (Docker)

# 사전 설정

모든 인증 방법 공통적으로 사용할 것은 바로 사용자 정보 테이블과 엔티티다. 각각 다음과 같이 설정했다.

## 데이터베이스

**Docker 명령어**

```bash
docker run --name mysql-db -e MYSQL_ROOT_PASSWORD=1234 -d -p 3306:3306 mysql:latest
```

**DDL**

```sql
CREATE DATABASE IF NOT EXISTS `security`;

USE `security`;

CREATE TABLE IF NOT EXISTS `user`
(
	id       BIGINT AUTO_INCREMENT PRIMARY KEY,
	username VARCHAR(255) NULL,
	password VARCHAR(255) NULL,
	role     VARCHAR(255) NULL
);
```

정말 간단하게 필수정보인 사용자 ID와 비밀번호, 그리고 역할 정보만 저장하도록 만들었다.

## 애플리케이션 설정

**application.yml**

```yaml
spring:
  application:
    name: spring-security
  datasource:
    url: jdbc:mysql://localhost:3306/security?useSSL=false&serverTimezone=UTC
    username: root
    password: 1234
    driver-class-name: com.mysql.cj.jdbc.Driver
```

**JPA 엔티티**

```java
@Entity
@Table(name = "user")
@Getter
@NoArgsConstructor(access = AccessLevel.PROTECTED)
public class User {
	@Id
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	private Long id;
	private String username;
	private String password;
	private String role;

	@Builder
	private User(String username, String password, String role) {
	  this.username = username;
	  this.password = password;
	  this.role = role;
	}
}
```

**UserRepository**

```java
@Repository
public interface UserRepository extends JpaRepository<User, Long> {
	Optional<User> findByUsername(String username);
}
```

사용자 인증을 제공하기 위해서는 우선 사용자가 애플리케이션에 회원으로 등록할 수 있어야 한다. 그 부분을 구현하기 전에 자체 로그인을 구현하는데 있어서 필수적인 컴포넌트들 부터 짚고 넘어가도록 하자.

# Username/Password 인증 주요 컴포넌트

## PasswordEncoder

`PasswordEncoder`는 스프링 시큐리티에서 비밀번호의 단방향 암호화를 담당하는 인터페이스이다. 다음과 같이 설계되어 있다.

```java
public interface PasswordEncoder {
	@Nullable String encode(@Nullable CharSequence rawPassword);

	boolean matches(@Nullable CharSequence rawPassword, @Nullable String encodedPassword);

	default boolean upgradeEncoding(@Nullable String encodedPassword) {
		return false;
	}
}
```

주요 `PasswordEncoder` 구현체와 특징을 살펴보자. 구현체의 이름은 일반적으로 암호화에 사용된 알고리즘 이름 + `PasswordEncoder`로 구성되어 있다.

- BCryptPasswordEncoder

임의의 솔트를 생성하여 레인보우 테이블 공격 방지 및 해싱 횟수를 조절할 수 있다. 가장 널리 사용되는 방식이다.

- Argon2PasswordEncoder

CPU와 메모리 사용량까지 조절이 가능하여 특수 하드웨어(FPGA, ASIC 등)를 동반한 대규모 무차별 대입 공격을 방어하는데 효과적이다.

- Pbkdf2PasswordEncoder

연방 정보 처리 표준(FIPS)이 요구되는 시스템에서 주로 사용된다. 반복 횟수를 늘려 연산 비용을 높이는 방식으로 보안을 강화했다.

- SCryptPasswordEncoder

많은 양의 메모리를 사용하도록 설계되어, 하드웨어 가속을 이용한 공격을 어렵게 만든다.

참고로 이 외에 다른 구현체들은 안전하지 않다고 판단되어 더 이상 사용되고 있지 않다고 한다.

### DelegatingPasswordEncoder

비밀번호 암호화 관련 빈을 등록할 때는 일반적으로 `PasswordEncoder` 구현체를 직접적으로 다음과 같이 빈으로 등록했다.

```java
@Bean
public PasswordEncoder passwordEncoder() {
  return PasswordEncoderFactories.createDelegatingPasswordEncoder();
}
```

다만 이렇게 등록하게 되면 다음과 같은 문제가 발생한다.

- `PasswordEncoder` 구현체를 변경하기 쉽지 않다.
- 최적의 암호화는 시간이 지나면서 바뀌게 된다.
- 프레임워크는 호환성을 유지해야 하는데, 위와 같은 변경은 호환성을 깨뜨리게 된다.

따라서 스프링 시큐리티는 `DelegatingPasswordEncoder`를 도입했다. 이는 현재 가장 널리 사용되는 암호화 방식을 기본적으로 지원하면서도, 이전의 레거시 암호화 방식도 지원한다. 게다가 향후 새로운 암호화 구현체가 도입되어도 간단하게 업그레이드 할 수 있다.

이것이 가능한 이유는 `DelegatingPasswordEncoder`가 `PasswordEncoderFactories`에 의해서 다음과 같이 구성되기 때문이다.

```java
public final class PasswordEncoderFactories {

	private PasswordEncoderFactories() {}

	@SuppressWarnings("deprecation")
	public static PasswordEncoder createDelegatingPasswordEncoder() {
		String encodingId = "bcrypt";
		Map<String, PasswordEncoder> encoders = new HashMap<>();
		encoders.put(encodingId, new BCryptPasswordEncoder());
		encoders.put("ldap", new org.springframework.security.crypto.password.LdapShaPasswordEncoder());
		encoders.put("MD4", new org.springframework.security.crypto.password.Md4PasswordEncoder());
		// ...
		return new DelegatingPasswordEncoder(encodingId, encoders);
	}
}

```

따라서 요즘에는 다음과 같이 `PasswordEncoder`를 등록하면 된다.

```java
@Bean
public PasswordEncoder passwordEncoder() {
  return PasswordEncoderFactories.createDelegatingPasswordEncoder();
}
```

그리고 이렇게 등록하면, 암호화할 때 어떤 알고리즘으로 암호화되었는지 중괄호 앞에 표시된다.

```
{bcrypt}$2a$10$oQGVQuacv0iJjWcB6B781.SfrvwHMhJiekjbnpxUQYpduUDyxfIiq
```

이후 사용자가 로그인을 시도하면 스프링 시큐리티는 다음과 같이 동작한다.

1. 암호화된 비밀번호 조회
2. `{}`에 표기된 암호화 알고리즘에 해당하는 구현체에 검증 위임
3. 암호문 내에 포함된 솔트(salt)값과 사용자가 입력한 평문을 사용하여 비교 연산 수행 (`matches()`)

## UserDetailsService

사용자 정보에 접근하기 위한 서비스 계층이다. 스프링 시큐리티는 기본적으로 인메모리, JDBC, 캐싱 관련 `UserDetailsService` 구현체를 제공하지만, 일반적으로는 애플리케이션에 적절한 `UserDetailsService` 구현체를 만들어 사용하게 된다.

## UserDetails

`UserDetailsService`에 의해 반환되어서 `DaoAuthenticationProvider`에 의해 검증된다. 이때 검증이 통과하면 이를 기반으로 `Authentication`을 만들어서 반환한다. 이 역시 애플리케이션에 적절한 `UserDetails` 구현체를 만들어서 사용한다.

### CredentialsContainer

사용자 자격 증명을 저장하는 클래스들은 모두 `CredentialsContainer`를 구현할 수 있다. 이는 다음과 같이 이루어진 인터페이스다.

```java
public interface CredentialsContainer {
	void eraseCredentials();
}
```

스프링 시큐리티는 사용자 정보가 캐싱되지 않는다면 이를 무조건적으로 구현하는 것이 좋다고 말한다. 그 이유는 사용자의 비밀번호를 메모리에 유지한다면 메모리 덤프 같은 공격에 취약하기 때문이다.

이때 이 인터페이스를 구현하면 인증이 처리된 직후 사용자의 비밀번호 정보를 지우도록 `AuthenticationManager`가 `eraseCredentials()`를 호출한다. 이를 통해 일관되게 비밀번호를 메모리에 저장하지 않고 관리할 수 있다.

참고로 로그인 시 데이터베이스 접근을 최소화하기 위해서 `UserDetails` 구현체를 캐싱해야 한다면, 민감 정보가 존재하는 객체 상태에서 복제해서 사용하는 것을 권장한다고 한다.

## **DaoAuthenticationProvider**

`AuthenticationProvider` 구현체로, `UserDetailsService`와 `PasswordEncoder`를 사용하여 인증을 처리한다.

![image.png](/assets/images/development/spring-security/26-01-24-spring-security-authentication-authorization/01.png)

1. 사용자가 입력한 정보를 기반으로 `Filter`가 `UsernamePasswordAuthenticationToken`을 생성한다. 해당 토큰은 `AuthenticationManager`(`ProviderManager`)로 전달된다.
2. `ProviderManager`는 `DaoAuthenticationProvider`를 이용해서 인증을 시도한다.
3. `DaoAuthenticationProvider`는 `UserDetailsService`로부터 `UserDetails`를 조회한다.
4. 조회된 `UserDetails`에 대해서 `PasswordEncoder`를 사용해 사용자 비밀번호의 검증을 수행한다.
5. 검증이 성공하면 `Authentication`이 `UsernamePasswordAuthenticationToken` 타입으로 반환되어 `Filter`가 `SecurityContextHolder`에 저장함으로써 사용자 인증이 처리된다.

# 폼(Form) 로그인

스프링을 가장 처음 접하면 기본적으로 서버 사이드 랜더링 애플리케이션 개발 부터 시작할 것이다. 이때 해당 애플리케이션에 스프링 시큐리티를 도입하면 아래의 처리 흐름이 전체 요청에 대해서 활성화 된다.

![image.png](/assets/images/development/spring-security/26-01-24-spring-security-authentication-authorization/02.png)

인증되지 않은 사용자가 접근하면 인증 예외가 발생하고, 스프링 시큐리티에 의해 /login으로 리다이렉트 된다. 이때 별도 설정이 없으면, 스프링 시큐리티에서 제공하는 `/login` URL에 대해서 기본적인 `login.html`을 제공한다.

## 애플리케이션 설정

**build.gradle**

폼 로그인 구현을 위해서는 다음 의존성이 필요하다.

```groovy
dependencies {
	implementation 'org.springframework.boot:spring-boot-starter-security'
	implementation 'org.springframework.boot:spring-boot-starter-webmvc'
	implementation 'org.springframework.boot:spring-boot-starter-thymeleaf'
	implementation 'org.springframework.boot:spring-boot-starter-data-jpa'
	implementation 'com.mysql:mysql-connector-j:9.5.0'
	compileOnly 'org.projectlombok:lombok'
	annotationProcessor 'org.projectlombok:lombok'
	testImplementation 'org.springframework.boot:spring-boot-starter-security-test'
	testImplementation 'org.springframework.boot:spring-boot-starter-webmvc-test'
	testRuntimeOnly 'org.junit.platform:junit-platform-launcher'
}
```

## 회원 가입

회원가입은 간단하게 HTML form 형식으로부터 사용자 ID와 비밀번호, 그리고 가입된 사용자의 역할을 입력받아 데이터베이스에 저장할 것이다. 이때 입력된 사용자의 비밀번호는 `DelegatingPasswordEncoder`에 의해 암호화 되어 데이터베이스에 저장된다.

### 템플릿

**register.html**

```html
<html>
  <head>
    <title>Login</title>
  </head>
  <body>
    <h1>Login</h1>
    <form method="post" th:action="@{/register}">
      <div>
        <label>사용자 ID:</label>
        <input name="username" required type="text" />
      </div>
      <div>
        <label>비밀번호:</label>
        <input name="password" required type="password" />
      </div>
      <div>
        <label>역할</label>
        <select name="role">
          <option value="USER">일반 사용자</option>
          <option value="ADMIN">어드민</option>
        </select>
      </div>
      <button type="submit">가입</button>
    </form>
  </body>
</html>
```

### 컨트롤러

**UserRegisterRequest**

```java
public record UserRegisterRequest(
		String username,
		String password,
		String role
) {

  public User toEntity(String encodedPassword) {
	  return User.builder()
        .username(username)
        .password(encodedPassword)
        .role(role)
        .build();
  }
}
```

**UserController**

```java
@Slf4j
@Controller
@RequiredArgsConstructor
public class UserController {

  private final UserService userService;

  @GetMapping("/register")
  public String register() {
    return "register";
  }

  @PostMapping("/register")
  public String registerPost(
		  @ModelAttribute UserRegisterRequest request
  ) {
    userService.register(request);
    return "redirect:/login";
  }
}
```

회원 가입이 성공적으로 처리되면 `/login` 경로로 바로 리다이렉트 되도록 만들었다.

### 서비스

**UserService**

```java
@Service
@RequiredArgsConstructor
@Transactional
public class UserService {

  private final PasswordEncoder passwordEncoder;

  private final UserRepository userRepository;

  public void register(UserRegisterRequest request) {
    String encodedPassword = passwordEncoder.encode(request.password());
    userRepository.save(request.toEntity(encodedPassword));
  }
}
```

## 로그인

폼 로그인은 앞서 살펴본 대로 관련 `Filter`가 존재하며, 해당 필터가 `DaoAuthenticationProvider`에게 인증을 하도록 되어 있기 때문에 적절한 `SecurityFilterChain`와 `UserDetails`, `UserDetailsService`만 구현하면 된다.

### 시큐리티 컴포넌트 설정

**SecurityConfiguration**

```java
@Configuration
@EnableWebSecurity
public class SecurityConfiguration {

  @Bean
  public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
	  return http
        .sessionManagement(session -> session
            .sessionCreationPolicy(SessionCreationPolicy.IF_REQUIRED)
            .maximumSessions(1) // 하나의 사용자에게 몇 개의 다중 디바이스를 허용할 지 여부
        )
        .formLogin(form -> form
            .loginPage("/login")             // 사용자 정의 로그인 페이지 경로
            .loginProcessingUrl("/login") // 로그인 처리 URL (POST 요청)
            .usernameParameter("username") // 로그인 폼에서 주어지는 사용자 이름 파라미터 값
            .passwordParameter("password") // 로그인 폼에서 주어지는 비밀번호 파라미터 값
            .defaultSuccessUrl("/", true) // 로그인 성공 시 이동할 페이지 true면 항상 여기로 이동
            .failureUrl("/login?error=true")  // 로그인 실패 시 이동할 페이지
            .permitAll()
        )
//        .rememberMe(httpSecurityRememberMeConfigurer -> {}) // 로그인 기억 기능
//        .anonymous(httpSecurityAnonymousConfigurer -> {}) // 익명 로그인 설정
        .logout(logout -> logout
            .logoutUrl("/logout") // 로그아웃 URL 명시 (POST 요청)
            .logoutSuccessUrl("/login") // 로그아웃 성공 시 이동할 페이지
            .permitAll()
        )
        .authorizeHttpRequests(auth -> auth
            .requestMatchers(PathRequest.toStaticResources().atCommonLocations())
            .permitAll() // 정적 리소스는 무조건 통과
            .requestMatchers("/login", "/register")
            .permitAll() // 로그인 페이지는 누구나 접근 가능
            .requestMatchers("/admin").hasAuthority("ADMIN") // ADMIN 인가 설정
            .anyRequest()
            .authenticated() // 그 외 모든 요청은 인증 필요
        )
        .build();
  }

  @Bean
  public PasswordEncoder passwordEncoder() {
    return PasswordEncoderFactories.createDelegatingPasswordEncoder();
  }
}
```

코드에 대한 설명은 주석을 참고하도록 하자. 참고로 `/admin` 페이지에 대한 접근을 설정할 때 `hasRole()`이 아닌 `hasAuthority()`를 사용한 이유는 AI가 최근에는 역할 기반 보다는 권한 기반의 인가 방식이 증가하고 있다고 했기 때문이다. 이에 대한 근거를 다음과 같이 제시했다.

- NIST의 RBAC(역할 기반 접근 제어) 모델이 최근에는 역할과 권한의 관계를 분리하도록 권장한다.
- 최근에는 OAuth2 도입이 필수적이다. 이는 `scope`라는, 스프링 시큐리티에서 `GrantedAuthority`에 대응하는 것인데, `ROLE_`이라는 접두사가 없으므로 권한 기반 검증이 선호된다.
- MSA 환경에서 서비스 간 통신이 중요한데, 각 서비스에서 필요한 최소한의 권한이 필요하다, 따라서 역할 보다는 권한을 토큰에 담아 전달하는 방식이 보안 아키텍처의 정석으로 자리잡았다.

역할과 권한의 분리는 다시 말해서 `hasRole()`과 `hasAuthority()`를 혼용해서 사용하라는 것이다. 이를 위한 예시로 만약 어떤 관리자가 주문과 결제를 관리할 수 있다고 가정해보자. 이때 회사에서 신사업으로 자체 배송 서비스를 만들게 되면서 배송 관리 페이지 또한 만들어졌다.

이러한 상황에서 관리 시스템이 역할 기반으로만 되어 있다면 기존의 관리자도 배송 관리 페이지에 접근하여 어떤 처리를 해도 알 수 없다. 이때 역할과 관계를 분리한다면 관리자라는 역할에서도 주문과 결제만 관리할 수 있는 관리자, 배송만 관리할 수 있는 관리자 이렇게 나눌 수 있다.

이를 설정하는 코드를 살펴보기 위해 `/admin`으로 시작하는 주소의 하위 주소로 `/payment`와 `/shipping`이 있다고 가정해보자. 이때 `/admin`주소는 관리자 간 메신저 역할을 해야 하므로 위 두 관리 페이지를 제외한 다른 모든 페이지는 `ADMIN`이라는 역할만 있으면 접속할 수 있지만, 각 관리 페이지는 접근 주소 + `_MANAGER` 권한이 있어야 한다면 다음과 같이 설정해주면 된다.

```java
.requestMatchers("/admin/payment/**").access(AuthorizationManagers.allOf(
    AuthorityAuthorizationManager.hasRole("ADMIN"),
    AuthorityAuthorizationManager.hasAuthority("PAYMENT_MANAGER")
))
.requestMatchers("/admin/shipping/**").access(AuthorizationManagers.allOf(
    AuthorityAuthorizationManager.hasRole("ADMIN"),
    AuthorityAuthorizationManager.hasAuthority("SHIPPING_MANAGER")
))
.requestMatchers("/admin/**").hasRole("ADMIN")
```

**CustomUserDetails**

```java
@Getter
public class CustomUserDetails implements UserDetails, CredentialsContainer {

  @Serial
  private static final long serialVersionUID = 1L;
  private final Long id;
  private final String username;
  private final Collection<? extends GrantedAuthority> authorities;
  private String password;

  public CustomUserDetails(User user) {
    this.id = user.getId();
    this.username = user.getUsername();
    this.password = user.getPassword();
    this.authorities = List.of(roleToAuthority(user.getRole()));
  }

  private GrantedAuthority roleToAuthority(String role) {
    return new SimpleGrantedAuthority(role);
  }

  @Override
  public Collection<? extends GrantedAuthority> getAuthorities() {
    return authorities;
  }

  @Override
  public String getPassword() {
    return password;
  }

  @Override
  public String getUsername() {
    return username;
  }

  @Override
  public void eraseCredentials() {
    password = null;
  }
}
```

UserDetails에는 이 외에도 계정 및 인증 정보의 상태를 판단하기 위한 4개의 디폴트 메서드가 존재한다. 이 메서드들은 계정 만료, 잠금 여부, 자격 증명 만료 여부, 활성화 여부를 결정하며, 기본 구현은 모두 true를 반환한다. 따라서 해당 정보를 별도로 관리하지 않는다면 굳이 재정의할 필요는 없다.

또한 앞서 언급한 역할(Role)과 권한(Authority)을 분리하는 설계를 적용하려면, `roleToAuthority()`를 `List<GrantedAuthority>` 형태로 반환하도록 구현하여 하나의 사용자 정보가 여러 권한을 가질 수 있도록 구성하면 된다.

마지막으로 `serialVersionUID`는 객체를 직렬화·역직렬화할 때 클래스 구조 변경 여부를 판단하기 위한 직렬화 버전 식별자이다. 객체를 파일, 세션, 캐시 등에 저장한 뒤 다시 읽어올 때, 저장 당시의 클래스와 현재 클래스가 호환되는지를 검증하는 데 사용된다. 특히 세션 기반 인증을 사용하는 애플리케이션에서는 `UserDetails` 구현체가 세션에 저장될 수 있으므로, `serialVersionUID`를 명시하는 것이 사실상 필수에 가깝다.

**CustomUserDetailsService**

```java
@Service
@RequiredArgsConstructor
public class CustomUserDetailsService implements UserDetailsService {

  private final UserRepository userRepository;

  @Transactional(readOnly = true)
  @Override
  public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
    User user = userRepository.findByUsername(username)
            .orElseThrow(() -> new UsernameNotFoundException("User not found with username: " + username));

    return new CustomUserDetails(user);
  }
}
```

이렇게 구성하면 미인증된 사용자가 서버에 접근할 경우, 스프링 시큐리티에서 기본적으로 제공하는 HTML로 리다이렉트되어 인증을 요구한다. 이때 애플리케이션에 맞는 HTML로 리다이렉트되도록 하려면 앞서 `formLogin()`을 설정할 때 명시했던 `loginPage()`에 해당하는 주소에 대해서 별도 HTML을 반환하는 컨트롤러를 정의하면 된다.

### 템플릿

**login.html**

```html
<html>
  <head>
    <title>Login</title>
  </head>
  <body>
    <h1>Login</h1>
    <form method="post" th:action="@{/login}">
      <div>
        <label>사용자 ID:</label>
        <input name="username" required type="text" />
      </div>
      <div>
        <label>비밀번호:</label>
        <input name="password" required type="password" />
      </div>
      <div th:if="${param.error}">
        <p style="color: red;">아이디 또는 비밀번호가 올바르지 않습니다.</p>
      </div>
      <button type="submit">로그인</button>
      <a th:href="@{/register}">회원가입</a>
    </form>
  </body>
</html>
```

이때 Thymeleaf에서 지원하는 기능인 `th:action`을 사용한 이유는 CSRF 공격 방지를 위한 토큰 처리 때문이다. 서버 사이드 랜더링 애플리케이션에서는 CSRF 공격에 취약하다.

그 이유는 세션이 동작하는 원리가 애플리케이션 내부적으로 사이트의 JSESSIONID 쿠키 문자열을 키로 하기 때문이다. 따라서 어떤 애플리케이션에 인증된 상태에서, 악성 사이트를 클릭했을 때 애플리케이션 주소로 어떤 처리를 하도록 요청을 보낸다면 브라우저는 쿠키에 JSESSIONID를 동봉해 요청하기 때문에 서버는 해당 요청이 악의적 요청인지 알 수 없다.

이 공격에 대해서 스프링 시큐리티는 CSRF 토큰을 도입했다. 이는 사용자 세션에 임의의 값을 저장하여 모든 요청마다 해당 값을 포함해 전송하도록 유도한다. 이후 서버에서 요청을 받을때마다, 세션에 저장된 값과 요청으로 전송된 값을 검증하여 방어하는 방법이다.

여기서 일반적인 `action`을 사용하면 개발자가 직접 `${_csrf.token}`을 꺼내서 `hidden` 속성을 가진 `<input>`을 만들어야 한다. 하지만 `th:action`을 사용하면 타임리프가 이를 감지하여 자동으로 토큰 필드를 삽입해준다.

### 컨트롤러

**UserController**

```java
@Slf4j
@Controller
@RequiredArgsConstructor
public class UserController {
  // ...
  @GetMapping("/login")
  public String login() {
    return "login";
  }
}
```

# REST API + JWT

일반적으로 REST API는 로그인이 성공했을 때 사용자 정보를 서버의 세션에 저장하는 것이 아니라, 클라이언트에게 JWT를 발급하고 이후 HTTP 요청 헤더에 `Authorization`으로 JWT 정보를 첨부하는 구조를 가지게 된다. 따라서 인증이 성공하면 JWT를 발급받는 것과, 이후 헤더에 JWT가 전달되면 이를 기반으로 `SecurityContextHolder`에 `Authentication`을 저장할 `AuthenticationManager`를 만들어야 한다.

## 애플리케이션 설정

**build.gradle**

JWT를 발급하고 파싱하기 위해서 JJWT 의존성을 사용한다.

```groovy
dependencies {
  implementation 'org.springframework.boot:spring-boot-starter-security'
  implementation 'org.springframework.boot:spring-boot-starter-web'
  implementation 'org.springframework.boot:spring-boot-starter-data-jpa'
  implementation 'com.mysql:mysql-connector-j:9.5.0'
  implementation 'io.jsonwebtoken:jjwt-api:0.13.0'
  runtimeOnly 'io.jsonwebtoken:jjwt-impl:0.13.0'
  runtimeOnly 'io.jsonwebtoken:jjwt-jackson:0.13.0'
  compileOnly 'org.projectlombok:lombok'
  annotationProcessor 'org.projectlombok:lombok'
  testImplementation 'org.springframework.boot:spring-boot-starter-security-test'
  testImplementation 'org.springframework.boot:spring-boot-starter-web-test'
  testRuntimeOnly 'org.junit.platform:junit-platform-launcher'
}
```

**application.yml**

JWT의 시그니처와 엑세스 및 리프레시 토큰 만료 시간 설정을 위해 다음 설정을 추가한다.

```yaml
jwt:
	secret: ${JWT_SECRET}
	access-token-expiration: 86400000
	refresh-token-expiration: 604800000
```

## 회원가입

회원가입은 앞서 진행한 폼 로그인과 마찬가지로 사용자의 가입 요청을 받으면 비밀번호를 암호화하여 데이터베이스에 저장한다.

### 컨트롤러

**UserRegisterRequest**

```java
public record UserRegisterRequest(
		String username,
		String password,
		String role
) {
  public User toEntity(String encodedPassword) {
	  return User.builder()
        .username(username)
        .password(encodedPassword)
        .role(role)
        .build();
  }
}
```

**UserController**

```java
@RestController
@RequiredArgsConstructor
public class UserController {

  private final UserService userService;

  @PostMapping("/register")
  public ResponseEntity<Void> registerUser(
		  @RequestBody UserRegisterRequest request
  ) {
    userService.registerUser(request);
    return ResponseEntity.ok().build();
  }
}
```

### 서비스

**UserService**

```java
@Slf4j
@Service
@RequiredArgsConstructor
@Transactional
public class UserService {

  private final UserRepository userRepository;
  private final PasswordEncoder passwordEncoder;

  public void registerUser(UserRegisterRequest request) {
    String encodedPassword = passwordEncoder.encode(request.password());
    userRepository.save(request.toEntity(encodedPassword));
  }
}
```

## 로그인

인증이 성공하면 사용자에게 JWT를 반환할 수 있어야 한다. 이를 위해서 JWT를 발급하고, 파싱할 수 있는 클래스를 만들어야 한다.

### JWT 토큰 발급

**TokenInformation**

```java
public record TokenInformation(
    String accessToken,
    String refreshToken
) {
}
```

**JwtProvider**

```java
@Component
public class JwtProvider {

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

	  if (!(authentication.getPrincipal() instanceof CustomUserDetails userDetails)) {
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
}
```

전체적으로 이전에 [스프링 시큐리티 없이 구현했던 OAuth2](https://velog.io/@gkrdh99/series/OAuth)와 비슷하지만, 인증 성공 이후 `Authentication` 객체로부터 JWT를 발급한다는 차이점이 있다. 토큰의 페이로드에는 사용자 ID, 이름, 역할 정보가 저장된다.

### 시큐리티 컴포넌트 설정

폼 로그인에서 사용한 `UserDetails`와 `UserDetailsService`를 비교했을 때, 다른 부분은 없기 때문에 굳이 보지 않아도 된다.

**CustomUserDetails**

```java
@Getter
public class CustomUserDetails implements UserDetails, CredentialsContainer {
  private static final long serialVersionUID = 1L;
  private final Long id;
  private final String username;
  private final String role;
  private String password;

  public CustomUserDetails(User user) {
    id = user.getId();
    username = user.getUsername();
    password = user.getPassword();
    role = user.getRole();
  }

  @Override
  public Collection<? extends GrantedAuthority> getAuthorities() {
    return List.of(new SimpleGrantedAuthority(role));
  }

  @Override
  public @Nullable String getPassword() {
    return password;
  }

  @Override
  public String getUsername() {
    return username;
  }

  @Override
  public void eraseCredentials() {
    password = null;
  }
}
```

**CustomUserDetailsService**

```java
@Service
@RequiredArgsConstructor
public class CustomUserDetailsService implements UserDetailsService {

  private final UserRepository userRepository;

  @Override
  @Transactional(readOnly = true)
  public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
	  User loginUser = userRepository.findByUsername(username)
	          .orElseThrow(() -> new UsernameNotFoundException("User not found with username: " + username));

    return new CustomUserDetails(loginUser);
  }
}
```

### 컨트롤러

**UserLoginRequest**

```java
public record UserLoginRequest(
    String username,
    String password
) {}
```

**UserController**

```java
@RestController
@RequiredArgsConstructor
public class UserController {

  private final UserService userService;

	// ...

  @PostMapping("/login")
  public ResponseEntity<TokenInformation> loginUser(
		  @RequestBody UserLoginRequest request
  ) {
    return ResponseEntity
        .ok(userService.login(request));
  }
}
```

구현의 단순함을 위해서 엑세스 토큰과 리프레시 토큰을 반환하도록 만들었다. 해당 포스팅은 스프링 시큐리티를 이용한 인증 / 인가 구축에만 초점을 다루기 때문에 JWT 관련 처리는 하나도 하지 않도록 하겠다.

### 서비스

여기서 사용자 ID와 비밀번호를 가지고 인증을 시도하므로 별도의 인증 절차를 만드는 대신에 폼 로그인의 동작 과정을 일부 빌릴 것이다. `DaoAuthenticationProvider`를 이용해 인증 처리를 한 다음에 반환받은 `Authentication` 객체를 바탕으로 JWT를 발급받도록 하자. 이를 위해서 우선 `AuthenticationManager`를 스프링 빈으로 등록해야 한다.

```java
@Bean
public AuthenticationManager authenticationManager(
		AuthenticationConfiguration authenticationConfiguration
) throws Exception {
  return authenticationConfiguration.getAuthenticationManager();
}
```

**UserService**

```java
@Slf4j
@Service
@RequiredArgsConstructor
@Transactional
public class UserService {

  private final UserRepository userRepository;
  private final PasswordEncoder passwordEncoder;
  private final AuthenticationManager authenticationManager;
  private final JwtProvider jwtProvider;

  // ...

  public TokenInformation login(UserLoginRequest request) {
	  UsernamePasswordAuthenticationToken authenticationToken
        = new UsernamePasswordAuthenticationToken(request.username(), request.password());

	  Authentication authentication = authenticationManager.authenticate(authenticationToken);

	  return jwtProvider.generateToken(authentication);
  }
}
```

## JWT 인증 / 인가

JWT를 사용하게 되면 헤더에 전달되는 엑세스 토큰 정보를 바탕으로 사용자 정보를 얻게 된다. 매 요청마다 헤더에 전달되는 JWT를 파싱하기 위해서 `JwtProvider`에 다음 코드를 추가한다.

```java
@Component
public class JwtProvider {

  private static final String ID_CLAIM = "id";
  private static final String USERNAME_CLAIM = "username";
  private static final String ROLE_CLAIM = "role";

	// ...

  public UserDetails getUserDetails(String token) {
    Claims claims = parseToken(token);

    return new CustomUserDetails(createAuthenticatedUser(claims));
  }

  private Claims parseToken(String token) {
      try {
        return Jwts.parser()
            .verifyWith(secretKey)
            .build()
            .parseSignedClaims(token)
            .getPayload();
      } catch (UnsupportedJwtException | IllegalArgumentException e) {
        throw new RuntimeException("Invalid JWT token", e);
      }
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

사용자 헤더로부터 JWT를 조회하여서 `SecurityContextHolder`에 `Authentication`을 조회하는 방법은 두 가지가 존재한다. 하나는 굉장히 쉽지만 스프링 시큐리티의 설계 철학을 일부 위배하는 것이며, 다른 하나는 스프링 시큐리티의 설계 철학을 따르지만 이를 위한 코드가 굉장히 많다. 하나씩 알아보도록 해보자.

### **OncePerRequestFilter 상속**

모든 요청마다 한 번씩 실행되는 필터에서 `Authorization` 헤더의 JWT를 추출하고, `JwtProvider`를 통해 인증 객체(`Authentication`)를 얻어 `SecurityContextHolder`에 저장하는 방식이다. 구현이 매우 직관적이고 간편하지만, 스프링 시큐리티의 `AuthenticationManager`를 거치지 않기 때문에 프레임워크가 설계한 표준 인증 흐름을 우회하는 것이다. 코드를 보도록 하자.

```java
@RequiredArgsConstructor
public class JwtAuthenticationFilter extends OncePerRequestFilter {

  private static final String AUTHORIZATION_HEADER = "Authorization";

  private final JwtProvider jwtProvider;

  @Override
  protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain chain)
          throws ServletException, IOException {

    String token = resolveToken(request);

    if (token != null) {
	    UserDetails userDetails = jwtProvider.getUserDetails(token);
	    SecurityContextHolder.getContext().setAuthentication(
	            new UsernamePasswordAuthenticationToken(userDetails, null, userDetails.getAuthorities())
	    );
    }
    chain.doFilter(request, response);
  }

  private String resolveToken(HttpServletRequest request) {
      return request.getHeader(AUTHORIZATION_HEADER);
  }
}
```

이를 `SecurityFilterChain` 구성 빈에 다음과 같이 등록하기만 하면 된다.

```java
@EnableWebSecurity
@RequiredArgsConstructor
public class SecurityConfiguration {

  private final JwtProvider jwtTokenProvider;

	@Bean
	public SecurityFilterChain filterChain(HttpSecurity http) {
	  return http
			  // ...
	      .addFilterBefore(
			      new JwtAuthenticationFilter(jwtTokenProvider),
	          UsernamePasswordAuthenticationFilter.class
	      )
	      .build();
	}
}
```

### AbstractAuthenticationProcessingFilter 상속

스프링 시큐리티의 기본 필터인 `UsernamePasswordAuthenticationFilter`와 유사하게 구현하는 방법이다. 필터는 인증 요청(인증 토큰)을 생성하여 `AuthenticationManager`에게 전달하고, 실제 인증은 `AuthenticationProvider`가 담당한다. 이후 인증이 완료되면 인증 토큰이 인증된 상태로 변경되어 반환되고, 내부적으로 `SecurityContextHolder`에 저장된다.

이 방식은 스프링 시큐리티의 확장 포인트를 모두 활용하는 모범 사례이며, 향후 다른 인증 수단이 추가되어도 유연하게 대응할 수 있다. 다만, 앞선 필터 방식에 비해 구현해야 할 클래스와 설정 코드가 상당히 많아진다는 것이 단점이다.

특히 이 방식을 사용하면 스프링 시큐리티가 자동으로 구성해주던 `DaoAuthenticationProvider`를 더 이상 사용할 수 없기 때문에 직접 빈으로 등록해야 한다.

**JwtAuthenticationToken**

```java
public class JwtAuthenticationToken extends AbstractAuthenticationToken {

  private final Object principal;
  private Object credentials;

  // 인증 전 생성자 호출
  public JwtAuthenticationToken(String token) {
    super(Collections.emptyList());
    this.credentials = token;
    this.principal = null;
    setAuthenticated(false);
  }

  // 인증 후 생성자 호출
  public JwtAuthenticationToken(
		  Object principal,
		  Object credentials,
		  Collection<? extends GrantedAuthority> authorities
  ) {
	  super(authorities);
	  this.principal = principal;
	  this.credentials = credentials;
	  super.setAuthenticated(true);
  }

  @Override
  public @Nullable Object getCredentials() {
    return credentials;
  }

  @Override
  public @Nullable Object getPrincipal() {
    return principal;
  }
}
```

JWT를 기반으로 인증을 수행하기 위해서는 우선 `UsernamePasswordAuthenticationToken`처럼 인증 토큰을 만들어야 한다. 인증이 통과되기 전 호출되는 생성자와 인증이 통과된 후 호출되는 생성자가 다르다는 것에 유의하도록 하자.

**JwtAuthenticationProvider**

```java
@RequiredArgsConstructor
public class JwtAuthenticationProvider implements AuthenticationProvider {

  private final JwtProvider jwtProvider;

  @Override
  public @Nullable Authentication authenticate(Authentication authentication) throws AuthenticationException {
    String token = (String) authentication.getCredentials();

    try {
	    UserDetails userDetails = jwtProvider.getUserDetails(token);
	    return new JwtAuthenticationToken(userDetails, null, userDetails.getAuthorities());
    } catch (RuntimeException e) {
      throw new BadCredentialsException("Invalid JWT token", e);
    }
  }

  @Override
  public boolean supports(Class<?> authentication) {
    return JwtAuthenticationToken.class.isAssignableFrom(authentication);
  }
}

```

필터에 의해 호출되는 클래스로, JWT를 파싱하여 `Authentication` 객체를 반환한다.

**JwtAuthenticationFilter**

```java
public class JwtAuthenticationFilter extends AbstractAuthenticationProcessingFilter {

  private static final String AUTH_HEADER = "Authorization";
  private static final String TOKEN_PREFIX = "Bearer ";

  public JwtAuthenticationFilter(AuthenticationManager authenticationManager) {
    super(request -> request.getHeader(AUTH_HEADER) != null);
    setAuthenticationManager(authenticationManager);
  }

  @Override
  public Authentication attemptAuthentication(HttpServletRequest request, HttpServletResponse response)
      throws AuthenticationException, IOException {

    String token = request.getHeader(AUTH_HEADER);

    if (token == null || !token.startsWith(TOKEN_PREFIX)) {
      throw new InsufficientAuthenticationException("Token not found");
    }

    String jwt = token.substring(TOKEN_PREFIX.length());

    JwtAuthenticationToken authRequest = new JwtAuthenticationToken(jwt);

    return this.getAuthenticationManager().authenticate(authRequest);
  }

  @Override
  protected void successfulAuthentication(HttpServletRequest request, HttpServletResponse response,
                                          FilterChain chain, Authentication authResult)
      throws IOException, ServletException {

    SecurityContextHolder.getContext().setAuthentication(authResult);
    chain.doFilter(request, response);
  }
}
```

`super(request -> request.getHeader(AUTH_HEADER) != null)`는 해당 필터가 동작하기 위한 조건이다. `JwtAuthenticationFilter`는 사용자가 인증이 완료된 이후 전달받은 엑세스 토큰이 `Authorization` 헤더에 있는 경우에 동작해야 하므로 `Authorization` 헤더를 조회했을 때 `null`이 아닌 경우에 동작하도록 설정했다.

인증이 성공적으로 처리되면 이후 `successfulAuthentication`이 호출되어 최종적으로 `SecurityContextHolder`에 사용자 정보가 저장된다. 최종적으로 등록된 필터를 `SecurityFilterChain` 구성 빈에 다음과 같이 등록하면 된다.

```java
@Bean
public SecurityFilterChain filterChain(HttpSecurity http, AuthenticationManager authenticationManager) {
  return http
      // ...
      .addFilterBefore(new JwtAuthenticationFilter(authenticationManager),
              UsernamePasswordAuthenticationFilter.class)
      .build();
}
```

**AuthenticationManager 직접 등록**

해당 구조를 사용하게 되면 `DaoAuthenticationProvider`와 `JwtAuthenticationProvider`가 사용된다. 이때 스프링 시큐리티는 사용자가 직접적으로 `AuthenticationProvider` 타입의 빈을 등록하면 인증 제공자들을 더 이상 자동으로 등록해주지 않게된다.

따라서 다음과 같이 직접적으로 `DaoAuthenticationProvider`와 `JwtAuthenticationProvider`를 가지고 있는`AuthenticationManager`를 빈으로 등록해야 한다.

```java
@EnableWebSecurity
@RequiredArgsConstructor
public class SecurityConfiguration {

	private final JwtProvider jwtTokenProvider;

  @Bean
  public PasswordEncoder passwordEncoder() {
    return PasswordEncoderFactories.createDelegatingPasswordEncoder();
  }

  @Bean
  public AuthenticationManager authenticationManager(
		  DaoAuthenticationProvider daoAuthenticationProvider,
		  JwtAuthenticationProvider jwtAuthenticationProvider
  ) {
    return new ProviderManager(
	      daoAuthenticationProvider,
	      jwtAuthenticationProvider
    );
  }

  @Bean
  public DaoAuthenticationProvider daoAuthenticationProvider(
      CustomUserDetailsService userDetailsService
  ) {
	  DaoAuthenticationProvider provider
	      = new DaoAuthenticationProvider(userDetailsService);
    provider.setPasswordEncoder(passwordEncoder());
    return provider;
  }

  @Bean
  public JwtAuthenticationProvider jwtAuthenticationProvider() {
    return new JwtAuthenticationProvider(jwtTokenProvider);
  }
}
```

## SecurityFilterChain 구성 코드

`SecurityFilterChain`을 구성하는 코드를 넣을만한 섹션이 없어서 전체 코드형식으로 마지막에 첨부한다.

```java
@Configuration
@EnableWebSecurity
@RequiredArgsConstructor
public class SecurityConfiguration {

  private final JwtProvider jwtTokenProvider;

  @Bean
  public SecurityFilterChain filterChain(HttpSecurity http, AuthenticationManager authenticationManager) {
	  return http
        .csrf(AbstractHttpConfigurer::disable) // REST API는 불필요
        .httpBasic(AbstractHttpConfigurer::disable)
        .formLogin(AbstractHttpConfigurer::disable)
        .cors(cors ->
            cors.configurationSource(corsConfigurationSource()) // CORS 설정
        )
        .sessionManagement(session -> // JWT를 사용하므로 세션은 무상태를 가진다.
            session.sessionCreationPolicy(SessionCreationPolicy.STATELESS)
        )
        .authorizeHttpRequests(auth -> auth // 인가 설정
                .requestMatchers("/login", "/register").permitAll()
                .requestMatchers("/admin/**").hasAuthority("ADMIN")
                .requestMatchers("/**").authenticated())
        .addFilterBefore(new JwtAuthenticationFilter(authenticationManager),
                UsernamePasswordAuthenticationFilter.class)
        .build();
  }

  @Bean
  public CorsConfigurationSource corsConfigurationSource() {
	  CorsConfiguration configuration = new CorsConfiguration();
	  configuration.setAllowedOriginPatterns(Collections.singletonList("sample_url"));
	  configuration.setAllowedMethods(List.of("GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS"));
	  configuration.setAllowedHeaders(List.of("*"));
	  configuration.setAllowCredentials(true);
	  configuration.setMaxAge(3600L);

	  UrlBasedCorsConfigurationSource source = new UrlBasedCorsConfigurationSource();
	  source.registerCorsConfiguration("/**", configuration);
	  return source;
  }

  @Bean
  public PasswordEncoder passwordEncoder() {
    return PasswordEncoderFactories.createDelegatingPasswordEncoder();
  }

  @Bean
  public AuthenticationManager authenticationManager(
		  DaoAuthenticationProvider daoAuthenticationProvider,
		  JwtAuthenticationProvider jwtAuthenticationProvider
  ) {
	  return new ProviderManager(
		    daoAuthenticationProvider,
		    jwtAuthenticationProvider
	  );
  }

  @Bean
  public DaoAuthenticationProvider daoAuthenticationProvider(
		  CustomUserDetailsService userDetailsService
  ) {
	  DaoAuthenticationProvider provider
	          = new DaoAuthenticationProvider(userDetailsService);
	  provider.setPasswordEncoder(passwordEncoder());
	  return provider;
  }

  @Bean
  public JwtAuthenticationProvider jwtAuthenticationProvider() {
    return new JwtAuthenticationProvider(jwtTokenProvider);
  }
}
```

해당 코드는 `AbstractAuthenticationProcessingFilter`를 상속한 JWT 파싱 필터가 존재하는 스프링 시큐리티 설정 코드다.

# 인증 / 인가 후 사용자 정보 가져오기

폼 로그인과 REST API에서 각각 세션과 JWT에 사용자 정보가 담기도록 구현했다. 추가적으로 JWT는 매번 헤더에 담겨서 오는 값을 기반으로 요청에 대한 `Authentication`을 만들어서 `SecurityContextHolder`에 담았다.

이 정보를 사용하기 위해서는 간단하게 다음과 같이 호출하면 된다.

```java
SecurityContextHolder.getContext().getAuthentication().getPrincipal();
```

이렇게 호출하면 현재 요청에서 인증된 사용자 정보를 가지고 올 수 있다. 하지만 사용자 정보가 필요할때마다 매번 저 코드를 호출해야 하는 건 너무 번거로울 것이다. 따라서 스프링 시큐리티는 `@AuthenticationPrincipal` 이라는 어노테이션을 지원한다.

해당 어노테이션은 파라미터 수준에서 동작하는데, 내부적으로 `SecurityContextHolder`에 접근하여 위와 같은 코드를 호출한다. 따라서 사용자 정보가 필요한 컨트롤러에 다음과 같이 작성하면 된다.

```java
@GetMapping("/")
public ResponseEntity<UserInfoResponse> home(
		@AuthenticationPrincipal CustomUserDetails userInfo
) {}
```

이 과정에서 `Authentication.getPrincipal()`에 저장된 객체가 파라미터 타입에 맞게 바인딩된다.

## 애플리케이션에 적절한 객체로 변환

만약에 애플리케이션에서 사용하는 엔티티 같은 객체로 변환시키고자 한다면 어노테이션과 `ArgumentResolver`를 활용하여 다음과 같이 만들 수도 있다.

**AuthenticationUser 어노테이션**

```java
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.PARAMETER)
public @interface AuthenticationUser {
}
```

**AuthenticationUserArgumentResolver**

```java
@Slf4j
public class AuthenticationUserArgumentResolver implements HandlerMethodArgumentResolver {
  @Override
  public boolean supportsParameter(MethodParameter parameter) {
    boolean hasAuthenticationUser = parameter.hasParameterAnnotation(AuthenticationUser.class);
    boolean hasUserEntity = User.class.isAssignableFrom(parameter.getParameterType());
    log.info("hasAuthenticationUser: {}, hasUserEntity: {}", hasAuthenticationUser, hasUserEntity);

    return hasAuthenticationUser && hasUserEntity;
  }

  @Override
  public @Nullable Object resolveArgument(
		  MethodParameter parameter,
		  @Nullable ModelAndViewContainer mavContainer,
		  NativeWebRequest webRequest,
		  @Nullable WebDataBinderFactory binderFactory
	) throws Exception {

    CustomUserDetails userDetails = (CustomUserDetails) SecurityContextHolder
        .getContext()
        .getAuthentication()
        .getPrincipal();

    if (userDetails == null) {
      throw new RuntimeException("User is not authenticated");
    }

    return User.builder()
        .id(userDetails.getId())
        .username(userDetails.getUsername())
        .role(userDetails.getRole())
        .build();
  }
}
```

**WebConfiguration**

```java
@Configuration
public class WebConfiguration implements WebMvcConfigurer {
  @Override
  public void addArgumentResolvers(List<HandlerMethodArgumentResolver> resolvers) {
    resolvers.add(new AuthenticationUserArgumentResolver());
  }
}
```

이렇게 구현하면 컨트롤러 메서드에 `@AuthenticationUser` 어노테이션이 명시된 `User` 엔티티가 자동으로 만들어진다.

**HomeController**

```java
@GetMapping("/")
public ResponseEntity<UserInfoResponse> home(
		@AuthenticationUser User user
) {}
```

---

최대한 스프링 시큐리티가 권장하는 방법으로 인증 / 인가를 구현하려고 공식문서를 많이 찾아봤다. 폼 로그인 부분은 로그인 기억하기 기능이나 익명 로그인 기능도 있지만, 최근에는 REST API 개발을 주로 다루기 때문에 코드에 주석으로만 남기고 넘어갔다. 다음 포스팅은 OAuth2를 구현해볼 것이다.

# 참고자료

[**Spring Security - Password Storage**](https://docs.spring.io/spring-security/reference/features/authentication/password-storage.html)

[**Spring Security - UserDetailsService**](https://docs.spring.io/spring-security/reference/servlet/authentication/passwords/user-details-service.html)

[**Spring Security - UserDetails**](https://docs.spring.io/spring-security/reference/servlet/authentication/passwords/user-details.html)

[**Spring Security - DaoAuthenticationProvider**](https://docs.spring.io/spring-security/reference/servlet/authentication/passwords/dao-authentication-provider.html)

[**Spring Security - Form Login**](https://docs.spring.io/spring-security/reference/7.0/servlet/authentication/passwords/form.html)
