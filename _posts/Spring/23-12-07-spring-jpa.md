---
title:  "[Spring] JPA"
excerpt: " "

categories:
  - Spring

toc: true
toc_sticky: true
 
date: 2023-12-07
---

# H2 

주기억장치에 데이터를 저장하는 인메모리 데이터베이스, 프로젝트 생성 시 의존성을 추가하여 사용할 수 있다.

## 콘솔에 접근

**application.properties** 파일에 콘솔을 사용한다고 선언한다.

```
spring.h2.console.enabled=true
```

`서버주소/h2-console`로 이동하면 로그인 창이 보이고 JDBC URL값을 입력하는 부분에 프로젝트를 실행하면 로그창에 나오는 `url=jdbc:...`로 시작하는 값을 입력한다.

`url`값을 프로퍼티 파일에 작성하여 임의로 변경 가능하다.

```
spring.datasource.url=jdbc:h2:mem:testdb
```

## 테이블 만들기

리소스 폴더에 **schema.sql**을 생성하고 작성한다. 예를 들어 아이디, 이름, 저자로 구성되고 기본키가 아이디인 테이블은 다음과 같다.

```sql
create table example
(
    id bigint not null,
    name varchar(255) not null,
    author varchar(255) not null,
    primary key (id)
);
```

# Spring JDBC

일반적인 JDBC보다 훨씬 적은 코드 작성을 할 수 있다.

앞서 만든 `EXAMPLE` 테이블을 예로 들어 데이터를 하나 삽입한다고 가정하면 sql 문구는 다음과 같다.

```sql
insert into EXAMPLE (id, name, author) values(0, 'Spring', 'A');
```

이걸 스프링 JDBC를 활용하면 다음과 같다.

```java
@Repository
public class ExampleJdbcRepository {

    @Autowired
    JdbcTemplate springJdbcTemplate;

    private static String INSERT_QUERY =
            """
            insert into EXAMPLE (id, name, author) values(0, 'Spring', 'A');
            """;
    public void insertData() {
        springJdbcTemplate.update(INSERT_QUERY);
    }
}
```

확인을 위해 `CommandLineRunner` 인터페이스를 구현한 클래스를 하나 생성한다. 

해당 인터페이스는 빈이 스프링 앱 안에 포함되어 있을 때 실행되도록 할 때 쓰인다. 쉽게 말해 스프링 시작 시 실행되어야 할 로직이 있을 때 구현하는 인터페이스다.

```java
@Component
// 스프링 앱 시작 시 실행될 로직이 있을 때 구현하는 인터페이스
public class JdbcCommandLineRunner  implements CommandLineRunner {

    @Autowired
    private ExampleJdbcRepository exampleJdbcRepository;

    @Override
    public void run(String... args) throws Exception {
        exampleJdbcRepository.insertData();
    }
}
```

## 막간: JDBC와 비교

앞서 일반적인 JDBC보다 코드가 더 간략하게 작성된다는데 정말 그럴까? 이럴 때 아주 유용한 챗 GPT를 통해 간단하게 확인해봤다. 코드가 정확한 지는 나도 모르겠다. 

Spring JDBC를 통해 작성한 삽입 sql 코드를 JDBC로 구현해달라고 요청하였고 결과는 다음과 같다.

```java
public class ExampleJdbcRepository {

    // JDBC 연결을 위한 정보
    private static final String JDBC_URL = "jdbc:mysql://your_database_url";
    private static final String JDBC_USERNAME = "your_username";
    private static final String JDBC_PASSWORD = "your_password";

    // 쿼리 문자열
    private static String INSERT_QUERY =
            "INSERT INTO EXAMPLE (id, name, author) VALUES (0, 'Spring', 'A')";

    public void insertData() {
        Connection connection = null;
        PreparedStatement preparedStatement = null;

        try {
            // JDBC 드라이버 로딩
            Class.forName("com.mysql.cj.jdbc.Driver");
            // 데이터베이스 연결
            connection = DriverManager.getConnection(JDBC_URL, JDBC_USERNAME, JDBC_PASSWORD);
            // 쿼리 실행을 위한 PreparedStatement 생성
            preparedStatement = connection.prepareStatement(INSERT_QUERY);
            // 쿼리 실행
            preparedStatement.executeUpdate();

        } catch (ClassNotFoundException | SQLException e) {
            e.printStackTrace(); // 예외 처리는 실제 프로덕션 코드에서는 더 세밀하게 해야 합니다.

        } finally {
            // 리소스 해제
            try {
                if (preparedStatement != null) {
                    preparedStatement.close();
                }
                if (connection != null) {
                    connection.close();
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
}
```

## 검색과 삭제 구현

검색고 삭제를 구현하기 이전에 우선 하드코딩된 삽입 문구부터 개선한다. 이를 위해 데이터 클래스가 필요하다.

```java
public class Example {
    private long id;
    private String name;
    private String author;

    public Example() {}

    public Example(long id, String name, String author) {
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

    public void setId(long id) {
        this.id = id;
    }

    public void setName(String name) {
        this.name = name;
    }

    public void setAuthor(String author) {
        this.author = author;
    }

    @Override
    public String toString() {
        return "Example{" +
                "id=" + id +
                ", name='" + name + '\'' +
                ", author='" + author + '\'' +
                '}';
    }
}
```

그러면 삽입 문구는 다음과 같이 변경될 수 있다.

