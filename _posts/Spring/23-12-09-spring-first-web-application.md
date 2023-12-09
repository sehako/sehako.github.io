---
title:  "[Spring] 웹 어플리케이션 빌드 - 1"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-09
---

[강의](https://www.udemy.com/course/spring-boot-and-spring-framework-korean/)를 통해 클론할 웹 앱의 목표는 로그인 기능 구현과 할 일(todo)리스트를 출력하는 것이다.

# 컨트롤러

스프링의 컨트롤러는 웹 브라우저를 통해 서버의 특정 경로에 접근하면 경로에 매핑된 함수가 실행되어 그 값을 반환하는 역할을 한다. 

예를 들어 `서버주소/hello`에 접근하면 Hello World!를 반환하는 코드는 다음과 같다.

```java
@Controller
public class ExampleController {

    @RequestMapping("hello")
    // Spring MVC는 문자열을 리턴할 때 문자열이 아닌 문자열 이름으로 된 뷰를 검색하여 리턴
    // 해당 어노테이션으로 리턴 값 그대로를 출력하도록 함
    @ResponseBody
    public String sayHello() {
        return "Hello World!";
    }
}
```

스프링 MVC는 문자열 반환 시 문자열 이름으로 된 뷰를 검색하여 반환한다. 따라서 `@ResponseBody` 어노테이션으로 반환 값을 문자열 그대로 반환하도록 한다. 

## html 반환

html을 문자열 형식으로 만들어 반환할 수 있다.

```java
@RequestMapping("hello-html")
@ResponseBody
public String html() {

    // html을 리턴할 수도 있다
    StringBuffer sb = new StringBuffer();
    sb.append("<html>");
    sb.append("<head>");
    sb.append("<title> My first HTML </title>");
    sb.append("</head>");
    sb.append("<body>");
    sb.append("First body");
    sb.append("</body>");
    sb.append("</html>");
    return sb.toString();
}
```

# JSP 반환

JSP는 스프링에서 뷰의 역할을 담당한다. 뷰를 반환하기 위해 **pom.xml**에 의존성 추가가 필요하다.

```
<dependency>
    <groupId>org.apache.tomcat.embed</groupId>
    <artifactId>tomcat-embed-jasper</artifactId>
</dependency>
```

프로젝트의 **resources** 폴더 내에 다음 디렉터리를 정의한다.

**META-INF/resources/WEB-INF/jsp/**

해당 디렉터리 내에 JSP를 정의하여 사용한다. 위 html 코드를 jsp로 정의한 **sayHello.jsp** 파일은 다음과 같다.

```java
<html>
    <head>
        <title>My Title</title>
    </head>
    <body>
        First JSP Body
    </body>
</html>
```

어플리케이션 프로퍼티에서 뷰 파일의 경로와 확장자를 다음과 같이 지정한다.

```
spring.mvc.view.prefix=/WEB-INF/jsp/
spring.mvc.view.suffix=.jsp
```

`prefix` 부분에서 보시다시피 스프링은 **META-INF/resources/**까지는 자동으로 설정되어 있기 때문에 나머지 경로만 적으면 된다.

컨트롤러에 라우팅을 추가한다.

```java
@RequestMapping("hello-jsp")
public String helloJsp () {
    return "sayHello";
}
```

최종적으로 **META-INF/resources/WEB-INF/jsp/sayHello.jsp** 파일을 불러 해당 경로에 접근한 사용자에게 반환한다.