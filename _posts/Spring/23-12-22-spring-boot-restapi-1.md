---
title:  "[Spring] 스프링 부트 & REST API - 1"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-25
---

# REST API

주소에 접근하면 Hello World 문자열을 반환하는 REST API 예시

```java
@RestController
public class HelloWorldController {
    // GetMapping으로 GET요청에 대한 처리를 수행
    @GetMapping(path = "/hello-world")
    public String showHelloWorld() {
        return "Hello World!";
    }
}
```

### 요청의 종류

|요청|설명|
|:---:|:---:|
GET|기존 정보 검색
POST|새로운 정보 생성
PUT|존재하는 정보 업데이트
PATCH|존재하는 정보의 일부분 업데이트
DELETE|정보 삭제

## @Controller vs @RestController

1. `@Controller`

- MVC 웹 어플리케이션에서 사용
- View와 상호작용, 사용자에게 데이터를 보여주고 입력을 받아 비즈니스 로직 수행

2. `@RestController`

- 주로 RESTful 웹 서비스에서 사용, 데이터 제공, 반환값은 HTTP 응답 본문에 직접 매핑
- JSON또는 XML 형식으로 데이터 반환, View와의 상호작용이 적음

HTML을 랜더링 하여 사용자에게 보여주고 사용자 입력을 처리 > `@Controller`

요청에 따라 처리한 값을 JSON또는 XML로 리턴 > `@RestController`

## JSON 리턴하기

```java
public class HelloWorldBean {
    private String message;

    //getter and setter
    //toString...
}
```

```java
@GetMapping(path = "/hello-world-bean")
public HelloWorldBean helloWorldBean() {
    // JSON 값으로 반환됨
    return new HelloWorldBean("Hello World!");
}
```

### 과정

실행 시 프레임워크는 다음 과정을 수행한다.

1. DispatcherServletAutoConfiguration(Auto Configuration)에 의해 설정된 디스페지 서블렛이 가장 먼저 실행되어 `/`로 매핑
2. `@ResponseBody` + JacksonHttpMessageConverters(Auto Configuration)이 `HelloWorldBean`을 JSON으로 변환
3. ErrorMvcAutoConfiguration(Auto Configuration)이 오류 매핑
4. Starter Projects가 모든 jar을 활성화

정리하자면 스프링 부트의 두 가지 주요 기능인 Auto Configuration과 Starter Projects가 웹 어플을 빌드한다.

## 주소 매개변수 전달

```java
@GetMapping(path = "/hello-world/path-variable/{name}")
public HelloWorldBean helloWorldPathVariable(@PathVariable String name) {
    // JSON 값으로 반환됨
    return new HelloWorldBean(
            String.format("Hello World, %s", name)
    );
}
```

`@PathVariable`로 `path`에 `{}`로 선언한 변수 값을 가져와 메소드 내에서 사용 가능하다.

# 고급 REST API

REST API 사용자가 알아야 할 사항은 다음과 같다.

1. 리소스
2. 수행되고 있는 작업
3. 요청/응답 구조
4. 제약과 검증

따라서 대부분의 REST API는 그 사용법을 작성해놓은 문서가 존재한다. 

문서는 항상 최신 변경 사항을 담고 있어야 하며, 전체 문서는 일관되게 관리하여야 한다.

문서 관리는 수동으로 관리하는 것과 코드에서 문서를 생성하는 방법이 있다.

## Swagger & Open API

Swagger는 REST API를 문서화 할 수 있는 도구다. Swagger를 기반으로 Open API가 만들어지고 현재는 사실상의 표준으로 자리매김 하였다.

JSON또는 YAML형식으로 API를 정의하여 문서화한다. Swagger UI를 통해 문서를 시각화할 수 있고 테스트도 가능하다. 스프링에서는 프레임워크를 추가하면 스프링의 코드를 자동으로 문서화할 수 있다.

```
<dependency>
    <groupId>org.springdoc</groupId>
    <artifactId>springdoc-openapi-starter-webmvc-ui</artifactId>
    <version>2.3.0</version>
</dependency>
```

앱을 실행하고 `주소값/v3/api-docs`에 접속하면 문서를 볼 수 있고, `주소값/swagger-ui.html`에 접속하면 다음 문서를 시각화한 문서를 볼 수 있다.

## 미디어 타입

Content Nagotiation으로도 불리며, 동일한 요청에 대해 다양한 응답 형식을 보유할수 있다. 예를 들어 API 사용자가 `Accept` 헤더를 이용하여 JSON이 아닌 XML 형식으로 응답을 받거나, 다른 언어를 받을 수 있다.

XML 형식으로 응답을 주고자한다면, 다음 의존성만 추가해주면 된다.

```
<dependency>
    <groupId>com.fasterxml.jackson.dataformat</groupId>
    <artifactId>jackson-dataformat-yaml</artifactId>
</dependency>
```

요청 헤더에 `Accept`를 `application/xml`으로 정의하면 xml 형식으로 데이터를 보내준다.

## i18n

국제화라고도 하며, 요청 헤더에 `Accept-Language`를 사용하여 사용자가 요청하는 언어로 응답해줄 수 있다.

