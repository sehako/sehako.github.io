---
title:  "[Spring] 스프링 부트 & REST API -2"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-26
---

# HATEOAS

어플리케이션의 상태 엔진으로서의 하이퍼미디어(Hypermedia as the Engine of Application State)이다.

## 구현 방법

1. 직접 포맷을 짜고 구현
2. 표준 구현: 스프링 HATEOAS로 HAL을 생성

## 표준 구현

사용자의 번호, 이름, 생일을 반환하는 REST API에 HATEOAS를 적용하면 다음과 같다.

```java
@GetMapping(path = "/users")
public List<User> getAllUsers() {
    return service.findAll();
}

@GetMapping(path = "/users/{id}")
public EntityModel<User> getUser(@PathVariable int id) {
    User user = service.findUserById(id);
    if(user == null) {
        throw new UserNotFoundException("id: " + id);
    }
    // User 클래스를 EntityModel로 래핑
    EntityModel<User> entityModel = EntityModel.of(user);
    // 링크에 해당되는 메소드를 지정
    WebMvcLinkBuilder link = WebMvcLinkBuilder.linkTo(WebMvcLinkBuilder.methodOn(this.getClass()).getAllUsers());
    // json 키 이름과 함께 링크 추가
    entityModel.add(link.withRel("all-users"));
    return entityModel;
}
```

```json
{
    "id": 1,
    "name": "A",
    "birthDate": "2003-12-26"
}
// 위 응답이 다음 응답값으로 바뀜

{
    "id": 1,
    "name": "A",
    "birthDate": "2003-12-26",
    "_links": {
        "all-users": {
            "href": "http://localhost:8080/users"
        }
    }
}
```

## Hal Explorer

```
<dependency>
    <groupId>org.springframework.data</groupId>
    <artifactId>spring-data-rest-hal-explorer</artifactId>
</dependency>
```

# 사용자 지정 응답

## 직렬화

객체를 스트림으로 전환(JSON)하는 것, 상태를 저장하거나 네트워크를 통해 전송 가능한 형태로 변환하는 과정을 말한다. 

하나의 예로 `@JsonProperty("value")`을 이용하여 특정 필드의 이름을 명시적으로 지정할 수 있다.

앞서 사용자 정보를 응답하는 API에서 데이터 클래스의 변수에 다음과 같은 어노테이션을 선언한다.

```java
    @JsonProperty("user_name")
    private String name;
```

그러면 `name` 부분이 `user_name`이 된다.

```json
{
    "id": 1,
    "user_name": "A",
    "birthDate": "2003-12-26"
}
```

## 필터링

### 정적 필터링

모든 REST API에 대한 동일한 필터링을 적용할 수 있다. 데이터 클래스에서 변수에 `@JsonIgnore` 어노테이션을 선언하거나, 데이터 클래스 자체에 `@JsonIgnoreProperties` 어노테이션을 선언하면 된다.

```java
public class SomeBean {
    private String field1;
   @JsonIgnore
    private String field2;
    private String field3;
}
```

`field2`가 제외되고 나머지가 응답값으로 반환된다.

```java
@JsonIgnoreProperties("field1")
public class SomeBean {
    private String field1;
    private String field2;
    private String field3;
}
```

`@JsonIgnoreProperties`는 여러 변수를 필터링할 수 있다. `filed2`도 제외하고 싶다면 다음처럼 선언한다.

```java
@JsonIgnoreProperties({"field1", "field2"})
```

이 방법은 컨트롤러에 어떤 메소드를 정의해도 동일하게 필터링된 응답을 받는다.

```java
@RestController
public class FilteringController {
    @GetMapping("/filtering")
    public SomeBean filtering() {
        return new SomeBean("value1", "value2", "value3");
    }
}
```

### 동적 필터링

REST API에 따라서 다른 필터링을 적용할 수 있다. 데이터 클래스가 아닌 컨트롤러에서 필터를 적용하는 방식이다.

```java
@RestController
public class FilteringController {
    @GetMapping("/filtering")
    public MappingJacksonValue filtering() {
        SomeBean someBean = new SomeBean("value1", "value2", "value3");
        // 필터링 할 대상 변수 지정
        MappingJacksonValue mappingJacksonValue = new MappingJacksonValue(someBean);
        // 필터 정의
        SimpleBeanPropertyFilter filter = SimpleBeanPropertyFilter.filterOutAllExcept("field1", "field3");
        FilterProvider filters = new SimpleFilterProvider().addFilter("SomeBeanFilter", filter);
        // 필터링
        mappingJacksonValue.setFilters(filters);
        return mappingJacksonValue;
    }

    @GetMapping("/filtering-list")
    public MappingJacksonValue filteringList() {
        List<SomeBean> listSomeBean = Arrays.asList(new SomeBean("value1", "value2", "value3"), new SomeBean("value4", "value5", "value6"));
        MappingJacksonValue mappingJacksonValue = new MappingJacksonValue(listSomeBean);
        SimpleBeanPropertyFilter filter = SimpleBeanPropertyFilter.filterOutAllExcept("field2");
        FilterProvider filters = new SimpleFilterProvider().addFilter("SomeBeanFilter", filter);
        mappingJacksonValue.setFilters(filters);

        return mappingJacksonValue;
    }
}
```

반환값으로 `MappingJacksonValue`를 반환해야 한다. 

동적 필터링은 선택된 필드를 제외시키는 것이 아닌 선택된 필드만 응답으로 반환하도록 한다. 따라서 첫 번째 메소드는 `field1`과 `field3`를, 두 번재 메소드는 `field2`만을 응답으로 반환한다.

데이터 클래스에서 필터의 이름(`SomeBeanFilter`)을 적용한다.

```java
@JsonFilter("SomeBeanFilter")
public class SomeBean {
    // ...
}
```

#### 중복 코드 리팩토링

두 메소드 모두 `MappingJacksonValue`를 반환하고 클래스의 생성에 변수의 타입은 상관 없어 보여 하나의 제네릭 함수로 만들어보았다.

```java
@RestController
public class FilteringController {
    @GetMapping("/filtering")
    public MappingJacksonValue filtering() {
        SomeBean someBean = new SomeBean("value1", "value2", "value3");
        return fieldFiltering(someBean, new String[]{"field1", "field2"});
    }

    @GetMapping("/filtering-list")
    public MappingJacksonValue filteringList() {
        List<SomeBean> listSomeBean = Arrays.asList(new SomeBean("value1", "value2", "value3"), new SomeBean("value4", "value5", "value6"));
        return fieldFiltering(listSomeBean, new String[]{"field1"});
    }

    private <T> MappingJacksonValue fieldFiltering(T value, String[] filterValue) {
        MappingJacksonValue mappingJacksonValue = new MappingJacksonValue(value);
        SimpleBeanPropertyFilter filter = SimpleBeanPropertyFilter.filterOutAllExcept(filterValue);
        FilterProvider filters = new SimpleFilterProvider().addFilter("SomeBeanFilter", filter);
        mappingJacksonValue.setFilters(filters);

        return mappingJacksonValue;
    }
}
```