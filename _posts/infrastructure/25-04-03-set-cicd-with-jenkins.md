---
title: 젠킨스를 이용한 CI/CD 구축

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true

date: 2025-04-03
last_modified_at: 2025-04-03
---

CI/CD는 지속적 통합(Continuous Integration)과 지속적인 전달 혹은 배포(Continuous Delivery and/or Deployment)를 뜻한다. 전달과 배포의 차이점은 변경된 어플리케이션이 프로덕션 환경까지 반영되는 지 안되는 지 이다.

CI/CD를 쉽게 말하자면 어플리케이션의 변경 사항을 감지하고, 이런 변경 사항을 자동으로 배포한다는 것이다. 나의 경우 젠킨스를 이용하여 CI/CD를 구축하였다. 참고로 젠킨스를 사용한 이유는 다음과 같다.

- 오픈 소스라 무료로 사용할 수 있고,
- 수많은 플러그인이 제공되며,
- 오래되고 널리 쓰인 도구라 참고할 자료가 많다.

그리고 무엇보다도, 그냥 사용해보고 싶었다. 자 그럼 EC2에 젠킨스와 스프링 부트를 도커 컨테이너로 실행하는 것부터, 그리고 CI/CD까지 단계적으로 알아보도록 하자.

참고로 프로젝트의 원할한 CICD 목적으로 gradle bootjar를 실행시키면 app.jar가 나오도록 설정하였다.

{% include code-header.html %}

```groovy
bootJar {
    archiveFileName = 'app.jar'
}
```

jar 파일의 이름을 프로젝트의 이름으로 빌드되도록 설정할 수도 있다.

{% include code-header.html %}

```groovy
bootJar {
    archiveFileName = "${rootProject.name}.jar"
}
```

# EC2 구축

젠킨스를 사용하기에 앞서 EC2가 필요하다. 젠킨스와 형상 관리 툴을 연동하는 지속적 통합(CI) 작업 까지는 ngrok 같은 localhost를 외부에서 접근이 가능하게 만드는 툴 같은 것을 이용하면 어떻게든 될 것 같았지만, 문제는 지속적 배포(CD)였다.

어떻게 안될까 고민을 조금 하다가 그냥 EC2를 생성하는 것이 속편할 것 같아서 앞으로의 포스팅은 EC2에 앞서 첨부한 실습 자료에서 데이터베이스 부분만 제외하고 nginx, 스프링 부트, 젠킨스만 컨테이너로 실행 하도록 하겠다.

