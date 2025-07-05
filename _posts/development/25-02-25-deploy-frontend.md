---
title: S3 + CloudFront + Route 53

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true
 
date: 2025-02-25
last_modified_at: 2025-02-25
---

프론트앤드 개발자가 리액트나 Vue 같은 프레임워크로 개발을 하고 빌드를 하면 프로젝트 내에 Vue 프로젝트 기준으로 dist라는 디렉터리가 하나 생기게 된다. 이 디렉터리를 들어가면 index.html 하나가 있는데, 이 파일이 바로 프론트앤드 개발자가 열심히 개발한 결과물이다.

그리고 이 결과물이 바로 사용자에게 보여지는 부분이다. 따라서 이 역시 앞선 백엔드와 같이 배포를 해야 한다. 이때 프론트앤드는 두 가지의 배포 방식을 선택할 수 있다.

1. 웹 서버(Nginx)가 설치된 서버에 저장 후 배포하는 방법
2. S3에 dist 디렉터리를 저장한 후 클라우드 프론트를 사용하여 배포하는 방법

# 웹 서버 정적 파일 호스팅

1번 방식은 웹 서버가 설치된 시스템에 dist 디렉터리를 저장하고, nginx.conf에 index.html 파일을 지정하면 된다.

```
location / {
    root /usr/share/nginx/html;
    index index.html;
    error_page 404 /index.html;
    error_page 403 /index.html;
}
```

나의 경우에는 도커를 이용하여 Nginx를 실행시켰기 때문에 도커 마운트로 dist 디렉터리를 nginx 컨테이너의 /usr/share/nginx/html에 저장하도록 하였다. 따라서 root 디렉터리가 위와 같은 경로가 된 것이다.

복습을 할 겸 nginx의 도커 컴포즈 설정을 다시 보도록 하자.

```yaml
nginx:
  container_name: nginx
  image: nginx:stable-alpine3.20-perl
  ports:
    - 80:80
    - 443:443
  volumes:
    - ./settings/nginx/nginx.conf:/etc/nginx/nginx.conf
    - ./dist:/usr/share/nginx/html
    - /ssl/path/fullchain.pem:/etc/nginx/ssl/fullchain.pem:ro
    - /ssl/path/privkey.pem:/etc/nginx/ssl/privkey.pem:ro
  environment:
    - TZ=Asia/Seoul
  depends_on:
    - spring
    - mysql
    - mongo
  # command: "/bin/sh -c 'while :; do sleep 6h & wait $${!}; nginx -s reload; done & nginx -g \"daemon off;\"'"
  restart: always
  networks:
    - compose-network
```

이때 만약에 백엔드 서버도 하나의 서버에서 띄운다면 다음과 같이 백엔드 서버를 리버스 프록시 하면 된다.

```yaml
location /api/ {
    proxy_pass http://spring:8080;
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto $scheme;
    proxy_redirect off;

    # WebSocket 지원 (Spring Boot에서 필요할 경우 추가)
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "Upgrade";
}

location / {
    root /usr/share/nginx/html;
    index index.html;
    error_page 404 /index.html;
    error_page 403 /index.html;
}
```

# S3와 CloudFront를 이용한 프론트앤드 배포

프론트 파일을 배포하는 방법 중에는 AWS에서 제공하는 S3와 CloudFront를 사용하는 방법이 있다. 본론으로 들어가기 전에 S3가 무엇인지 알아보도록 하자.

## S3 (Simple Storage Service)

> *Amazon Simple Storage Service(S3)는 업계 최고 수준의 확장성, 데이터 가용성, 보안 및 성능을 제공하는 객체 스토리지 서비스입니다.*  - AWS

S3는 AWS에서 제공하는 클라우드 저장소라고 보면 된다. 이를 사용하면 저장 장치의 용량도 알아서 조절해주고, 당연히 아마존에서 서비스 하는 것이기 때문에 내구성과 가용성이 좋고, 보안도 뛰어나다. 그리고 무엇보다 저렴하다는 장점이 있다.

![지불 정보](/assets/images/deploy-frontend_01.png)

2025년 2월에 프론트 배포를 위해 0.63달러를 사용했는데, 결제 및 비용 관리에서 사용했던 요금 누적 정보를 보면 실제로 저렴하다는 것을 볼 수 있다.

아무튼 S3는 데이터를 버킷이라는 단위에 객체로 저장한다. 쉽게 말해 버킷이 하드디스크, 객체가 파일 또는 디렉터리라고 보면 된다. 

### S3 버킷 생성

시작하기에 앞서 당연히 AWS 계정이 있어야 한다. 그리고 S3 서비스에 들어가 버킷 생성을 클릭하도록 하자.

