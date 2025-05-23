---
title: 설정 서버 보안 처리 및 자동 busrefresh

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true
 
date: 2025-05-04
last_modified_at: 2025-05-04
---

# application.yaml vs bootstrap.yaml

설정 서버를 사용할 때 두 가지 방법이 있는데, 하나는 `application.yaml` 파일에서 설정 서버를 명시하는 것이고, 다른 하나는 spring cloud starter bootstrap 의존성을 사용하는 것이다. 해당 의존성을 사용하게 되면 `bootstrap.yaml`이라는 파일에서 설정 서버를 지정하여 설정 정보를 불러올 수 있다.

- `application.yaml`을 사용하는 경우

```yaml
spring:
  application:
    name: ${SERVICE_NAME}
  config:
	  # 이 부분에서 설정 서버의 위치 정의
    import: "optional:configserver:${CONFIG_SERVER_URL:http://127.0.0.1:8888}"
```

- `bootstrap.yaml`을 사용하는 경우

```yaml
spring:
  cloud:
    config:
      uri: ${CONFIG_URL}
      name: ${CONFIG_RESOURCE}
      username: ${CONFIG_USERNAME}
      password: ${CONFIG_PASSWORD}
```

이때 나는 첫 번째 방법을 사용하였는데, 그 이유는 bootstrap 의존성이 상당히 오래된 코드이며, 더 이상 유지보수가 없을 것이라는 정보를 접했기 때문이었다. 하지만 `application.yaml`을 통해 설정 정보를 가져오게 되었을 때 설정 서버의 변경 사항을 반영하려고 busrefresh 요청을 보냈는데도 변경이 반영되지 않는 문제를 겪었다.

## 두 방법의 차이점

이때 GPT에게 이러한 문제를 공유하니 bootstrap 의존성을 사용하는 것을 추천하였다. 그 이유는 bootstrap을 통해 설정 정보를 불러오는 작업은 스프링 어플리케이션 컨텍스트가 초기화되기 이전에 이루어지기 때문이라고 한다. GPT의 정리 표를 보도록 하자.

| 항목 | `bootstrap.yaml` | `application.yaml` |
| --- | --- | --- |
| 로딩 시점 | 컨텍스트 초기화 **이전** | 컨텍스트 초기화 **이후** |
| 주용도 | 외부 설정 소스에 접근하기 위한 메타 설정 | 일반적인 앱 설정 (포트, 로그 등) |
| 설정 예시 | Config URI, 서비스 이름 등 | 포트, DB 연결정보, 로그레벨 등 |
| Config 서버 연동시 필수 여부 | Spring Cloud Config 사용 시 필수 | 아니어도 무방 |

이 표를 보니 아마 `application.yaml` 에서 설정 정보를 불러오는 방식은 컨텍스트 초기화 이후이기 때문에 busrefresh가 제대로 적용이 되지 않는건가 싶다. 

물론 내가 제대로 안써서 그럴 수 있으니 이 부분은 불확실 하긴 하지만, bootstrap 의존성을 사용하면 실제 어플리케이션 내에는 `bootstrap.yaml`에서 설정 서버만 명시하고, 서비스의 모든 설정 정보 자체는 깃허브 같은 중앙 저장소에서 관리할 수 있기 때문에 굳이 busrefresh 반영 문제가 아니더라고 bootstrap 의존성을 사용할 이유는 충분해 보인다.

따라서 아래 모든 주제는 bootstrap 의존성을 이용하여 설정 서버에 접근한다고 가정하고 글을 작성할 것이다.

# 설정 서버 보안 처리

설정 서버를 사용하면서 굉장한 편리함을 느꼈지만, 한 편으로는 배포한 URL만 알 수 있으면 서비스가 사용하는 모든 설정 정보를 알 수 있게 되어 결과적으로 보안에 취약해진다고 생각하였다. 설정 서버에서 키 값을 암호화 처리해줄 수는 있지만, 이는 매번 설정 서버에 HTTP 요청을 보내고, 반환 받은 값을 명시해줘야 하기 때문에 귀찮을 것 같았다.

