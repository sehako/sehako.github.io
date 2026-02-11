---
title: MSA 환경 분리 및 스웨거 설정

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true

date: 2025-04-20
last_modified_at: 2025-04-20
---

몇 번의 포스팅에서 언급했던 것 처럼 스프링 클라우드를 통한 MSA를 구축하고 이를 나름대로 성공적으로 배포하였다. 오늘은 MSA를 구축할 때 고려하였던 개발 및 배포 환경 분리와 스웨거를 적용하는 법을 짧게 다뤄보겠다. 막간 자랑 타임으로 프로젝트의 최종적인 소프트웨어 아키텍처를 보여주겠다.

![아키텍처 이미지](/assets/images/infrastructure/25-04-20-dev-deploy-env-separation-swagger-config-in-msa/01.png)

상당히 복잡하다. 간략하게 설명하면 디스커버리 서비스로 마이크로 서비스를 등록해두고, 게이트웨이가 디스커버리 서비스를 활용하여 각각의 마이크로 서비스로 연결해준다고 생각하면 된다. FastAPI는 AI 서버를 구축하기 위해 사용되었다고 이해하면 된다.

Nginx를 통해 경유하기 때문에 `{SERVER_URL}/service/`에 요청을 보냈을 때 게이트웨이에 접근할 수 있도록 설정하였다. 여기서 인증 서비스로 접근하려면 `{SERVER_URL}/service/auth/` 이런 식으로 접근할 수 있게 하였다.

# 개발 환경과 배포 환경에서의 라이브러리 차이

각각의 마이크로 서비스들을 로컬에서 개발할 때 다음 라이브러리들이 굳이 필요하지 않다.

{% include code-header.html %}

```groovy
ext {
    set('springCloudVersion', "2024.0.0")
}

dependencyManagement {
    imports {
        mavenBom "org.springframework.cloud:spring-cloud-dependencies:${springCloudVersion}"
    }
}

dependencies {
		implementation 'org.springframework.cloud:spring-cloud-starter-netflix-eureka-client'
		implementation 'org.springframework.cloud:spring-cloud-starter-bus-amqp'
		implementation 'org.springframework.cloud:spring-cloud-starter-config'
}
```

따라서 이러한 라이브러리들을 로컬 환경에서 개발할 때에는 의도적으로 동작하지 않도록 해야 한다. 이는 간단하게 조건문을 추가하여 해결할 수 있었다.

{% include code-header.html %}

```groovy
dependencies {
    if (project.hasProperty("profile") && project.profile == "prod") {
        implementation 'org.springframework.cloud:spring-cloud-starter-netflix-eureka-client'
        implementation 'org.springframework.cloud:spring-cloud-starter-bus-amqp'
        implementation 'org.springframework.cloud:spring-cloud-starter-config'
    }
}
```

그리고 빌드 명령어를 다음과 같이 작성하면 된다.

{% include code-header.html %}

```bash
./gradlew clean bootJar -Pprofile=prod
```

앞 명령어들은 볼 필요가 없고, 중요한 것은 `-Pprofile=prod`이것이다. 이를 통해서 빌드시 설정된 프로필에 따른 라이브러리들을 jar 파일에 포함시킬 수 있다.

# 스웨거 적용

배포 환경에서 각각의 마이크로 서비스들에 스웨거를 접근할 수 있는 방법은 존재하지 않는다. 따라서 이를 게이트웨이에서 통합적으로 관리할 수 있도록 설정해줘야 한다. 좋은 블로그 글이 있어서 참고하여 설정할 수 있었다.

게이트웨이에서 스웨거를 사용하려면 다음 라이브러리를 선언해야 한다.

{% include code-header.html %}

```groovy
implementation 'org.springdoc:springdoc-openapi-starter-webflux-ui:2.8.6'
```

또한 설정 파일에 다음과 같은 설정을 선언하도록 하자.

{% include code-header.html %}

```yaml
springdoc:
  api-docs:
    enabled: true
  swagger-ui:
    enabled: true
    #    path: /api-docs
    # /service/는 nginx 때문에 붙인 설정
    config-url: /service/v3/api-docs/swagger-config
    urls:
      - url: /service/auth/v3/api-docs
        name: Auth-Docs
```

참고로 Nginx 때문에 스웨거의 리다이렉트 기능을 제대로 사용하지 못하고 `{서버주소}/service/swagger-ui/index.html`로 접근할 수 밖에 없다.

마이크로 서비스의 정보 수집을 위한 url 역시도 마찬가지로 `/service/{SERVICE_SWAGGER_URL}`로 접근하도록 해야한다.

## 마이크로 서비스의 빈 등록

마지막으로 마이크로 서비스에 스웨거 설정 빈을 등록해야 한다.

{% include code-header.html %}

```java
@OpenAPIDefinition
@Configuration
public class SwaggerConfig {
    @Bean
    public OpenAPI customOpenAPI(@Value("${openapi.service.url}") String url) {
        return new OpenAPI()
                .servers(List.of(new Server().url(url)))
                // JWT 토큰 사용하는 경우
                .components(new Components().addSecuritySchemes("Bearer",
                        new SecurityScheme().type(SecurityScheme.Type.HTTP)
                        .scheme("bearer").bearerFormat("JWT")))
                .addSecurityItem(new SecurityRequirement().addList("Bearer"))
                .info(new Info().title("API 문서 제목")
                        .description("어떤 API 문서인지 설명")
                        .version("버전 설정 (ex. v0.1.0)"));
    }
}
```

그리고 마이크로 서비스에서는 다음과 같이 설정하도록 하자.

{% include code-header.html %}

```yaml
springdoc:
  api-docs:
    path: /v3/api-docs
  swagger-ui:
    path: /swagger-ui.html

open-api:
  service:
    url: { 배포 도메인 또는 localhost 주소 }
```

---

스프링 클라우드를 활용한 MSA 구축은 색다른 경험이었다. 특히 소켓 서버 로드 벨런싱을 한답시고 공식 문서랑 구현 코드를 살펴보면서 지속성 해싱을 구현하기는 했는데 HTTP 요청에만 해당된다고 해서 시무룩 했던 경험도 있고, 소켓 서버가 불안정 한 것 같아서 따로 Nginx로 리버스 프록시 처리만 해주기도 하였다.

또한 원래 쿠버네티스를 학습해서 최종적으로 적용해보려고 했는데, 시간적 요인 때문에 잘 안되었다. 그리고 MSA 관련 자료를 찾아보던 도중에 쿠버네티스를 활용한 MSA 배포에 관한 블로그 글을 읽을 수 있었다.

여기서 쿠버네티스를 활용하면 디스커버리 서비스를 사용하지 않아도 되며, 다른 언어로 작성되어도 상관이 없는 진정한 의미의 MSA를 구축할 수 있다고 하여 흥미가 생겨서 아마 쿠버네티스를 학습하여서 간단하게 내 로컬에서만이라도 적용해보고 싶다.

# 참고 자료

[**MSA에서의 swagger 적용**](https://velog.io/@cutepassions/MSA%EC%97%90%EC%84%9C%EC%9D%98-swagger-%EC%A0%81%EC%9A%A9)

[**[MSA] 마이크로서비스 배포:SpringCloud vs Kubernetes**](https://bryceyangs.github.io/study/2021/07/28/MSA-SpringCloud-vs-Kubernetes/)
