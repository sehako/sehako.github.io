---
title:  "[Spring] 컴포넌트"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-03
---

# @Component

설정 파일에 빈을 만들어 접근하는 대신 `@Component`로 스프링이 대신 빈을 생성하도록 할 수 있다. 컨텍스트가 다른 클래스의 인스턴스를 빈으로 만들고 관리하고자 한다면 다음 코드를 예시로 들 수 있다.

```java
public interface C {
    //...
}
```

```java
import org.springframework.context.annotation.Primary;
import org.springframework.stereotype.Component;

@Component
public class A implements C {
    //...
}
```

```java
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
@ComponentScan
public class Main {
    public static void main(String[] args) {

        try(var context = new AnnotationConfigApplicationContext(App01GamingBasicJava.class)) {
            context.getBean(A.class);
        }
    }
}
```

`@ComponentScan`은 자동적으로 현재 패키지의 `@Component`가 명시된 클래스를 참조하여 빈을 생성한다. 또한 위 코드는 별도의 설정 파일 없이 실행 파일이 설정 파일 역할도 한다는 것도 볼 수 있다.

만약 참조하고자 하는 클래스가 현재 패키지 외부에 있다면 다음과 같이 선언한다.

```java
@Component("package")
```

컴포넌트도 빈 처럼 `@Primary`와 `@Qualifier`를 사용하여 중복되는 클래스에 대한 처리를 수행할 수 있다.

## 컴포넌트 vs 빈

컴포넌트는 클래스를 대상으로, 빈은 메소드를 대상으로 수행된다. 

컴포넌트는 프레임워크에 의해 빈을 생성하고 빈은 개발자가 코드를 직접 작성함으로써 빈을 생성한다. 

일반적으로 대부분의 상황에서 컴포넌트가 권장된다. 

빈은 수 많은 사용자 정의 비즈니스 로직을 수행하거나 제 3자 라이브러리 빈을 인스턴스화 할 때 권장된다.(스프링 시큐리티 빈 등)

# 의존성 주입의 다양한 유형

생성자 주입, 필드 주입, 수정자(setter) 주입 이 세 가지 방법으로 나누어진다. 3개의 컴포넌트가 존재하고 한 컴포넌트가 다른 두 개의 컴포넌트를 참조할 때 의존성 주입은 각각 다음과 같다.

## 생성자 주입

```java
@Component
class Test {
    public Business(Dependency1 dependency1, Dependency2 dependency2) {
        System.out.println("Business");
        this.dependency1 = dependency1;
        this.dependency2 = dependency2;
    }
}
@Component
class Dependency1 {

}

@Component
class Dependency2 {

}
```

생성자 주입에서는 `@Autowired`가 필요하지 않다. 스프링 팀은 모든 초기화가 하나의 메소드에서 발생하는 생성자 기반 의존성 주입을 추천한다.

## 필드 주입

```java
@Component
class Test {
    @Autowired
    Dependency1 dependency1;
    @Autowired
    Dependency2 dependency2;
}
```

## 수정자 주입

```java
@Component
class Test {
    @Autowired
    public void setDependency1(Dependency1 dependency1) {
        this.dependency1 = dependency1;
    }
    @Autowired
    public void setDependency2(Dependency2 dependency2) {
        this.dependency2 = dependency2;
    }
}
```