스프링에서 국제화를 다루기 위해서 **resources** 폴더에 **messages.properties**를 추가해야 한다.

```
i18n.msg=Korean
```

자바에서 `MessageSource` 인터페이스를 이용하여 프로퍼티 파일의 메시지를 가져온다.

`Locale`로 현재 위치를 가져와 `getMessage` 메소드에 전달해주고 이를 기본 메시지로 설정하면 된다.

```java
@RestController
public class HelloWorldController {
    private MessageSource messageSource;

    public HelloWorldController(MessageSource messageSource) {
        this.messageSource = messageSource;
    }

    @GetMapping(path = "/hello-world-i18n")
    public String helloWorldI18n() {
        Locale locale = LocaleContextHolder.getLocale();
        return messageSource.getMessage("i18n.msg", null, "Default Message", locale);
    }
}
```

국제화를 위해 프로퍼티 파일을 하나 더 생성해야 한다. 현재 위치는 한국이므로 요청 헤더에 영어(`en`)를 요청하면 다른 메시지를 띄우도록 하였다. (**messages_en.properties**)

```
hello.world.msg=English
```

헤더에 `Accept-Language`값을 `en`으로 보내면 다른 메시지가 응답된다.

## 버전 관리

기존 API에서 주요 기능(요청 방법, 응답값 형식 등)에 변동사항이 있을 때, 기존 기능을 사용하던 사용자들을 고려하여 버전을 관리 예를 들어 다음 응답을 주던 API의 변동사항이 있다고 가정한다.

```json
{
    "name": "Fname Lname"
}

// 위 응답 형식에서 다음과 같이 변동됨

{
    "name": {
        "firstName": "Fname",
        "LastName": "Lname"
    }
}
```

버전관리가 없다면 사용자들 또한 응답에 대한 처리를 즉각적으로 변경해야만한다. 이를 방지하고자 버전 관리를 한다.

다음 방법들로 버전관리를 수행한다.

- URL
- 요청 파라미터
- 헤더
- 미디어 타입

위 응답값 구현을 위해서 우선 데이터 클래스를 정의한다.

1번 버전

```java
public class Person {
    private String name;
    // 생성자, getter, toString...
}
```

2번 버전은 `name`키 값에 성과 이름이 들어있는 형태이므로 데이터 클래스가 두 개 필요하다.

```java
public class Name {
    private String fName;
    private String lName;
    // 생성자, getter, toString...
}
```

```java
public class Person2 {
    private Name name;

    public Name(String name) {
        String[] temp = name.split("\\s");
        this.fName = temp[0];
        this.lName = temp[1];
    }
    // getter, toString...
}
```

### URL

```java
@RestController
public class VersioningPersonController {
    @GetMapping(path = "/v1/person")
    public Person getPersonV1() {
        return new Person("Fname Lname");
    }

    @GetMapping(path = "/v2/person")
    public Person2 getPersonV2() {
        return new Person2(new Name("Fname Lname"));
    }
}
```

### 파라미터

```java
// 주소값/person/ver=1
@GetMapping(path = "/person", params = "ver=1")
public Person getPersonParam() {
    return new Person("Fname Lname");
}

@GetMapping(path = "/person", params = "ver=2")
public Person2 getPersonParam2() {
    return new Person2(new Name("Fname, Lname"));
}
```

### 헤더

```java
@GetMapping(path = "/person/header", headers = "X-API-VERSION=1")
public Person getPersonHeader() {
    return new Person("Fname Lname");
}

@GetMapping(path = "/person/header", headers = "X-API-VERSION=2")
public Person2 getPersonHeader2() {
    return new Person2(new Name("Fname, Lname"));
}
```

요청 헤더에 `X-API-VERSION`값을 설정하여 응답을 다르게 받을 수 있다.

### 미디어 타입

```java
@GetMapping(path = "/person/media", produces = "application/vmd.company.app-v1+json")
public Person getPersonMedia() {
    return new Person("Fname Lname");
}

@GetMapping(path = "/person/media", produces = "application/vmd.company.app-v2+json")
public Person2 getPersonMedia2() {
    return new Person2(new Name("Fname, Lname"));
}
```

`Accept` 헤더 값을 위 `produces`에 명시된 값으로 정의하여 응답을 받는다.

### 고려 사항

- URI Pollution: URL와 매개변수를 통한 버전 관리는 새로운 URL을 생성함
- HTTP 요청의 오용: HTTP 헤더는 버전 관리 용도로 사용해서는 안됨 (헤더와 미디어 타입을 이용한 버전 관리)
- Caching: 헤더와 미디어 타입을 이용한 방법은 URL 기반으로 캐싱을 할 수 없음
- 브라우저 실행 가능 여부: 미디어 타입과 헤더는 일반적인 브라우저에서 실행할 수 없음
- API 문서: 헤더와 미디어 타입 버전 관리 방법은 자동 문서 생성 등이 잘 이루어지지 않을 수 있음

이런 사항을 참고하여 버전 관리 방식을 사용해야 한다. 

무조건 추천되는 방법은 없으며, 모범사례는 하나의 프로젝트 또는 기업에서 하나의 버전 관리 방법을 사용하여 전체 앱을 빌드하는 일관성을 보이는 것이다.