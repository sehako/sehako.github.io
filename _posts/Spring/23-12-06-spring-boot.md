---
title:  "[Spring] Spring Boot"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-06
---

# 스프링 부트 이전의 앱 빌드

스프링 부트 이전의 앱 빌드는 다음 4가지 과정을 거쳤다.

1. 의존성 관리(pom.xml)
2. 웹 앱 설정 정의(web.xml)
3. 스프링 빈 관리(context.xml)
4. 비 기능적 요구 구현(NFRs)

# 스프링 부트

스프링 부트는 프로덕션 환경에서 사용 가능한(PRODUCTION-READY) 앱을 빠르게(QUICKLY) 구축하는 것을 목표로 한다.

빠른 어플리케이션 빌드를 도와주는 몇몇 기능이 있다.

- [Spring Initializer](https://start.spring.io/)
- Spring Boot Starter Projects
- Spring Boot Auto Configuration
- Spring Boot DevTools

PRODUCTION-READY 상태가 제공해야 하는 기능은 다음과 같다.

- 로깅
- 다양한 환경에서의 다른 설정 제공(Profiles, ConfigurationProperties)
- 모니터링(Spring Boot Actuator)

## 예시

예를 들어 Spring Initializer를 통해 구축한 프로젝트에서 REST API를 작성하고 그를 통해 간단한 데이터를 보여주는 과정은 다음과 같다.

1. 데이터 클래스 작성

**Information.java**
```java
package com.in28minutes.springboot.learnspringboot;

public class Information {
    private long id;
    private String name;
    private String author;

    public Information(long id, String name, String author) {
        this.id = id;
        this.name = name;
        this.author = author;
    }

    public long getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public String getAuthor() {
        return author;
    }
}
```

2. REST API 작성

```java
package com.in28minutes.springboot.learnspringboot;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Arrays;
import java.util.List;

//REST API
@RestController
public class CourseController {

    // /info로 매핑
    @RequestMapping("/info")
    public List<Information> getInfo() {
        return Arrays.asList(
                new Information (1, "Spring Boot", "A"),
                new Information (2, "AWS", "B")
        );
    }
}
```

```java
// 메인 메소드
@SpringBootApplication
public class LearnSpringBootApplication {
	public static void main(String[] args) {
		SpringApplication.run(LearnSpringBootApplication.class, args);
	}
}
```

## Quickly

### Spring Boot Starter Projects

하나의 어플리캐이션을 빌드할 때 수 많은 프레임워크가 필요하다. 예를 들어 REST API를 빌드하려면 Spring, Spring MVC, Tomcat, JSON 변환기 등이 필요하다. 스타터는 이런 다양한 프레임워크를 통합 및 빌드하기 위해 편리한 의존성 설명서를 제공한다.

- 웹 앱 & REST API: Spring Boot Starter Web
- 유닛 테스트: Spring Boot Starter Test
- JPA를 사용하는 DB: Spring Boot Starter Data JPA
- JDBC를 사용하는 DB: Spring Boot Starter JDBC
- 보안: Spring Boot Starter Security

### Spring Boot Auto Configuration

스프링 앱을 빌드할 때 많은 설정을 필요로 한다. 이런 과정을 간소화하기 위해 자동화된 설정을 지원한다.

기본적으로 다음 규칙을 따른다.

- 클래스 경로에 있는 프레임워크에 따라서 생성
- 개발자가 임의로 작성한 설정을 참조하여 오버라이드

### Spring Boot DevTools

**pom.xml**에 다음 의존성 추가

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-devtools</artifactId>
</dependency>
```

추가로 인텔리제이는 다음 설정을 해줘야 한다. [유튜브 링크](https://www.youtube.com/watch?v=BvIM9gNTDM4&ab_channel=coder4life)

매 프로젝트의 변화마다 어플리케이션을 자동으로 다시 빌드한다.

## Production-Ready

### 로깅

로깅은 다음 프로퍼티들을 작성하여 개발자가 원하는 로그를 볼 수 있다.

**application.properties**

```
logging.level.org.springframework=debug
```

**application-prod.properties**

```
logging.level.org.springframework=info
```

**application-dev.properties**

```
logging.level.org.springframework=trace
```

이 경우 개발자가 따로 프로필을 지정해주지 않았기 때문에 자동적으로 우선순위가 높은 **application.properties**를 참조한다. 따라서 해당 파일에 다음 문장을 추가한다.

```
spring.profiles.active=prod
```

이러면 **application-prod.properties** 파일을 참조하여 설정된 값에 따라 로깅을 수행한다.

### ConfigurationProperties

**application.properties**에 값을 정의하고 그 값을 참조할 수 있다. 

예를들어, 임의의 경로, 사용자 이름, 키 값을 정의하고자 한다면 다음과 같이 입력하면 된다.

```
example.url=https://test.com
example.username=defaultusername
example.key=defaultkey
```

그 후 데이터 클래스와 REST API를 구축한다.

```java
// 어노테이션으로 properties에 작성된 키워드에 접근
@ConfigurationProperties(prefix = "example")
@Component
public class Example {
    private String url;
    private String username;
    private String key;

    public String getUrl() {
        return url;
    }

    public String getUsername() {
        return username;
    }

    public String getKey() {
        return key;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public void setUsername(String username) {
        this.username = username;
    }

    public void setKey(String key) {
        this.key = key;
    }
}
```

```java
@RestController
public class ExampleController {
    @Autowired
    private Example example;

    @RequestMapping("/example")
    public Example getExample() {
        return example;
    }
}
```

이 방법은 모든 어플리케이션 관련 설정을 위한 중앙집중화된 클래스 역할을 수행한다. 이를 통해 어플리케이션에 필요한 모든 설정을 외부화 할 수 있다.

### Embedded Servers

배포 과정을 간략화 하기 위해 JAR 파일을 배포, 이 경우 사용자는 자바만 있으면 실행이 가능

### Spring Boot Actuator

프로덕션 환경의 앱을 모니터링 하는 방법

**pom.xml**에 다음 의존성 추가

```xml
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-actuator</artifactId>
</dependency>
```

이 다음 앱을 실행하고 `접근 주소/actuator`에 접근하면 여러 앤드포인트가 나온다. **application.properties**에 수집할 여러 정보(앤드포인트)를 작성할 수 있다.

```
management.endpoints.web.exposure.include=health,metrics
```

`*`로 설정 하면 모든 앤드포인트를 가지지만, 여기서 중요한 점은 여러 앤드포인트를 사용 설정할수록 CPU와 메모리를 많이 사용한다는 것이다.

# 스프링 부트 vs 스프링 MVC vs 스프링

스프링의 핵심은 의존성 주입(`@Component`, `@Autowired` 등)이다. 앱을 빌드하기 위해서는 다른 프레임워크가 필요하기 때문에 스프링 모듈이나 스프링 프로젝트로 스프링 셍태계를 확장한다.

스프링 MVC는 웹 어플과 REST API를 간단하게 빌드한다. (`@Controller`, `@RestController`, `@RequestMapping("")`등) 

스프링 부트는 스프링 프로젝트이다. 핵심 키워드는 PRODUCTION-READY와 QUICKLY이다. 즉, 프로덕션 환경에 배포할 어플리케이션을 빠르게 빌드하도록 돕는다. starter projects와 auto configuration으로 앱을 쉽게 빌드하고 다른 프레임워크를 쉽게 설치한다.

또한 비기능적 요구(NFRs)를 Actuator, Embedded Server, 로깅과 오류 핸들링, 그리고 프로필과 ConfigurationProperties로 쉽게 충족시킬 수 있다.