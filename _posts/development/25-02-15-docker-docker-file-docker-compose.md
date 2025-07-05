---
title: 도커와 도커 컴포즈

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true
 
date: 2025-02-15
last_modified_at: 2025-02-15
---

최근에 진행하는 프로젝트에서 처음 해보는 서버 배포와 CI/CD 부분을 맡았다. 따라서 이 포스팅은 인프라 입문이 막막한 사람들을 위해 인프라에 대해 하나도 몰랐던 내가 어떤 과정을 거쳐 배포를 하였는지 작성하는 시리즈가 될 것이다. 그 첫 번째로 도커와 도커 컴포즈가 되겠다.

# 도커

우선 도커가 무엇인지부터 간단하게 알고 넘어가도록 하자 도커는 Go 언어로 작성된 오픈소스 프로젝트이다. 기존의 가상화 기술은 일반 호스트에 비해 성능의 손실이 발생하였지만, 도커는 가상화된 공간 생성을 위해 리눅스 자체 기능인 chroot, namespace, cgroup을 사용하여 프로세스 단위의 격리 환경을 만들고 컨테이너에서 커널이 필요하면 호스트와 공유하여 사용한다. 

![도커 vs 가상 머신](https://velog.velcdn.com/images/gkrdh99/post/6bf1dd0b-9016-4625-84ba-38b0eea0a273/image.png)

그렇기 때문에 컨테이너에는 어플리케이션 구동을 위한 라이브러리 및 실행 파일만 존재하게 되어 기존의 가상화 기술보다 배포 시간이 빠르고 가상화된 공간을 위한 성능 손실이 거의 없다는 장점이 있다.

## 도커를 사용해야 하는 이유

**편리한 개발과 배포**

컨테이너는 도커를 실행하는 호스트 OS와는 완전히 격리된 공간으로 취급된다. 따라서 컨테이너 내부에서 여러 작업과 설정을 한 다음에 이를 배포환경에 이미지로 전달만 한다면 새로운 배포 환경에는 도커만 있으면 바로 실행이 가능하고, 여러 배포서버에 올리고자 하면 단순히 이미지를 전달하고, 도커에서 실행시키기만 하면 된다.

**어플리케이션의 독립성과 확장성 보장**

최근 대규모 프로젝트는 MSA(Micro Service Architecture) 구조를 차용하는 경우가 있다. MSA는 각각의 단일한 기능을 수행하는 여러 어플리케이션을 두어 특정 기능에 부하가 발생하면 해당 기능을 수행하는 어플리케이션을 스케일 아웃하여 효과적으로 처리할 수 있는 등의 이점이 있다.

여기에 도커를 도입하면 특정 기능을 수행하는 어플리케이션 컨테이너만 늘리는 방식으로 해결이 가능하다. 이 방식은 나도 아직 잘 모르는 도커 스웜 모드나 쿠버네티스 등의 컨테이너 오케스트레이션 플랫폼을 사용한다고 한다.

## 도커 파일

우리는 한 가지 중요한 문제를 고려해야 한다. 바로, 내가 개발한 애플리케이션을 도커 이미지를 만들어 컨테이너로 실행하는 방법이다.

단순히 컨테이너를 띄운 후, 배포에 필요한 모든 설정을 하고 이를 이미지로 만드는 방법도 있다. 하지만 이 방법만 존재했다면, 도커를 활용한 배포가 쉽다고 말하기는 어려웠을 것이다.

이러한 문제를 해결하기 위해 도커는 Dockerfile을 제공한다. Dockerfile은 애플리케이션을 이미지로 만들 때 필요한 각종 명령어를 기록한 파일이다. 도커는 build 명령어를 통해 이 파일을 읽고, 완성된 이미지를 생성할 수 있다.

다음은 스프링 애플리케이션을 배포할 때 사용한 Dockerfile의 예시다.

```Dockerfile
FROM openjdk:17-jdk
LABEL maintainer "sehako"

# JAR 파일 경로 (build 후 target 폴더 안에 위치한다고 가정)
ARG JAR_FILE=./build/libs/app.jar

# JAR 파일 복사
COPY ${JAR_FILE} app.jar

# 앱 실행 명령어 추가 (필요시)
ENTRYPOINT ["java", "-jar", "/app.jar"]
```

`build` 명령어를 내리면 `FROM`부터 한 줄 씩 밑으로 수행해 나간다.

### 도커 파일 명령어 모음

도커 파일을 잘 작성하기 위해서는 명령어들을 알아야 한다. 간단하게 정리해보았다.

- `FROM`: 생성할 이미지의 베이스가 되는 이미지를 뜻한다. 스프링 부트의 경우에는 jdk 환경에서 작동하므로 jdk가 베이스 이미지가 되는 것이다.

- `LABEL`: 생성할 이미지에 메타 데이터를 추가할 수 있다. 위 명령어의 경우에는 이미지를 생성한 나의 닉네임을 넣었다. 만약 이메일도 넣고 싶으면 다음과 같이 표현하기도 한다.

```Dockerfile
LABEL maintainer "sehako <dhtpgkr1999@gmail.com>"
```

- `RUN`: 이미지를 만들기 위해서 컨테이너 내부에서 실행시킬 명령어를 정의할 수 있다. 이를 설명하기 위해서 내가 정의한 젠킨스 도커 파일을 보여주도록 하겠다.

```Dockerfile
FROM jenkins/jenkins:2.492.1-lts-jdk17

USER root

# Docker CLI 및 AWS CLI 설치
RUN apt-get update && apt-get install -y \
    ca-certificates curl gnupg unzip lsb-release && \
    install -m 0755 -d /etc/apt/keyrings && \
    curl -fsSL https://download.docker.com/linux/debian/gpg | tee /etc/apt/keyrings/docker.asc > /dev/null && \
    chmod a+r /etc/apt/keyrings/docker.asc && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian bookworm stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null && \
    apt-get update && \
    apt-get install -y docker-ce-cli && \
    curl "https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip" -o "/tmp/awscliv2.zip" && \
    unzip /tmp/awscliv2.zip -d /tmp && \
    /tmp/aws/install && \
    rm -rf /var/lib/apt/lists/* /tmp/awscliv2.zip /tmp/aws

# AWS CLI 버전 확인 (디버깅용, 선택사항)
RUN aws --version

# Jenkins 실행
CMD ["/usr/local/bin/jenkins.sh"]
```

나는 CI/CD를 위해서 빌드한 스프링 부트 jar 파일을 도커 이미지로 만들어 도커 허브에 업로드하는 방식을 선택하였다. 

이 과정에서 도커 명령어를 사용하기 위해서는 컨테이너 내부에 도커 CLI를 설치하고 호스트의 도커 소켓 파일과 젠킨스 컨테이너의 도커 소켓 파일을 마운트 시켜야 하는데, 이를 위해서 `RUN` 명령어를 통해 젠킨스 이미지에 미리 도커 CLI를 설치하였다. 

참고로 이미지 빌드 과정에서는 추가 명령어 입력이 불가능 하기 때문에 모든 설치에 대해서 -y 옵션을 줘야 한다.

- `USER`: 이 명령어를 사용하여 사용자를 지정하면, 이후 실행될 명령어는 해당 사용자의 권한에 따라서 실행된다. 위의 젠킨스 도커 파일의 경우 설치 권한을 얻기 위해서 root 사용자를 명시하였다.

- `ARG`: 도커 파일에서 사용할 변수를 지정한다. 

- `COPY`: 로컬 디렉터리에서 읽어 들인 컨텍스트로부터 이미지에 파일을 복사하는 역할을 한다.

- `ADD`: 파일을 이미지에 추가하는 명령어다. 추가하는 파일은 도커 파일이 위치한 디렉터리인 컨텍스트에서 가져온다. `COPY` 명령어와의 차이점은 `ADD` 명령어는 외부 URL 및 tar 파일에서도 파일을 추가할 수 있다. `COPY`보다 더 좋아보이지만 정확하게 어떠한 파일이 추가될 지 알 수 없기 때문에 정말 필요할 때만 사용해야 한다.

- `WORKDIR`: 명령어를 실행할 디렉터리를 나타낸다. 이는 `cd` 명령어를 입력하는 것이라고 생각하면 된다.

- `EXPOSE`: 생성된 이미지에서 노출할 포트를 설정한다. 그러나 반드시 이 포트가 호스트의 포트와 바인딩 되는 것은 아니고, 컨테이너가 지정하는 포트를 사용할 것임을 명시하는 것이다. 호스트의 포트와 바인딩 하려면 실행할 때 `-p` 옵션을 사용하면 된다.

- `CMD`: 컨테이너가 시작될 때 실행할 기본 명령어를 설정하며, Dockerfile에서 한 번만 사용할 수 있다. 하지만 `docker run` 실행 시 새로운 명령어를 입력하면 CMD는 완전히 덮어씌워진다.

- `ENTRYPOINT`: 필수적으로 실행되어야 할 명령어를 설정하며, `docker run`의 인자가 ENTRYPOINT의 인자로 추가되어 실행된다.

## 도커 컴포즈

하나의 EC2에서 스프링 부트 애플리케이션을 배포하려면, 스프링 부트 컨테이너뿐만 아니라 데이터베이스 컨테이너도 함께 실행해야 한다. 하지만 개별 컨테이너를 하나씩 실행하는 방식은 번거롭고 비효율적이다. 

이를 해결하기 위해 도커에서는 Docker Compose라는 플러그인을 제공한다. Docker Compose를 사용하면 여러 개의 컨테이너를 하나의 애플리케이션처럼 구성하여, 한 번의 명령어로 실행 및 관리할 수 있다.

윈도우나 맥OS X의 경우에는 도커 데스크탑을 다운받으면 자동으로 도커 컴포즈도 설치되지만, 우분투의 CLI 환경에서는 다음 명령어를 사용해야 한다.

```sh
sudo apt-get update
sudo apt-get install docker-compose-plugin
```

도커 컴포즈는 기본적으로 정의된 yaml 파일을 읽어들여 여러 컨테이너를 실행할 수 있다. mysql, 스프링 부트, 젠킨스를 도커 컴포즈에 정의한 예시를 보자.

```yaml
services:
  mysql:
    container_name: mysql
    image: mysql:latest
    environment:
      MYSQL_ROOT_PASSWORD: "1234"
      TZ: Asia/Seoul
    volumes:
      - mysql_data:/var/lib/mysql  # MySQL 데이터를 Docker Volume으로 저장
      - ./ddl.sql:/docker-entrypoint-initdb.d/init.sql # 도커 마운트를 이용하여 서비스에 필요한 ddl을 컨테이너 초기 실행 때 정의하도록 함
    ports:
      - 3306:3306
    healthcheck:
      test: ['CMD', 'mysqladmin', 'ping', '-h', 'localhost', '-u', 'root', '-proot']
      interval: 5s
      timeout: 10s
      retries: 5
    networks:
      - compose-network

  spring:
    container_name: spring
    image: sehako/example_image:laatest
    env_file:
      - ./.env
    ports:
      - 8080:8080
    depends_on:
      mysql:
        condition: service_healthy
    networks:
      - compose-network

  jenkins:
    container_name: jenkins
      #    image: jenkins/jenkins:2.492.1-lts-jdk17
    build:
      context: .
      dockerfile: jenkins.Dockerfile
    restart: unless-stopped
    environment:
      - TZ=Asia/Seoul
      - JENKINS_OPTS=--httpPort=9000 --prefix=/jenkins
    ports:
      - 9000:9000
    volumes:
      - jenkins_home:/var/jenkins_home  # Jenkins 데이터를 Docker Volume으로 저장
      - /var/run/docker.sock:/var/run/docker.sock
      # - ~/.ssh:/root/.ssh  # SSH 키 공유
    networks:
      - compose-network

networks:
  compose-network:
    driver: bridge
```

참고로 젠킨스는 기본적으로 8080포트를 사용하기 때문에 나는 9000번으로 바꿔서 진행하였다. 아무튼 도커 컴포즈 파일을 작성할 때 초기에는 yaml 파일에 버전을 정의해줘야 했다. 

```yaml
version: 3.0
```

하지만 도커 컴포즈가 최신화 되면서 이제는 yaml 파일에 버전을 명시하지 않고 바로 `services`로 시작할 수 있다. 위와 같이 yaml 파일을 정의한 다음에는 해당 파일이 위치한 디렉토리에서 다음과 같은 명령어를 입력하면 된다.

```sh
docker-compose up -d
```

또한 특정 컨테이너만 지정해서 실행시킬 수 있다. 

```sh
docker-compose up -d spring
```

### 도커 컴포즈 옵션

도커 컴포즈를 이용하여 컨테이너를 여러 개 실행시킬 때, 각 컨테이너에 여러 설정을 해줄 수 있다. 

**image**

서비스의 컨테이너를 생성할 때 쓰일 이미지의 이름을 설정한다. 만약 이미지가 존재하지 않으면 저장소에서 자동으로 내려받는다.

**container_name**

서비스의 컨테이너를 띄울 때 컨테이너의 이름을 지정해 줄 수 있는데, 이는 도커 명령어를 사용할 때 컨테이너를 가리킬 수 있는 이름이 된다.

**build**

정의된 도커 파일을 실행하여 이미지를 빌드하여 서비스의 컨테이너를 생성하도록 한다. 이때 도커 파일에 사용될 컨텍스트나 도커 파일의 이름 등을 지정할 수 있다.

```yaml
build:
  context: .
  dockerfile: jenkins.Dockerfile
```

**enviroment**

컨테이너를 실행할 때 내부에서 사용할 환경변수를 지정하는 옵션이다. mysql의 경우에는 루트 비밀번호를 초기에 설정해줘야 하는데, 이를 환경 변수를 통해서 정의할 수 있다.

```yaml
environment:
  MYSQL_ROOT_PASSWORD: 1234
```

**env_file**

스프링 부트의 경우에는 다양한 환경 변수가 존재하고, 이를 .env 파일로 관리하는 것이 일반적이다. 도커 컴포즈를 이용하여 컨테이너를 만들 때 이런 환경 변수들을 하나하나 설정하지 않고 환경 변수 파일을 불러오는 방식을 사용할 수 있다.

```yaml
env_file:
  - ./.env 
```

이러면 현재 디렉터리 내의 .env 파일을 컨테이너가 빌드될 때 불러오게 된다.

**healthcheck**

어떤 컨테이너의 상태를 명시하기 위한 부분이다.

```yaml
healthcheck:
  test: ['CMD', 'mysqladmin', 'ping', '-h', 'localhost', '-u', 'root', '-proot']
  interval: 5s
  timeout: 10s
  retries: 5
```

위 mysql 컨테이너의 명령어를 보면 mysql이 완전히 실행되었을 때 ping 명령어가 동작하면 `service_healthy` 상태가 된다. 이를 이용하여 특정 컨테이너가 완전히 생성되었을 때 이에 의존하는 컨테이너를 실행시키도록 만들 수 있다.

**depends_on**

컨테이너의 실행 순서를 명시하거나, 특정 컨테이너가 다 실행되었을 때 실행되도록 할 수 있는 의존관계 설정이다. 스프링 부트의 경우에는 mysql 컨테이너가 완전히 실행된 상태일 때 실행되어야 예외가 발생하지 않고 정상적으로 동작한다. 따라서 스프링 부트에 `depends_on` 옵션을 사용하여야 한다.

```yaml
depends_on:
  mysql:
  condition: service_healthy
```

단순히 실행되는 순서만을 정의하고 싶다면 다음과 같이 작성하면 된다. 이 경우에는 실행의 순서만 보장하고 컨테이너의 준비가 완료되는 순서는 보장되지 않는다.

```yaml
depends_on:
  - mysql
```

**ports**

`docker run`의 `-p` 옵션과 같으며, 서비스 컨테이너를 개방할 포트를 설정한다. 만약 외부에서의 접속을 차단하고, 컨테이너 내부 통신 용도로만 사용하고 싶다면 다음과 같이 작성하면 된다.

```yaml
ports:
  - 127.0.0.1:3306:3306
```

**volumes**

호스트의 특정 파일또는 디렉터리를 마운트 시키거나, 도커 볼륨 기능으로 데이터를 영속적으로 저장하게 할 수 있다. 젠킨스의 설정을 보도록 하자.

```yaml
volumes:
  - jenkins_home:/var/jenkins_home  # Jenkins 데이터를 Docker Volume으로 저장
  - /var/run/docker.sock:/var/run/docker.sock
```

볼륨에 지정된 옵션 중에서 첫 번째 부분은 컨테이너에서 `/var/jenkins_home` 경로의 해당 디렉터리를 포함한 하위 파일들을 `jenkins_home`이라는 도커 볼륨에 저장하도록 한 것이다. 만약 그러한 도커 볼륨이 없다면 도커는 해당 이름을 가진 도커 볼륨을 생성하여 저장한다. 그리고 도커 컴포즈 내에 다음과 같은 볼륨 지정 명령어가 있어야 한다.

```yaml
volumes:
  jenkins_home:  # Jenkins 홈 디렉터리 저장
```

두 번째 줄은 도커 마운트로, 호스트의 특정 파일과 도커 컨테이너 내의 특정 파일을 마운트 시킨다. 이때 호스트에 파일이 없다면 마운트가 제대로 되지 않으므로 호스트에 파일이 있어야 한다. 위 설정이 바로 앞서 언급한 젠킨스 내에서 도커 명령어를 실행하기 위한 도커 소켓 파일 마운트 설정 부분이다.

### 컨테이너 간의 통신

도커 컴포즈를 통해 생성한 컨테이너 간 통신을 위해서는 네트워크를 동일하게 맞춰줄 필요가 있다. 그리고 컨테이너간 통신을 위해서 `localhost`가 아닌 컴포즈를 작성할 때 명시한 서비스 이름을 지정해 줘야 한다.

예를 들어 스프링 부트 어플리케이션을 띄운 컨테이너에 내부적으로 접근하고 싶다면 `http://spring:8080`이라고 명시해야 한다.

가끔 컨테이너 이름과 헷갈리는 경우가 있었는데, 컨테이너 이름은 앞서 설명했듯이 도커 명령어를 실행할 때 컨테이너를 가리키는 역할이 되는 것이고, 서비스 이름은 각 컨테이너 간 통신을 할 때 사용하는 일종의 DNS가 되는 것이다.

## 도커 & 도커 컴포즈에서 자주 사용되는 명령어들

도커와 도커 컴포즈 기능을 사용할 때 자주 사용한 명령어들을 적어보았다.

```sh
docker run --name {CONTAINER_NAME} --net {CONTAINER_NETWORK} -p {HOST_PORT:PORT} -d {IMAGE_NAME:TAG}
```

단일 도커 컨테이너를 실행할 때 가장 많이 사용되는 명령어이다. 일반적으로 `--name`과 `-p`를 주로 사용하고 이를 `-d` 옵션으로 실행시키는 구조이다. `-d` 옵션은 실행하고자 하는 도커 컨테이너를 백그라운드에서 실행한다는 것인데, 이 옵션을 주지 않으면 포그라운드에서 실행되기 때문에 컨테이너가 실행되는 과정의 로그를 보고싶은 것이 아니라면 사실상 거의 필수 옵션이다.

만약 현재 호스트에 지정된 도커 이미지가 없다면 자동으로 이미지를 가져와 컨테이너를 생성한다.

```sh
docker build --rm -t {IMAGE_NAME:TAG} .
```

현재 디렉터리에 위치한 도커 파일을 이용하여 이미지를 빌드하는 명령어이다. 위 명령어는 현재 디렉터리에 위치한 도커 파일을 이미지로 만드는 명령어이다. `--rm` 옵션의 경우에는 이미지 생성에 성공했을 경우에 임시 컨테이너를 삭제하는 옵션이다.

```sh
docker push ${IMAGE_NAME}:latest
```

도커와 현재 cli 환경이 로그인 되었다고 가정했을 때, 특정 레포지토리의 이미지를 도커 허브에 업로드 하는 명령어다. CI/CD를 할 때 이 명령어를 사용할 것이다. 도커 허브에 이미지를 업로드하는 자세한 과정은 다음 글을 참고하면 좋을 것 같다. ([[docker] docker hub에 image 올리기](https://velog.io/@eoveol/docker-docker-hub%EC%97%90-image-%EC%98%AC%EB%A6%AC%EA%B8%B0))

```sh
docker ps | docker-compose ps
```

```sh
docker rmi {IMAGE_ID}
```

이미지를 삭제하는 명령어로, 컨테이너가 비활성화 된 상태여야 한다.

현재 실행 중인 도커 컨테이너의 목록을 볼 수 있다.

```sh
docker logs [-f] [CONTAINER_NAME] | docker-compose logs [-f] [SERVICE_NAME]
```

현재 실행중인 컨테이너의 터미널 출력 로그를 볼 수 있다. 이때 아무런 컨테이너 이름을 주지 않으면 현재 실행되는 컨테이너들의 모든 로그가 출력되고, `-f` 옵션을 주면 현재 실행 중인 컨테이너의 실시간 출력 로그를 볼 수 있게 된다.

```sh
docker-compose up [-d] [--no-deps] [--build] [SERVICE_NAME]
```

명시된 도커 컴포즈 파일을 이용하여 여러 컨테이너를 실행시킬 때 사용한다. 이때 `--no-deps` 옵션은 특정 서비스를 지정하여 사용하는 경우가 많은데, 이는 의존성이 있는 컨테이너를 의존성 없이 실행시키도록 한다.

그리고 `--build` 옵션은 전체 서비스 또는 특정 컨테이너를 실행할 때 빌드를 수행하도록 하는 옵션이다. 이 명령어도 CI/CD를 할 때 활용할 것이다.

```sh
docker-compose down [SERVICE_NAME]
```

특정 서비스 또는 전체 서비스를 도커 컨테이너에서 지울 때 사용한다. 

---

자주 사용하는 명령어는 아마 이 정도인 것 같다.


# 실습 자료

마지막으로 내가 이번에 배포를 진행하면서 사용한 파일들을 조금 다듬어서 공유하도록 하겠다. 컨테이너는 nginx, spring boot, mysql, mongodb, redis, jenkins이고, README도 간단하게 작성했으니 앞으로 나올 시리즈를 이 도구를 활용하여 작성할 예정이다.

이 자료는 백엔드 배포와 CI/CD 입문을 위한 기본적인 틀이지 않을까 싶다.

[다운로드 링크](https://drive.google.com/file/d/1WaVRYkistIRGLThTEQ6F8Pp8Wy0_X2MO/view)

---

생각보다 포스트가 짧은 것 같지만 할 때는 정말 막막했던 것 같다. 이 글을 보고 배포를 처음 시작하는 사람들에게 좋은 가이드가 되었으면 좋겠다. 다음 포스팅은 nginx 관련 내용이 될 것이다.

# 참고 자료

[**시작하세요! 도커/쿠버네티스 - 용찬호**](https://product.kyobobook.co.kr/detail/S000001766450)

[**Docker Vs Virtual Machines**](https://www.linkedin.com/pulse/docker-vs-virtual-machines-ashish-yadav)

[**[docker] docker hub에 image 올리기**](https://velog.io/@eoveol/docker-docker-hub%EC%97%90-image-%EC%98%AC%EB%A6%AC%EA%B8%B0)