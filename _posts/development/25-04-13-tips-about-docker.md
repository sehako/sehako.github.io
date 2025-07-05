---
title: 도커 관련된 팁

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true
 
date: 2025-04-13
last_modified_at: 2025-04-13
---

이번 포스팅은 프로젝트에서 도커와 도커 컴포즈를 사용하며 겪은 작은 불편함들을 해결하는 방법을 작성해보도록 하겠다.

# 하나의 도커 파일로 여러 이미지 빌드

나는 이번 프로젝트에서 jar 파일을 직접 배포 서버로 전달하여 해당 서버 내에서 자체적으로 도커 이미지로 만들어서 배포하도록 설계하였다. 중간에 도커 허브를 통해서 CI/CD 환경을 구축하기도 하였는데, 이상하게도 최신 이미지를 받아서 실행해도 구버전으로 되는 문제가 있었고, 해결 방법도 잘 모르겠어서 그냥 첫 번재 방법으로 다시 롤백했다.

아무튼  MSA 환경에서는 각 마이크로 서비스를 모두 CI/CD 해야만 했다. 우리 팀은 찍먹 느낌으로만 하자고 하였기 때문에 최종적으로 CI/CD 대상이 되는 서비스가 4개 정도밖에 안되었지만, 이들 모두 하나하나 도커 파일로 정의하고 컴포즈를 통해 각 도커파일을 지정하는 방식은 나에게 있어서 비효율적으로 다가왔다.

## 도커 파일 ARG 기능

그렇게 찾아보던 도중에 도커 파일에 인자를 전달할 수 있다는 사실을 알게되었다. 이를 통해서 하나의 도커 파일로 여러 jar 파일을 도커 이미지로 만들 수 있게 되었다.

```docker
FROM openjdk:17-jdk

LABEL maintainer="sehako <dhtpgkr1999@gmail.com>"

ARG SERVICE_NAME

ARG JAR_FILE

ENV SERVICE_NAME=$SERVICE_NAME

COPY ./$JAR_FILE app.jar

ENTRYPOINT ["java", "-jar", "/app.jar"]
```

`ARG` 부분이 바로 외부에서 전달한 인자가 되는 것이다. 이를 컴포즈 파일에서 전달하는 것은 다음과 같다. spring-auth.jar 라는 파일을 도커 이미지로 빌드한다고 가정하면 다음과 같이 정의해주면 된다.

```yaml
services:
  spring-auth:
    container_name: spring-auth
    build:
	    # 현재 디레터리에 존재하는 backend 디렉터리에 있는 Dockerfile을 명시
      context: ./backend
      dockerfile: Dockerfile
      args:
        SERVICE_NAME: auth-service
        JAR_FILE: auth-service.jar
    environment:
      - TZ=Asia/Seoul
    networks:
      - compose-network
```

이를 통해서 하나의 도커 파일로 여러 jar 파일을 빌드할 수 있게 되었다.

# 도커 컴포즈 파일 분리

도커 컴포즈로 배포를 할 때, MSA + 모니터링 툴 적용까지 하였기 때문에 총 15개의 컨테이너가 하나의 서버에서 실행되었고, 결과적으로 컴포즈 파일이 약 400줄 정도의 라인으로 구성되었다.

이렇게 되니 컴포즈 파일의 특정 부분만 수정하는 것에 불편함이 있었고, 하나의 컨테이너만 내리고 싶었는데 실수로 서비스를 명시하지 않아서 전체 컨테이너를 내리는 경우도 종종 있었다. 

따라서 데이터베이스, 모니터링 도구, 카프카 같은 메시지 큐, 스프링 부트 어플리케이션을 분리하여 도커 컴포즈를 통해서 컨테이너로 실행시키는 동시에 모두 같은 네트워크로 묶어서 서로간의 통신이 가능하도록 하면 좋겠다는 생각을 하게 되었다.

## -f 옵션

이러던 중 알게 된 것이 바로 도커 컴포즈의 명령어 중에 `-f` 옵션이다. `-f` 옵션은 특정 컴포즈 파일을 특정하여 실행할 수 있는 옵션인데, 만약에 내가 `docker-compose.mq.yml`로 컴포즈 파일 이름을 지었으면 다음과 같은 명령어를 통해서 해당 컴포즈 파일을 올릴 수 있다.