나는 지역을 서울로 하고 버킷 이름은 `sehako-deploy`라고 하였다. 그 외의 옵션은 따로 설정하지 않아도 된다.

![생성된 버킷 정보](/assets/images/deploy-frontend_02.png)

### 빌드 산출물 업로드

이제 간단하게 프론트앤드를 배포하기 위해 리액트를 빌드하도록 하자. 나는 그냥 프로젝트만 생성하여 바로 빌드하였다. 

이때 처음 알았는데 리액트는 빌드 시 산출되는 디렉터리가 build 였다. 따라서 앞으로 프론트 배포 디렉터리 이름을 build라고 명칭하겠다.

![업로드 사진](/assets/images/deploy-frontend_03.png)

S3 버킷을 프론트 배포용으로만 사용한다면 build 디렉터리에 있는 모든 파일들을 업로드해도 된다. 

하지만 나는 이 버킷을 백엔드의 파일 저장소로도 사용한다 생각하고 dist 디렉터리 그 자체로 업로드 하기 위해 폴더 추가를 선택하였다. 

![디렉터리 선택](/assets/images/deploy-frontend_04.png)

이대로 업로드를 클릭하면 버킷에 build 디렉터리를 포함한 하위 파일들이 모두 업로드할 수 있다.

![업로드 결과](/assets/images/deploy-frontend_05.png)

CloudFront가 서비스 되기 전까지는 S3 자체에서 정적 파일 호스팅을 처리했다는 것 같다. 하지만 이 서비스가 등장하면서 이제 S3의 역할은 단순히 이런 프론트앤드의 빌드 산출물을 저장하는 역할에서 끝난다.

## CloudFront

CloudFront는 짧은 지연 시간과 빠른 전송 속도로 전세계에 신속하고 안전하게 배포한다는 목적으로 만들어진 서비스이다. 따라서 장점도 다음과 같다.

- 짧은 지연 시간

전 세계에 분포된 접속 지점을 통해 데이터를 전송해 지연을 줄인다.

- 보안 강화

트래픽 암호화, 엑세스 제어, VPC 오리진을 통해 보안을 개선하고 AWS Standard를 사용하여 DDos 공격으로부터 보호한다.

- 데이터베이스

통합된 요청, 사용자 지정 가능한 요금 옵션, AWS 오리진에서 데이터 송신 시 무료 요금으로 비용 절감

- 코드 사용자 지정

비용, 성능 및 보안의 균형을 맞추도로고 서버리스 컴퓨터 기능을 사용하여 AWS 콘텐츠 전송 네트워크(CDN) 엣지에서 실행하는 코드를 사용자 지정한다.

마지막 부분이 잘 이해가 안되서 chatGPT에게 물어보니 다음과 같은 답변을 주었다.

> *특정 요청에 대한 동적 처리 가능 (예: URL 리다이렉트, 헤더 수정)*
> 

따라서 사실 지역이 한정된 서비스의 경우에는 웹 서버에 그냥 배포해도 된다고 생각한다. 아무튼 시작해보도록 하자.

### 배포 생성하기

AWS에서 CloudFront 서비스 페이지에 접속하면 배포 생성 버튼이 보일 것이다. 이를 클릭하면 다음과 같은 페이지가 나타나게 된다.

![배포 생성](/assets/images/deploy-frontend_06.png)

여기서 오리진 선택을 클릭하면 자신의 S3 버킷을 선택할 수 있다. Origin path 부분은 자신의 S3 버킷에서 배포할 정적 파일이 존재하는 부분이다. 나는 build라는 디렉터리 전체를 업로드 했으므로 /build라고 작성하면 된다.

.그 밑에 옵션들은 간단하게 첨부 사진으로 보도록 하자.

![옵션 1](/assets/images/deploy-frontend_07.png)

![옵션 2](/assets/images/deploy-frontend_08.png)

![옵션 3](/assets/images/deploy-frontend_09.png)

### 설정

**일반 탭**

일반 탭의 설정을 편집할 수 있는데, 여기에 Default root  object에 index.html을 기입해준다.

![루트 오브젝트 선택](/assets/images/deploy-frontend_10.png)

**원본 탭**

이제 만들어진 배포에서 원본 탭으로 이동하면 하나의 원본이 보일 것이다 이를 체크하면 편집할 수 있다.

![원본 영역](/assets/images/deploy-frontend_11.png)

![원본 설정](/assets/images/deploy-frontend_12.png)

여기서 이미 S3 버킷이 지정되어 있지만 Create new OAC를 클릭하면 S3 버킷을 만들 수 있다. 그리고 정책 복사를 클릭하면 S3에 대한 정책이 자동으로 복사가 되는데, 이를 S3의 버킷 정책에 붙여넣기 하면된다.

