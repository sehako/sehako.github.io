---
title:  "[Spring] 스프링 시작"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-11-30
---

# Spring

스프링은 자바 기반의 오픈 소스 프레임워크로, 간결하고 모듈화된 방식으로 어플리케이션을 구축하고 유지보수하는 것을 목표로 한다.

**주요 기능**

1. 의존성 주입: 결합도를 낮추고 유지 보수성 향상
2. AOP(Aspect-Oriented-Programming): 관심사의 분리를 통해 핵심 비즈니스 로직과 각 관심사를 모듈화
3. 트랜잭션 관리: 선언적 트랜잭션 처리를 지원, DB 트랜잭션을 쉽게 관리
4. MVC 웹 프레임워크: 스프링 MVC는 모델-뷰-컨트롤러 아키텍처를 기반의 웹 어플 구축 기능 제공
5. 데이터 액세스 지원: JDBC, ORM 프레임워크인 Hibernate와의 통합을 통해 쉬운 DB 엑세스 처리
6. 보안 기능: 스프링 시큐리티를 이용한 강력한 보안 기능
7. 배치 프로세싱: 대용량 데이터를 처리하는 배치 프로세스 개발 기능 제공
8. 테스트 지원: 단위 테스트, 통합 테스트, 인수 테스트 등 다양한 테스트 수준에서의 지원을 제공

## 강의 중 나온 이론

### 컨테이너의 차이점

- IOC 컨테이너: 클래스와 설정을 생성하면 런타임 시스템을 구성, 스프링 컨텍스트를 생성하고 Bean의 생명 주기를 관리
- Application 컨테이너: 앤터프라이즈 전용 기능이 있는 고급 스프링 컨테이너(가장 많이 사용함)

### 자바 bean과 스프링 bean의 차이점

- Java bean: 3가지 규칙 존재(인수 생성자가 없어야 함(public no-arg), 게터와 세터가 있어야 함, Serializable 인터페이스 구현), Java bean의 개념은 최근에는 중요하지 않음
- Spring bean: Spring 프레임워크에서 관리하는 모든 자바 객체
- Pojo: 모든 (오래된) 자바 객체

# 스프링 실습

스프링의 단계는 다음과 같다.

1. 스프링이 관리할 것들 설정 (`@Configuration`)
2. 스프링 컨텍스트 실행

**ExampleConfiguration.java**

```java
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class ExampleConfiguration {
  //Spring에서 관리하는 것들을 Spring bean이라고 함

  @Bean
  public String name() {
      return "Name";
  }
}
```

`@Configuration` 어노테이션을 선언하고 내부에 `@Bean` 어노테이션을 통해 스프링이 관리할 것들을 선언한다.

그 후 스프링 컨텍스트를 실행하고 Bean에 접근한다.

**Example.java**

```java
import org.springframework.context.annotation.AnnotationConfigApplicationContext;

public class App02HelloWorldSpring {
  public static void main(String[] args) {
  // 1. Spring Context 실행
    try(var context =
    new AnnotationConfigApplicationContext
    (ExampleConfiguration.class)) {
      //메소드 이름을 문자열로 명시
      System.out.println(context.getBean("name"));
    }
  }
}
```

예외 처리를 통해 컨텍스트를 실행하면 모든 처리가 끝난 후의 자원 관리를 자동으로 해준다. 따라서 `context.close()`를 안해도 된다.

Bean에 특정 이름을 지정해줄 수 있다. 이 경우 메소드 이름이 아닌 임의 지정한 Bean의 이름으로 접근해야 한다.

```java
@Bean(name = "NAME")
public String name() {
  return "Name";
}
```

```java
System.out.println(context.getBean("NAME"));
```

## 다양한 Bean

### record

```java
record Person(String name, int age, Address address) { };
record Address(String firstLine, String city) { };

@Configuration
public class ExampleConfiguration {
  @Bean
  public Person person() {
    return new Person("Name", 20, new Address("ABC", "EFG"));
  }
}
```

`record`는 JDK16부터 추가된 일종의 데이터 클래스로, getter, setter, 생성자 등을 자동으로 생성한다.

이렇게 선언한 경우 컨텍스트에서 다음과 같이 접근할 수도 있다.

```java
System.out.println(context.getBean(Person.class));
```

### 다른 bean에 접근

특정 bean에서 다른 bean에 메소드를 통해 접근할 수 있다. (메소드 콜)

```java
@Bean
public String name() {
  return "Name";
}
@Bean
public Person person() {
  return new Person(name(), 20, new Address("ABC", "EFG"));
}
```

### 매개변수를 통한 접근

```java
@Bean
public Address address() {
    return new Address("ABC", "EFG");
}

@Bean
public Person person(String name, int age, Address address) {
  return new Person(name, age, address);
}
```

이름이 설정된 bean 클래스의 경우 그 bean의 이름을 넣어야 한다.

### 중복 해결

`record`를 통해 다음과 같이 선언할 경우 컨텍스트에서 다음과 같이 접근하면 에러가 발생한다.

```java
@Bean
public Address address() {
  return new Address("A", "B");
}

@Bean
public Address address2() {
  return new Address("C", "D");
}
```

```java
System.out.println(context.getBean(Address.class));
```

이를 해결하기 위해 `@Primary` 어노테이션으로 후보 중에서 최우선 클래스를 지정할 수 있다.

```java
@Bean
@Primary
public Address address() {
  return new Address("A", "B");
}

@Bean
public Address address2() {
  return new Address("C", "D");
}
```

### 다른 Bean을 참조할 때

```java
@Bean
@Primary
public Address address() {
  return new Address("A", "B");
}

@Bean
public Address address2() {
  return new Address("C", "D");
}

@Bean
public Person person(String name, int age, Address address) {
  return new Person(name, age, address);
}
```

위 코드에서 `person`은 항상 `@Primary`가 명시된 클래스를 참조한다. 따라서 다른 클래스를 참조하고자 한다면 `@Qualifier` 어노테이션을 이용하여 다음과 같이 작성한다.

```java
@Bean
@Primary
public Address address() {
  return new Address("A", "B");
}

@Bean
@Qualifier("addressQualifier")
public Address address2() {
  return new Address("C", "D");
}

@Bean
public Person person(String name, int age, @Qualifier("addressQualifier") Address address) {
  return new Person(name, age, address);
}
```