```bash
docker-compose -f docker-compose.mq.yml up -d
```

## 외부 네트워크 설정

또한 컴포즈 파일에서 외부 네트워크를 사용하도록 설정하였다. 컴포즈를 띄울 때 네트워크를 자동으로 만들어주지 말고, 다음과 같이 도커 네트워크를 정의하자.

```bash
docker network create compose-network
```

그러면 해당되는 네트워크를 다음과 같이 사용하도록 설정하면 된다.

```yaml
networks:
  compose-network:
    external: true
```

결과적으로 각 컨테이너를 역할별로 나누고, 이를 모두 같은 네트워크에서 구동되도록 외부 네트워크를 사용하게 만들었다. 이 방식의 장점은 우선 파일별로 나눠져 있어서 관리가 편하고, 특정 역할을 하는 컨테이너만 한 번에 내릴 수 있다는 것 같다. 

또한 스프링 부트의 경우 데이터베이스 같은 필요한 외부 프로그램이 준비가 되지 않으면 아예 실행이 안되는 경우가 있는데, 이를 위해서 다음과 같이 실행 순서를 명시해야만 했다.

```yaml
services:
  spring-eureka:
		# ...
    depends_on:
      mysql:
        condition: service_healthy
    networks:
      - compose-network  
  
  mysql:
    container_name: mysql
    image: mysql:latest
    environment:
      TZ: Asia/Seoul
    volumes:
      - mysql_data:/var/lib/mysql
    ports:
      - 3306:3306
    healthcheck:
      test: ['CMD', 'mysqladmin', 'ping', '-h', 'localhost', '-u', 'root', '-proot']
      interval: 5s
      timeout: 10s
      retries: 5
    networks:
      - compose-network
```

하지만 컴포즈 파일을 분리하여 실행하게 되면 이러한 헬스 체크를 아예 고려하지 않고, 어플리케이션 내에서 필요한 실행 순서만 명시해주면 되기 때문에 각 파일의 길이도 짧아지게 되는 장점도 있다고 생각한다.

---

프로젝트가 마무리 되었다. 이번 프로젝트는 Drawaing이라는 AI와 대결하는 게임을 개발하는 것이 목적이었는데, 나는 개발자 포지션보다는 인프라 포지션에 더더욱 가까웠기 때문에 다양한 툴들을 직접 적용해보는 것이 가능하였다. 

EC2를 별도로 하나 만들어서 젠킨스를 이용한 CI/CD 서버로 활용하기도 하였고, AI를 활용한 FastAPI 서버가 배포 서버의 CPU 사용을 70프로 정도 차지하였기 때문에 GCP를 활용하여 AI 서버를 별도로 분리하기도 하였다. 

또한 프로메테우스와 그라파나를 활용하여 모니터링 툴을 직접 붙여도 보고, Promtail과 Loki를 활용하여 로그도 직접 수집해보았다. 확실히 로그 수집을 해놓으니 프론트엔드 개발자와 개발할 때, 백엔드 개발자가 로그를 보면서 무엇이 문제인지 말해주니 금방 해결되는 부분도 있어서 나름 뿌듯했다.

물론 아쉬움도 있었는데, 구직 활동과 나의 개인적인 게으름 덕분에 쿠버네티스를 학습하여 프로젝트에 적용해보지 못했다는 것이다. 그래서 쿠버네티스는 차차 천천히 학습하면서 나만의 미니 프로젝트에 적용해보는 것을 생각하고 있다.

인프라 분야도 깊게 들어가면 굉장히 심오하겠지만, 두 번의 프로젝트 모두 기간이 촉박하였기 때문에 인터넷에 필요한 것을 검색한 후 적용해보고, 여기서 오는 다양한 문제들을 chatGPT와 다른 블로그를 참고하여 해결하면서 배워나갔다. 물론 나중에 이를 좀 더 깊이 있게 배워보고는 싶지만, 그게 언제가 될 지는 나도 잘 모른다.