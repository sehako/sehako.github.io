---
title:  "[Spring] REST API로 소셜 미디어 어플 빌드 - 2"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-27
---

# 게시글 기능 추가

게시글을 관리하기 위해 데이터 클래스를 정의한다. 게시글과 유저는 다대일관계이므로 `User` 클래스를 선언하고 그 관계를 정의해야 한다.

```java
@Entity
public class Post {
    @Id
    @GeneratedValue
    private Integer id;

    @Size(min = 5)
    private String post;

    @ManyToOne(fetch = FetchType.LAZY)
    @JsonIgnore
    private User user;

    // 생성자, getter and setter, toString
}
```

`User` 클래스에도 `Post` 클래스를 선언하고 그 관계가 일대다 관계라는 것을 정의한다.

```java
// User와 Post는 1:다수 관계
@OneToMany(mappedBy = "user")
@JsonIgnore
private List<Post> posts;
```

두 클래스에서 모두 관계에 대한 게터와 세터도 정의해야 한다.

`Post`에 대한 리포지토리를 정의한다.

```java
public interface PostRepository extends JpaRepository<Post, Integer> {

}
```

## 컨트롤러에 메소드 정의

```java
@RestController
public class UserJpaResource {

    private UserRepository userRepository;
    private PostRepository postRepository;
    // 생성자 주입
    public UserJpaResource(UserDaoService service, UserRepository userRepository, PostRepository postRepository) {
        this.userRepository = userRepository;
        this.postRepository = postRepository;
    }

    //...

    @GetMapping(path = "/jpa/users/{id}/posts")
    public List<Post> getPosts(@PathVariable int id) {
        Optional<User> user = userRepository.findById(id);
        if(user.isEmpty()) {
            throw new UserNotFoundException("id: " + id);
        }
        return user.get().getPosts();
    }

    @PostMapping(path = "/jpa/users/{id}/posts")
    public ResponseEntity<Object> createPost(@PathVariable int id, @Valid @RequestBody Post post) {
        Optional<User> user = userRepository.findById(id);
        if(user.isEmpty()) {
            throw new UserNotFoundException("id: " + id);
        }
        post.setUser(user.get());
        Post savedPost = postRepository.save(post);
        // 헤더에 성공 시 포스트 주소를 넣어서 응답
        URI location = ServletUriComponentsBuilder.
                fromCurrentRequest().
                path("/{id}").
                buildAndExpand(savedPost.getId()).toUri();
        return ResponseEntity.created(location).build();
    }
}
```

`getPosts()`에서 눈여겨볼 것은 `postRepository`를 사용하지 않는다는 것이다. `User` 클래스 내부에 정의한 `Post`의 게터를 가지고 사용자가 작성한 글을 불러온다.

[웹 앱 어플리케이션 빌드](https://sehako.github.io/spring/spring-first-web-application-4/#docker%EB%A1%9C-mysql-%EC%97%B0%EA%B2%B0)때 했던 것과 같이 MySQL에 연결할 수도 있다.

# Spring Security

```
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-security</artifactId>
</dependency>
```

```
spring.security.user.name=username
spring.security.user.password=password
```

요청을 보낼 때 마다 스프링 시큐리티가 가로채 필터 체인을 거침

그 중 CSRF가 POST, PUT 요청을 거부

필터 체인을 오버라이딩 하여 POST와 PUT 요청도 받도록 함

```java
@Configuration
public class SpringSecurityConfiguration {
    @Bean
    public SecurityFilterChain securityFilterChain(HttpSecurity httpSecurity) throws Exception {
        httpSecurity.authorizeHttpRequests(
                // 모든 요청은 인증되어야 함
                auth -> auth.anyRequest().authenticated()
        );

        // 인증되지 않은 접근에 대해서 팝업으로 인증을 요구
        httpSecurity.httpBasic(Customizer.withDefaults());

        //csrf 비활성화
//        httpSecurity.csrf().disable();
        return httpSecurity.build();
    }
}
```

csrf 해제는 현제 deprecated된 상태이다.