```java
private static String INSERT_QUERY =
        """
        insert into EXAMPLE (id, name, author) values(?, ?, ?);
        """;

public void insertData(Example example) {
    springJdbcTemplate.update(INSERT_QUERY, example.getId(), example.getName(), example.getAuthor());
}
```

```java
exampleJdbcRepository.insertData(new Example(0, "Spring!", "A"));
```

### 삭제 구현

삭제는 반환되는 값이 없으므로 다음과 같이 구현한다.

```java
private static String DELETE_QUERY =
        """
        delete from EXAMPLE where id = ?
        """;

public void deleteData(long id) {
    springJdbcTemplate.update(DELETE_QUERY, id);
}
```

```java
exampleJdbcRepository.deleteData(0);
```

### 검색 구현

검색은 값을 반환받아야 한다. 따라서 데이터 클래스인 `Example`을 반환 값으로 설정한다.

```java
private static String SELECT_QUERY =
        """
        select * from EXAMPLE where id = ?
        """;

public Example selectById(long id) {
    return springJdbcTemplate.queryForObject(SELECT_QUERY, new BeanPropertyRowMapper<>(Example.class), id);
    // 결과값을 빈으로 매핑 => Row Mapper
    // 이 상황에서 데이터 클래스와 테이블의 형식은 같으므로 다음과 같이 지정
}
```

`queryForObject`는 일반적인 `query`와는 다르게 단일 행의 결과를 가져온다. 이 코드는 현재 특정 id값을 이용하여 단일 행을 검색하기 때문에 다음 메소드를 사용한다. 

결과값을 빈으로 매핑하는 것을 RowMapper라고 하는데 이는 `BeanPropertyRowMapper` 클래스를 통하여 수행한다. 지금 이 경우 반환받는 값의 속성과 데이터 클래스가 가진 속성이 같기 때문에 위와 같이 작성하면 된다.

# Spring JPA

현재 방식은 요구하는 쿼리와 데이터베이스가 보유한 테이블이 많아질 수록 쿼리 문이 증가한다. JPA를 사용하면 데이터 클래스를 데이터베이스에 존재하는 테이블로 직접 매핑하게 된다.

사용 방식은 간단하다. 앞서 작성한 데이터클래스에 몇 가지 어노테이션을 추가한다.

```java
@Entity
public class Example {

    @Id // 주요 키 선언
    private long id;
    @Column(name="name") // 변수 이름과 테이블 속성 이름이 일치하면 필요하지 않음
    private String name;
    @Column(name="author")
    private String author;
    //...
}
```

그 후 다음과 같이 리포지토리를 작성하면 된다.

```java
@Repository
public class ExampleJpaRepository {

    @PersistenceContext
    private EntityManager entityManager;

    public void insert(Example example) {
        entityManager.merge(example);
    }

    public Example selectById(long id) {
        return entityManager.find(Example.class, id);
    }

    public void deleteById(long id) {
        Example example = entityManager.find(Example.class, id);
        entityManager.remove(example);
    }
}
```

마지막으로 쿼리 사용 클래스에 다음 어노테이션을 추가한다.

```java
@Transactional
public class JpaCommandLineRunner implements CommandLineRunner { ... }
```

위 코드는 앞서 Spring JDBC로 작성한 코드와 정확히 같은 기능과 사용법을 가진다. JPA를 실행하고 있을 때 생성된 AQL을 확인하고 싶다면 **application.properties**를 설정한다.

위 방식의 장점은 쿼리를 굳이 신경쓸 필요가 없이 그에 맞는 엔터티를 전달하기만 하면 된다는 것이다.

```
spring.jpa.show-sql=true
```

# Spring Data JPA

이 방법은 엔터티까지 신경 쓸 필요가 없어진다. 그저 선언한 데이터를 특정 인터페이스를 상속 받는 인터페이스를 정의한 다음 제공하는 메소드를 사용하면 된다.

```java
// 엔터티와 주요 키의 데이터 타입을 정의
public interface ExampleDataJpa extends JpaRepository<Example, Long> {

}
```

```java
@Component
@Transactional
// 스프링 앱 시작 시 실행될 로직이 있을 때 구현하는 인터페이스
public class DataJpaCommandLineRunner implements CommandLineRunner {

    @Autowired
    private ExampleDataJpa exampleDataJpa;

    @Override
    public void run(String... args) throws Exception {
        exampleDataJpa.save(new Example(0, "Spring!", "A"));
        exampleDataJpa.save(new Example(1, "AWS!", "B"));
        exampleDataJpa.save(new Example(2, "Node.js!", "C"));
        exampleDataJpa.deleteById(0L);
        System.out.println(exampleDataJpa.findById(1L));

        System.out.println(exampleDataJpa.findAll());
        System.out.println(exampleDataJpa.findByAuthor("A"));
        System.out.println(exampleDataJpa.findByName("Node.js!"));
    }
}
```

## 명명 규칙

개발자가 임의의 메소드를 명명 규칙에 따라서 작성하면 작동한다. 이름에 따라서 특정 행을 검색하고 싶다면 다음 명명규칙을 따라서 인터페이스에 정의한다.

```java
List<Example> findByName(String name);
```

*Hibernate?*

JPA는 기술 명세를 정의하는 API이다. Hibernate는 JPA를 구현한 유명 구현체 중 하나이다. JPA를 따르면서도 자체 고유 기능과 설정을 제공한다. 표준을 따르는 경우에는 일반적으로 jpa를 사용하지만 프로젝트에 목표에 따라 이런 JPA 구현체를 사용할 수도 있다.