EC2는 다음 [블로그](https://olrlobt.tistory.com/83)를 참고하여 생성하였고, 이 중에서 보안 그룹 부분은 HTTP/HTTPS가 아닌 스프링 부트와 젠킨스 자체 포트인 8080과 9000번에 접근이 가능하도록 하였다.

![보안 그룹 설정](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/01.png)

또한 여러 어플리케이션이 띄워지면 메모리가 부족해질 수 있으니 혹시 몰라 스왑 메모리 설정도 해주었다. 다음 [블로그](https://diary-developer.tistory.com/32)를 참고하도록 하자.

```bash
sudo dd if=/dev/zero of=/swapfile bs=128M count=16
```

```bash
sudo chmod 600 /swapfile
```

```bash
sudo mkswap /swapfile
```

```bash
sudo swapon /swapfile
```

```bash
sudo vim /etc/fstab
```

```bash
# 파일 최하단 아래에 추가
LABEL=SWAP      /swapfile       swap    swap defaults 0 0
```

# 컴포즈 파일 수정 및 EC2에 전송

EC2에 띄우기에 앞서 nginx나 데이터베이스는 필요가 없으니 컴포즈 파일을 다음과 같이 수정하였다.

{% include code-header.html %}

```yaml
services:
  spring:
    container_name: spring
    # image: YOUR_DOCKER_NAME/YOUR_DOCKER_REGISTRY:TAG
    build:
      context: .
      dockerfile: spring.Dockerfile
    environment:
      - TZ=Asia/Seoul
    # 환경변수 파일이 있는 경우
    # env_file:
    #   - ./.env
    ports:
      - 8080:8080
    restart: always # <- 컨테이너 자동 재시작 설정
    networks:
      - compose-network

  jenkins:
    container_name:
      jenkins
      #    image: jenkins/jenkins:2.492.1-lts-jdk17
    build:
      context: .
      dockerfile: jenkins.Dockerfile
    restart: unless-stopped
    environment:
      - TZ=Asia/Seoul
      - JENKINS_OPTS=--httpPort=9000
    ports:
      - 9000:9000
    volumes:
      - jenkins_home:/var/jenkins_home # Jenkins 데이터를 Docker Volume으로 저장
      # - /var/run/docker.sock:/var/run/docker.sock
      # - ~/.ssh:/root/.ssh  # SSH 키 공유
    networks:
      - compose-network

networks:
  compose-network:
    driver: bridge
volumes:
  jenkins_home: # Jenkins 홈 디렉터리 저장
```

스프링은 각자 테스트 목적의 깃허브 레포지토리를 생성하여 사용하도록 하자. 그리고 젠킨스 도커 파일은 다음과 같다.

{% include code-header.html %}

```dockerfile
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

참고로 젠킨스의 버전이 낮으면 플러그인이 제대로 설치가 안될 수 있다고 한다. 따라서 플러그인 설치 중 오류가 발생한다면 위 도커 파일에서 젠킨스의 버전을 최신 버전으로 만들도록 하자.

변경한 실습 자료 전체를 EC2에 전송하도록 하자. (scp 명령어 [블로그](https://dejavuqa.tistory.com/358) 참고)

{% include code-header.html %}

```bash
scp -i {EC2 pem 키} -r ./DeploySet {사용자이름}@{서버주소}:{경로}
```

## 도커 & 도커 컴포즈 설치

EC2를 우분투로 생성하였을 거라고 가정하고 다음 명령어들을 입력하여 도커와 도커 컴포즈를 설치하도록 하자.

{% include code-header.html %}

```bash
sudo apt update
```

{% include code-header.html %}

```bash
sudo apt install apt-transport-https ca-certificates curl software-properties-common -y
```

{% include code-header.html %}

```bash
sudo wget -qO- http://get.docker.com/ | sh
```

{% include code-header.html %}

```bash
sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
```

{% include code-header.html %}

```bash
sudo chmod +x /usr/local/bin/docker-compose
```

이제 다음 명령어를 실행하면 nginx, 스프링 부트, 젠킨스가 컨테이너로 생성될 것이다.

{% include code-header.html %}

```bash
sudo docker-compose up -d
```

# 젠킨스 설정

이제 젠킨스가 도커 컨테이너로 잘 실행되었다면, `{EC2 접속 주소}:9000`으로 접근이 가능할 것이다. 그러면 다음과 같은 화면이 나타난다.

![최초 젠킨스 접속 화면](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/02.png)

젠킨스에 최초로 접속하면 관리자 비밀번호를 요구한다. 이는 젠킨스를 최초 실행할 때 나오는 값이므로 다음 명령어를 입력하여 확인하도록 하자.

{% include code-header.html %}

```bash
sudo docker-compose logs jenkins
```

```
jenkins  | *************************************************************
jenkins  | *************************************************************
jenkins  | *************************************************************
jenkins  |
jenkins  | Jenkins initial setup is required. An admin user has been created and a password generated.
jenkins  | Please use the following password to proceed to installation:
jenkins  |
jenkins  | {관리자 비밀번호}
jenkins  |
jenkins  | This may also be found at: /var/jenkins_home/secrets/initialAdminPassword
jenkins  |
jenkins  | *************************************************************
jenkins  | *************************************************************
jenkins  | *************************************************************
```

비밀번호를 입력하면 다음과 같은 화면이 나타난다.

![플러그인 설치](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/03.png)

그냥 젠킨스가 제시하는 플러그인을 설치하도록 하자. 저렇게 설치를 하고도 더 설치해야 한다. 설치를 완료하면 계정을 생성하는 창이 나타나게 된다.

![계정 설정](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/04.png)

실제 배포 서버라면 이 부분을 잘 만들어야 겠지만 지금은 그냥 admin/admin으로 통일하도록 하겠다.

## 플러그인 설치

CI/CD를 위해 젠킨스에서 지원하는 플러그인을 설치하도록 하자.

- [Generic Webhook Trigger](https://plugins.jenkins.io/generic-webhook-trigger)
- [SSH Agent](https://plugins.jenkins.io/ssh-agent)

## 깃허브와의 연동

파이프라인 구축을 하기 전에 우선 깃허브와 연동부터 하도록 하자. 깃허브에서 [토큰](https://github.com/settings/tokens)을 발급받고 이를 Credentials에 등록하도록 하자.

![깃허브 토큰 발급](/assets/images/set-cicd-with-jenkins_05.png)

여기서 나온 텍스트를 젠킨스의 Credentials에 등록하도록 하자. `{젠킨스 주소}/manage/credentials/`에 접속하거나 **Jenkins 관리 → Credentials**에 이동하여 Add credentials를 클릭하도록 하자.

![젠킨스 credentials](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/06.png)

그리고 Secret text에 토큰 값을 붙여넣도록 하자.

![credentials 등록](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/07.png)

## 서버 접속 키 등록

젠킨스에서 빌드가 완료되면 이를 도커 허브로 최신화를 하든 파일 또는 디렉터리를 배포 서버에 전송하든 서버에 접근해야 한다.

이를 위해서 SSH Agent 플러그인을 이용하여 접속에 필요한 키를 등록해야 한다. 마찬가지로 Credentials로 등록하자.

![접속 키 등록](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/08.png)

접속 키를 등록하려면 등록하려는 키를 텍스트 형식으로 열면 위와 같은 문자열이 나온다. 전체 복사 후 붙여넣기 하도록 하자.

# Job 생성 및 구축

이제 job을 생성하여 CI/CD를 수행하도록 하자.

![job 생성 화면](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/09.png)

## Generic Webhook Trigger 설정

설정은 간단하게 사진으로 보도록 하자.

![generic webhook trigger 1](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/10.png)

![generic webhook trigger 2](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/11.png)

## Github Webhook 추가

깃허브 레포지토리에서 Settings를 클릭하고 그 옆에 Webhooks를 클릭하도록 하자.

![github webhook 1](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/12.png)

이미 젠킨스가 등록된 상황이다. 이를 자세히 보면 다음과 같다.

![github webhook 2](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/13.png)

마지막 부분이 짤렸지만 저 `token` 부분이 generic webhook trigger에서 정의한 토큰 값이다. push 이벤트는 pr까지 커버가 된다.

다른 브랜치에서 main 브랜치로 pr이 이루어지면 main 브랜치에도 push 이벤트가 발생하는 방식이다. 따라서 push 이벤트에만 동작하도록 구성하였다.

### webhook으로 젠킨스에 보내는 값 확인

제대로 연동이 된다면 젠킨스에 어떤 값을 보낼까? 이벤트를 발생시켜 페이로드를 살펴보았다.

```json
{
  "ref": "refs/heads/main",
  "before": "91742159125bd5dd85bbbaf0e485d63332b8961e",
  "after": "55bb64a1e27dc62d6be698c8de014f8f92f2d8e9",
  "repository": {...}
...
}
```

이런 식으로 나오게 된다. 이때 우리가 필요한 것은 `ref` 부분이다. 앞서 generic webhook trigger 설정에서 보았겠지만 젠킨스는 깃허브가 보내준 이 페이로드 중에서 `ref`의 값을 추출하여 빌드 과정에서 환경변수로 사용할 수 있게 해준다.

## 파이프라인 구축

이제 webhook이 전달되면 처리할 파이프라인을 구축하도록 하자. 파이프라인은 두 가지로 관리할 수 있는데, 하나는 젠킨스 파일을 특정 레포지토리에 보관하여 관리하는 방법이 있고, 다른 하나는 젠킨스의 job에 직접 정의하는 방법이 있다.

{% include code-header.html %}

```groovy
pipeline {
    agent any

    environment {
        JAR_NAME = 'app.jar'               // 빌드된 JAR 파일 이름
        BUILD_DIR = 'build/libs'           // JAR 파일이 위치한 디렉토리
        BACKEND_DIR = 'backend'
        AWS_REGION = 'ap-northeast-2'  // AWS 리전
        TARGET_SERVER_PATH = 'ubuntu@ec2-3-39-228-2.ap-northeast-2.compute.amazonaws.com'
        PUSHED_BRANCH = "${env.PUSHED_BRANCH}"
    }

    options {
        disableConcurrentBuilds()
    }

    stages {
        stage('Clean Workspace') {
            steps {
                cleanWs() // workspace 정리
            }
        }

        stage('Conditional Execution') {
            steps {
                script {
                    echo "BUILD START"
                    if (env.PUSHED_BRANCH == 'refs/heads/main') {
                        echo 'BACK-END BUILD'
                        checkoutCode('main', 'github-token', 'https://github.com/sehako/cicd-study.git', BACKEND_DIR)
                        buildSpringboot()
                    }

                    echo 'BUILD END'
                }
            }
        }
    }

    post {
        success {
            echo 'Deployment successful!'
        }
        failure {
            echo 'Deployment failed!'
        }
    }
}

def checkoutCode(String branchName, String credentialValue, String gitUrl, String buildDir) {
    stage('Checkout') {
        dir(buildDir) {
            git branch: branchName, credentialsId: credentialValue, url: gitUrl
        }
    }
}

def buildSpringboot() {
    dir(BACKEND_DIR) {
        stage('Build JAR') {
            sh "chmod +x gradlew"
            sh "./gradlew clean bootJar"
        }
    }

    stage('Deploy on Server') {
        dir(BACKEND_DIR) {
            dir(BUILD_DIR) {
                sshagent(credentials: ['target-server-key']) {
                    sh '''
                        scp -v -o StrictHostKeyChecking=no ./app.jar ${TARGET_SERVER_PATH}:/home/ubuntu
                        ssh -v -o StrictHostKeyChecking=no ${TARGET_SERVER_PATH} "bash /home/ubuntu/deploy_backend.sh"
                    '''
                }
            }
        }

    }
}
```

위 파이프라인은 레포지토리의 main 브렌치에 푸시가 되거나 pr이 들어와 머지가 되면 빌드 후 배포를 수행하도록 하는 것이다.

여기서 위 파이프라인을 테스트 하면 오류가 발생할 것이다. 현재 서버에 `deploy_backend.sh` 파일이 없기 때문이다. 이를 다음과 같이 정의하도록 하자.

{% include code-header.html %}

```bash
 #!/bin/bash

rm ./DeploySet/app.jar

mv ./app.jar ./DeploySet/

cd DeploySet

sudo docker-compose up -d --build
```

젠킨스에서 위 명령어들을 작성할 수 있지만 그렇게 하고보니 내 눈에는 파이프라인 코드가 좀 맘에 안들었다. 그래서 이런 방식을 사용했다.

이제 내 프로젝트를 변경해보도록 하겠다.

{% include code-header.html %}

```java
@RestController
public class Controller {
  @GetMapping("/api/v1/hello")
  public ResponseEntity<Return> hello() {
    return ResponseEntity.ok(new Return("Hello World"));
  }

  record Return(
    String message
  ) {
  }
}
```

초기에는 다음과 같은 코드가 있는데, 여기에 다음 요청 메소드를 추가하도록 해보자.

{% include code-header.html %}

```java
  @GetMapping("/api/v1/test")
  public ResponseEntity<Return> test() {
    return ResponseEntity.ok(new Return("test"));
  }
```

성공적으로 CI/CD가 된 것을 볼 수 있을 것이다.

# 번외 - 설정 파일 관리

.env 파일로 관리하는 경우에는 EC2에 .env 파일을 저장해두고 관리하는 것이 속편한 것 같다. 하지만 일반적으로 스프링 부트의 application.properties또는 application.yml 파일을 아예 깃 허브에 업로드 하지 않는 방법을 더 많이 사용하는 것 같아서 조금 생각해보았다.

이 경우에는 application.yml 파일을 젠킨스의 credentials에 시크릿 파일로 등록을 한 다음에 빌드 직전에 이 파일을 프로젝트의 resources 디렉터리에 넣는 방법을 사용할 수 있을 것이다.

테스트를 위해 간단한 텍스트 파일(text.txt)을 하나 만들어서 젠킨스에 시크릿 파일로 등록하도록 하겠다.

![시크릿 파일 업로드](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/14.png)

앞서 작성한 파이프라인에서 `buildSpringBoot()` 부분을 다음과 같이 수정하도록 하자.

{% include code-header.html %}

```groovy
def buildSpringboot() {
    dir(BACKEND_DIR) {
		    // 젠킨스에 저장해둔 설정 파일을 resources에 옮기는 부분
        stage('Move Secret File') {
            withCredentials([file(credentialsId: 'file-test', variable: 'secretFile')]) {
                sh 'cp $secretFile ./src/main/resources/text.txt'
            }
        }

        stage('Build JAR') {
            sh "chmod +x gradlew"
            sh "./gradlew clean bootJar"
        }
    }

    stage('Deploy on Server') {
        dir(BACKEND_DIR) {
            dir(BUILD_DIR) {
                sshagent(credentials: ['target-server-key']) {
                    sh '''
                        scp -v -o StrictHostKeyChecking=no ./app.jar ${TARGET_SERVER_PATH}:/home/ubuntu
                        ssh -v -o StrictHostKeyChecking=no ${TARGET_SERVER_PATH} "bash /home/ubuntu/deploy_backend.sh"
                    '''
                }
            }
        }
    }
}
```

![시크릿 파일 복사 결과](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/15.png)

워크스페이스를 보면 잘 저장된 것을 볼 수 있다.

# 번외 - CloudFront + S3로 프론트엔드를 배포한 경우

만약 프론트엔드를 클라우드 프론트를 이용하여 배포한 경우에는 S3에 프론트앤드 빌드 파일을 바꾸기만 하면 된다. 이를 위해서 다음 플러그인이 필요하다.

- https://plugins.jenkins.io/aws-credentials
- https://plugins.jenkins.io/pipeline-aws

그 다음 AWS에서 IAM으로 S3에 대한 권한을 가지는 사용자를 하나 생성하여 이 사용자에 대한 엑세스 키를 발급받는다.

![IAM 발급](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/16.png)

그리고 발급받은 엑세스 키와 엑세스 비밀 키를 젠킨스 Credentials에 다음과 같이 등록하면 된다.

![IAM 키 설정](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/17.png)

그리고 npm을 사용해서 빌드를 해야하기 때문에 앞서 설치한 NodeJS 플러그인을 설정해주도록하자. Jenkins 관리 -> Tools로 들어가면 설정이 가능하다.

![npm 설정](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/18.png)

파이프라인 코드는 간단하게 작성하도록 하겠다.

파이프라인 코드는 간단하게 작성하도록 하겠다.

{% include code-header.html %}

```groovy
pipeline {
    agent any

    tools {
        nodejs "nodejs-23.10"  // Jenkins에 등록된 Node.js 설치 이름
    }

    environment {
        AWS_REGION = 'ap-northeast-2'      // AWS 리전
        FRONTEND_DIR = 'frontend'          // 실제 프론트엔드 디렉토리로 바꿔주세요
        S3_BUCKET = 'your-s3-bucket-name'  // 배포 대상 S3 버킷 이름
        DIST_PATH = 'your/dist/path'       // S3 내부 경로
        PUSHED_BRANCH = "${env.GIT_BRANCH}" // 실제 브랜치
    }

    stages {
        stage('Build') {
            steps {
                dir("${FRONTEND_DIR}") {
                    sh "npm install"
                    sh "npm run build"
                }
            }
        }

        stage('Deploy to S3') {
            when {
                expression { env.PUSHED_BRANCH == 'refs/heads/YOUR_FRONTEND_BRANCH' }
            }
            steps {
                script {
                    dir("${FRONTEND_DIR}") {
                        sh "pwd"      // 현재 디렉토리 출력
                        sh "ls -al"   // 파일 목록 확인

                        withAWS(credentials: 'aws-access-key', region: "${AWS_REGION}") {
                            sh 'aws s3 sync dist s3://${S3_BUCKET}/${DIST_PATH} --delete'

                            // 캐시 무효화 예시 (필요 시 주석 해제)
                            // sh 'aws cloudfront create-invalidation --distribution-id E2YNBJEMVHDZ8H --paths "/*"'
                        }
                    }
                }
            }
        }
    }
}
```

여기서 CloudFront 캐시 무효화 라는 주석은 변경 사항을 바로 반영하기 위해서 작성했었는데, 과금의 위험성이 있다고 해서 그냥 비활성화 시켰었다. 이 명령어를 위해서는 CloudFront 접근 권한도 IAM 사용자에게 줘야 한다.

# 번외 - 도커 허브를 이용한 CI/CD

도커 허브를 이용한 CI/CD도 필요할까 싶어서 간단하게 설명하도록 하겠다. 실제 프로젝트에서 나는 이 방법으로 CI/CD를 진행했었다. 다음 플러그인이 필요하다.

- [Docker Pipeline](https://plugins.jenkins.io/docker-workflow/)

그리고 Credentials에 도커 로그인을 위한 자신의 도커 계정의 아이디와 비밀번호를 등록하도록 하자.

![도커 아이디 비밀번호 캡쳐](/assets/images/infrastructure/25-04-03-set-cicd-with-jenkins/19.png)

{% include code-header.html %}

```groovy
def buildSpringboot() {
    dir(BACKEND_DIR) {
        stage('Build JAR') {
            sh "chmod +x gradlew"
            sh "./gradlew clean bootJar"
        }
    }

    stage('Build Docker Image') {
        sh "docker build --rm -t ${YOUR_DOCKER_ACCOUNT_NAME}/${YOUR_DOCKER_HUB_NAME}:latest ."
    }

    stage('Push to Docker Hub') {
        withDockerRegistry([credentialsId: 'docker-basic-auth']) {
            sh "docker push ${YOUR_DOCKER_ACCOUNT_NAME}/${YOUR_DOCKER_HUB_NAME}:latest"
        }
    }

    stage('Deploy on Server') {
        sshagent(credentials: ['target_server']) {
            sh '''
                ssh -v -o StrictHostKeyChecking=no {TARGET_SERVER_PATH} "bash {SERVER_PATH}/deploy_backend.sh"
            '''
        }
    }
}
```

사실 기억이 가물가물하다. 아무튼 젠킨스에서 빌드한 이미지를 자신이 구축한 도커 허브에 업로드하고. `deploy_backend.sh`를 실행시키면 된다. 해당 쉘 스크립트는 다음과 같다.

{% include code-header.html %}

```sh
#!/bin/bash

cd DeploySet

sudo docker-compose up -d --build
```

사실 이 부분은 약간 첨부 느낌으로 작성한 거라 불확실하다. 플러그인도 추가적으로 더 필요할 수 있으니 해서 안되면 다른 참고글을 찾아보도록 하자.

---

이것이 마지막 배포 시리즈가 될 것이다. 최대한 자세히 쓰느라 글이 두서 없는 것 같지만 차근차근 따라가다보면 어느정도 흐름을 파악하여 자신이 원하는 배포 및 CI/CD 파이프라인을 구축할 수 있을 것이다.

하다보니 초반에 자신있게 이 실습 파일로 처음부터 끝까지 다 된다고 해놓고는 마지막 시리즈에 수정한 부분이 있어서 조금 아쉽다. 특히 nginx 설정 문제인 줄 알고 nginx를 컨테이너에서 제외했는데 알고보니 EC2 자체에서 설치하지 않은 항목이 있어 생긴 문제였다...

다시 되돌릴까 생각했지만 이미 그때 EC2를 몇 번 내리고 다시 올리고 이것저것 한 뒤라 힘들어서 그냥 nginx 제외하고 보안 그룹도 바꿔 진행했다. 아마 보안 그룹을 HTTP/HTTPS로만 설정하고 nginx + spring boot + jenkins를 컨테이너로 실행해도 문제없이 위 과정을 따라할 수 있을 것이다. 참고로 그렇게 nginx를 적용하면 `{서버 주소}/jenkins`로 접속하면 된다.

한 가지 유념할 점은 실제 프로젝트 배포 도중에 젠킨스가 자동 업데이트 되면서 도커 볼륨에 저장해둔 설정 몇몇 부분을 제대로 읽지 못하여 결과적으로 젠킨스가 실행조차 되지 않는 문제가 발생하였었다.

따라서 generic webhook trigger 설정이나 파이프라인 코드는 주기적으로 백업해두자. 개인적으로 파이프라인 코드는 굳이 젠킨스에 정의하지 말고 저장할 레포지토리에 젠킨스 파일로 관리하는 방법인 Pipeline script from SCM를 고려하는 것도 좋은 방법일 것 같다.

또한 빌드된 파일을 서버에 올리는 방법도 있지만 빌드하여 도커 허브로 올린 다음에 배포 서버에서 도커 허브를 통해 업데이트 하는 방법도 있다. 이 부분도 알아두면 좋을 것이다. 쉘 스크립트만 변경하면 되므로 큰 틀은 벗어나지 않는다.

그리고 위 파이프라인은 모노레포 구조를 기반으로 작성된 코드를 조금 수정한 것이다. 만약 프론트엔드와 백엔드 레포지토리가 나누어진 상황이라면 굳이 함수로 안만들어도 될 것 같다. 이 포스팅 시리즈가 배포를 처음 해보는 사람들에게 도움이 되었으면 좋겠다.

# 참고자료

[**Jenkins 로고**](https://www.pngwing.com/en/free-png-tibbn)

[**What is CI/CD?**](https://www.redhat.com/en/topics/devops/what-is-ci-cd)

[**[INFRA] AWS EC2 프리티어 인스턴스 생성하기**](https://olrlobt.tistory.com/83)

[**[AWS] EC2 스왑 메모리 설정하기 (EC2 메모리 늘리기)**](https://diary-developer.tistory.com/32)

[**Certbot 설치, 사용법 (Ubuntu)**](https://velog.io/@juhyeon1114/certbot-%EC%84%A4%EC%B9%98-%EC%82%AC%EC%9A%A9%EB%B2%95-ubuntu)

[**scp - 파일 또는 폴더를 업로드/다운로드 합니다.**](https://dejavuqa.tistory.com/358)

[**[Jenkins] npm 빌드 설정**](https://sg-choi.tistory.com/213)