심심해서 chatGPT를 갈궈본 결과 스프링 시큐리티를 활용하면 Basic 인증 방법을 사용할 수 있게 되기 때문에 보안 처리를 할 수 있다고 해서 한 번 해봤고, 결과는 성공적이었다. 

다음 의존성들을 사용하였다.

```groovy
ext {
    set('springCloudVersion', "2024.0.1")
}

dependencies {
    implementation 'org.springframework.cloud:spring-cloud-config-server'
    implementation 'org.springframework.boot:spring-boot-starter-security'
    implementation 'org.springframework.boot:spring-boot-starter-actuator'
    implementation 'org.springframework.cloud:spring-cloud-starter-bus-amqp'
    testImplementation 'org.springframework.boot:spring-boot-starter-test'
    testRuntimeOnly 'org.junit.platform:junit-platform-launcher'
}

dependencyManagement {
    imports {
        mavenBom "org.springframework.cloud:spring-cloud-dependencies:${springCloudVersion}"
    }
}
```

이렇게 설정하였을 때, 스프링 시큐리티는 기본적으로 요청에 대해서 Basic 인증을 요구하게 된다. 이때 Basic 인증 정보를 `application.yaml`으로 수정할 수 있다. 가령 사용자 이름이 root이고, 비밀번호가 1234라면 다음과 같이 설정하면 된다.

```yaml
spring:
  security:
    user:
      name: root
      password: 1234
```

그러면 이 설정 서버로부터 값을 가져오는 서비스에서는 다음과 같이 설정해주기만 하면 된다.

```yaml
spring:
  cloud:
    config:
      uri: ${CONFIG_SERVER_URL}
      username: root
      password: 1234
```

# Github Actions를 활용한 실시간 설정 변경

MSA를 처음 구축했을 때, 나는 매번 설정 정보가 변경되면 서버에 직접 접속하여 다음과 같은 curl 명령어를 입력하였다.

```yaml
curl -X POST http://localhost:8081/actuator/busrefresh
```

이를 통해서 변경된 설정 정보를 스프링 클라우드 버스를 통해서 실시간 반영하고는 했는데, 이 방법은 귀찮은 방법이었다. 이와 관련하여 어떻게 안될까 생각하던 도중 chatGPT에게 한 번 물어봤고, GPT는 Github Actions를 통해서 설정 정보가 변경되면 이를 반영하도록 할 수 있다고 답변을 주어 한 번 해봤다.

스프링 클라우드 버스가 구축되었다는 가정에서 작성하도록 하겠다. 어플리케이션 설정 파일들이 담긴 비공개 레포지토리에서 다음과 같은 Github Action을 정의하였다.

```yaml
name: Refresh Spring Config Server

on:
  push:
    branches: [ main ]  # 또는 설정 변경 시

jobs:
  refresh-config:
    runs-on: ubuntu-latest
    steps:
      - name: Send POST to actuator bus-refresh
        run: |
          curl --http1.1 -X POST http://52.79.204.137:8081/actuator/busrefresh \
               -H "Content-Type: application/json" \
               -H "Authorization: Basic ${{secrets.BASIC_AUTH_HEADER}}"
```

또한 앞서 설정한 설정 서버의 보안 처리를 위해서 레포지토리 설정에서 설정 서버가 요구하는 아이디 패스워드의 Basic 인증 값을 명시하기만 하면 된다. 

![깃헙 엑션 인증 값 명시](/assets/images/set-server-security-handling-and-automatic-busrefresh_01.png)

참고로 인증 값은 API 테스트 도구에서 통해서 얻을 수 있다. 내가 사용하는 API 테스트 도구인 Talend API 도구의 경우 Add authorization 버튼을 통해서 입력된 아이디 패스워드 값의 Base64 인코딩 값을 얻을 수 있다.

![인증 값 Base64 사용 예시](/assets/images/set-server-security-handling-and-automatic-busrefresh_02.png)

---

오늘은 사실 굉장히 짧고 약간 불확실한 블로그 포스팅인 것 같다. 사실 저번 주 블로그 소재였는데 개인적인 건강 이슈로 병원을 다녀오느라 이제야 작성한다. 늘 건강이 먼저라는 것을 알지만 불안감에 휩싸여서 무리하게 되는 그런 시기인 것 같다.