![버킷 정책 설정](/assets/images/deploy-frontend_13.png)

이제 모든 설정이 완료되었다. 일반 탭의 세부 정보 부분에서 배포 도메인 이름을 복사해서 접속하면 나의 경우 리액트의 기본 페이지가 나올 것이다.

![배포 결과](/assets/images/deploy-frontend_14.png)

# Route 53

Route 53은 AWS에서 제공하는 DNS 서비스이다. 

## 가비아에서 도메인 구매하기

배포 도메인 이름을 보면 상당히 못생겼다. 이를 각자의 서비스에 맞게 AWS에서 제공하는 Route 53으로 한 번 더 감싸주도록 하자. 나는 가비아에서 sehako.store라는 도메인을 무려 550원 주고 구매하였다.

![도메인 구매 결과](/assets/images/deploy-frontend_15.png)

## Route 53 호스팅 영역 생성

이제 Route 53 서비스로 가도록 하자. Route 53으로 이동하면 호스팅 영역 생성이라는 버튼이 존재한다. 이를 클릭하면 다음과 같은 설정 부분이 나올 것이다.

![호스팅 영역 생성](/assets/images/deploy-frontend_16.png)

도메인 이름에 자신이 구매한 도메인의 주소를 입력하면 된다. 따라서 나의 경우에는 sehako.store가 될 것이다.

## 네임 서버 설정

그러면 레코드 부분에 유형이 NS이고, 값/트래픽 라우팅 대상이 4개가 즐비한 부분이 있을 것인데, 이 부분이 바로 네임 서버이다. 가비아로 돌아가서 네임 서버를 등록하도록 하자.

![네임 서버 설정](/assets/images/deploy-frontend_17.png)

![네임 서버 설정 결과](/assets/images/deploy-frontend_18.png)

등록을 할 때 끝에 `.`은 제외하고 등록해야 한다.

## ACM 인증서 발급

CloudFront와 도메인을 연결하기 위해서는 우선 ACM 인증서를 발급해야 한다고 한다. 따라서 이를 먼저 발급받도록 하자. 인증서는 무조건 버지니아 북부에서 발급받아야 한다는 것 외에는 설명할 것이 없는데, 요청 버튼을 누르고 퍼블릭 인증서 요청을 누르면 다음 화면이 나오게 된다.

![인증서 발급 부분](/assets/images/deploy-frontend_19.png)

여기서 그냥 내가 구매한 도메인을 입력하면 된다. 그리고 인증서가 발급될 떄 까지 기다려야 한다. 제대로 설정을 했다면 10분 정도 걸릴 것이다.

![인증서 발급 완료](/assets/images/deploy-frontend_20.png)

그 다음에 도메인 부분에서 Route 53에서 레코드 생성을 클릭하기만 하면 된다.

## CloudFront 대체 도메인 설정

대체 도메인은 내가 구매한 도메인 이름을 작성하고 발급받은 ACM 인증서를 선택하면 된다.

![대체 도메인 설정](/assets/images/deploy-frontend_21.png)

그럼 설정 부분이 다음과 같이 될 것이다. 

![대체 도메인 설정 완료](/assets/images/deploy-frontend_22.png)

## 도메인 연결

이제 다시 Route 53으로 돌아가서 레코드 유형을 A로 하여 별칭을 클릭하고 다음과 같이 설정하면 된다. 

![Route 53 도메인 연결](/assets/images/deploy-frontend_23.png)

이제 최종적으로 sehako.store에 접속하면 다음과 같은 화면이 나오게 된다.

![도메인 연결 결과](/assets/images/deploy-frontend_24.png)

---

생각보다 프론트 배포를 할 때 시행착오가 조금 있었다. 나는 스프링 부트 처럼 빌드를 하면 하나의 파일만 나오는 줄 알았는데, 프론트앤드 개발자가 디렉터리를 줘서 당황했던 기억도 있고, Route 53을 하는 과정에서 네임 서버를 설정하는 것을 잊어서 2시간이 지나도록 승인이 안났던 기억도 있다.

최대한 내가 어떻게 프론트앤드 빌드 산출물을 배포했는지 과정을 보여주고자 이렇게 글을 남긴다. 다음은 젠킨스를 이용한 CI/CD가 될 것이다.

# 참고 자료

[**Amazon S3**](https://aws.amazon.com/ko/s3/)

[**Amazon CloudFront**](https://aws.amazon.com/ko/cloudfront/)

[**Amazon Route53**](https://aws.amazon.com/ko/route53/)