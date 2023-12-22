---
title:  "[Spring] 스프링 부트로 REST API 빌드"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-22
---

# REST API

주소에 접근하면 Hello World 문자열을 반환하는 REST API

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

### 뒷 단에서 일어나는 과정

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

`@PathVariable`로 `path`에 `{}`로 선언한 변수 값을 가져와 메소드 내에서 사용 가능