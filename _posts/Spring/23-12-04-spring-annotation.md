---
title:  "[Spring] 다양한 어노테이션"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-04
---

# 컴포넌트의 세분화

컴포넌트는 제네릭 어노테이션으로 어느 클래스에서든 사용 가능하다. 또한 컴포넌트의 목적에 따라서 세분화된 어노테이션을 사용한다.

|어노테이션|설명|
|:---:|:---:|
@Service|비지니스 로직
@Controller|웹 어플과 REST API 컨트롤러
@Respository|DB와 통신

구체적인 어노테이션 사용은 클래스의 역할을 프레임워크에게 명확히 하고(AOP, 역할 기반 프로그래밍), 그 위에 부가적인 동작을 명시할 수 있다.

또한 그 역할마다 프레임워크가 제공하는 기능을 자동으로 사용할 수 있게 한다. 예를 들어 `@Respository`는 자동으로 JDBC 예외 변환 기능에 연결된다.

# Lazy

일반적으로 모든 컴포넌트는 컨택스트를 불러오는 동시에 초기화되어 메모리에 적재된다. 이런 자동 초기화를 방지하고 해당 컴포넌트를 사용할 때 초기화를 수행하도록 하는 어노테이션이 바로 `@Lazy`다.

```java
@Lazy
class B {
    private A a;
    public B(A a) {
        System.out.println("초기화");
        this.a = a;
    }

    public void doSomething() {
        System.out.println("Do something");
    }
}

@Configuration
@ComponentScan
public class Example {
    public static void main(String[] args) {
        try(var context = new AnnotationConfigApplicationContext(Example.class)) {
            Arrays.stream(context.getBeanDefinitionNames()).forEach(System.out::println);
            System.out.println("초기화 실행...");
            context.getBean(B.class).doSomething();
        }
    }
}
```

자주 사용되는 방법은 아니다.  

# PostConstruct & PreDestroy

빈 생성 직후 실행되어야 하는 메소드와 빈 삭제 직전 실행되어야 하는 메소드를 대상으로 하는 어노테이션이다.

```java
@Component
class A {
    private B b;

    public A(B b) {
        super();
        this.b = b;
        System.out.println("초기화");
    }

    // 의존성 주입이 완료된 이후 호출되어야 하는 메소드를 대상으로 실행
    @PostConstruct
    public void initialize() {
        b.getReady();
    }

// 컨텍스트에서 bean이 삭제되기 전에 수행하도록 하는 어노테이션
    @PreDestroy
    public void cleanup() {
        System.out.println("청소");
    }
}

@Component
class B {
    public void getReady() {
        System.out.println("Logic using B");
    }
}

    @Configuration
    @ComponentScan
    public class Example {
    public static void main(String[] args) {
        try(var context = new AnnotationConfigApplicationContext(Example.class)) {
            Arrays.stream(context.getBeanDefinitionNames()).forEach(System.out::println);
        }
    }
}
```

# Scope

```java
@Component
class Normal {

}

@Scope(value=ConfigurableBeanFactory.SCOPE_PROTOTYPE)
@Component
class Prototype {

}

@Configuration
@ComponentScan
public class DepInjectionExample {

    public static void main(String[] args) {

        try(var context = new AnnotationConfigApplicationContext(DepInjectionExample.class)) {
            System.out.println(context.getBean(Normal.class));
            System.out.println(context.getBean(Normal.class));

            System.out.println(context.getBean(Prototype.class));
            System.out.println(context.getBean(Prototype.class));
            System.out.println(context.getBean(Prototype.class));
        }
    }
}
```

스프링 빈의 생성 방식을 결정하는 어노테이션이다. 기본적으로 스프링 빈은 스프링 싱글톤으로 생성되어 IoC 컨테이너 당 객체 인스턴스가 하나이므로 주소값은 달라지지 않는다. 하지만 위 코드처럼 `@Scope`를 선언하고 그 값에 `ConfigurableBeanFactory.SCOPE_PROTOTYPE`를 대입하면, 해당 스프링 빈은 매 참조마다 스프링 빈이 새롭게 생성된다.

## 스프링 싱글톤 vs 싱글톤

두 용어는 약간 다르다.

1. 스프링 싱글톤
- IoC 컨테이너가 bean객체를 생성하고 생명주기 관리
- 어플리케이션 컨텍스트 내에서 단일 인스턴스로 유지

2. 싱글톤
- 클래스의 인스턴스가 오직 하나만 생성되고 이에 대한 전역적인 접근점을 제공

### 차이점(복붙)

- 스프링의 싱글톤은 IoC 컨테이너에 의해 객체의 라이프사이클이 관리되며, 스프링 컨테이너 내에서 빈으로 등록된 클래스에 적용

- 스프링 싱글톤은 빈 스코프로 제한되지 않고, 여러 다양한 스코프를 지원하고, 이는 설정에 따라 프로토타입, 세션, 요청 등 다양한 스코프를 사용할 수 있다는 것을 의미

- 스프링 싱글톤은 빈의 의존성 주입, AOP(Aspect-Oriented Programming) 등 다양한 기능을 통해 더 유연하게 확장

일반적으로 스프링에서는 싱글톤 패턴을 직접 구현하지 않고, 스프링의 IoC 컨테이너에 빈을 등록하여 싱글톤을 활용하는 것이 권장

# CDI

의존성 주입을 하는데 사용되는 다른 문법이라고 보면 된다. 실제로 잘 사용되는 지는 모르겠지만, 알아두면 좋다.

```java
//@Component
// 컴포넌트 대신 CDI 어노테이션을 사용(알아두면 좋음)
@Named
class BusinessService {
    private DataService dataService;
    public DataService getDataService() {
        return dataService;
    }

//    @Autowired
    @Inject
    public void setDataService(DataService dataService) {
        System.out.println("Setter Injection");
        this.dataService = dataService;
    }
}

//@Component
@Named
class DataService { }
@Configuration
@ComponentScan
public class DepInjectionExample {
    public static void main(String[] args) {
        try(var context = new AnnotationConfigApplicationContext(DepInjectionExample.class)) {
            Arrays.stream(context.getBeanDefinitionNames()).forEach(System.out::println);
            System.out.println(context.getBean(BusinessService.class).getDataService());
        }
    }
}
```

`@Component`가 `@Named`로, `@Autowired`가 `@Inject`로 대체될 수 있다.

# XML을 이용한 빈 생성

최근에는 사실상 거의 사용되지 않지만, 스프링을 다룰 때 XML에 빈을 작성하고 컨텍스트로 불러와 사용하는 방법도 있다.

**example.xml**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xmlns:context="http://www.springframework.org/schema/context" xsi:schemaLocation="
        http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd
        http://www.springframework.org/schema/context http://www.springframework.org/schema/context/spring-context.xsd"> <!-- bean definitions here -->

    <bean id="name" class="java.lang.String">
        <constructor-arg value="Name" />
    </bean>
    
   <context:component-scan base-package="package" />
</beans>
```

```java
// xml 경로를 문자열로 작성
try(var context = new ClassPathXmlApplicationContext("example.xml")) {
            Arrays.stream(context.getBeanDefinitionNames()).forEach(System.out::println);
    }
```