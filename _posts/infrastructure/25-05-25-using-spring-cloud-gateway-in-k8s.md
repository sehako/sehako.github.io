---
title: 쿠버네티스 환경에서 Spring Cloud Gateway 사용하기

categories:
  - Infrastructure

toc: true
toc_sticky: true
published: true

date: 2025-05-25
last_modified_at: 2025-05-25
---

프로젝트에서 쿠버네티스를 다뤄본 팀원이 있었고, 덕분에 쿠버네티스 환경에서 게이트웨이를 마이크로 서비스를 라우팅 하는 방법을 얕게나마 경험해볼 수 있었다.

# 쿠버네티스란?

쿠버네티스는 컨테이너 오케스트레이션 도구이다. 여기서 컨테이너 오케스트레이션이란 대규모 어플리케이션을 배포할 수 있도록 컨테이너의 네트워킹 및 관리를 자동화하는 프로세스를 말한다.

## 컨테이너 오케스트레이션 등장 배경

2008년 리눅스 커널에 컨테이너 기능이 포함되고, Docker 컨테이너 플랫폼이 등장하면서 컨테이너화된 마이크로서비스 또는 서버리스 기능이 최신 클라우드 네이티브 어플리케이션의 실질적인 컴퓨팅 단위로 자리잡았다고 한다.

그러면서 컨테이너화된 어플리케이션의 수가 빠르게 증가하고 있고, 이에 따른 CI/CD 또는 DevOps 파이프라인의 일부로 이러한 어플리케이션을 대규모로 관리하는 것은 자동화 없이는 불가능하게 되었다.

## 쿠버네티스의 등장

따라서 이러한 수 많은 컨테이너들을 관리하는 도구는 필수적이게 되었다. 다양한 컨테이너 오케스트레이션 플랫폼이 존재하지만, 가장 널리 사용되는 것은 바로 쿠버네티스이다. IBM에서 소개하는 쿠버네티스의 장점은 다음과 같다.

- **컨테이너 배포:** 지정된 수의 컨테이너를 지정된 호스트에 배포하고 원하는 상태로 계속 실행한다.
- **롤아웃(상태 변경):** 롤아웃을 시작, 일시 중지, 재개 또는 롤백할 수 있다.
- **서비스 검색:** DNS 이름 또는 IP 주소를 사용하여 컨테이너를 인터넷이나 다른 컨테이너에 노출이 가능하다.
- **스토리지 프로비저닝:** 개발자는 필요에 따라 컨테이너에 대한 영구 로컬 또는 클라우드 스토리지를 탑재하도록 설정할 수 있다.
- **로드 밸런싱 및 확장성:** 컨테이너에 대한 트래픽이 급증하면 로드 밸런싱 및 확장을 사용하여 네트워크 전체에 트래픽을 분산함으로써 안정성과 성능을 보장할 수 있다. (또한 개발자가 로드 밸런서를 설정하는 작업도 줄일 수 있습니다.)
- **고가용성을 위한 자가 복구:** 컨테이너에 장애가 발생하면 컨테이너를 자동으로 재시작하거나 교체할 수 있다. 또한 상태 확인 요구 사항을 충족하지 않는 컨테이너를 삭제할 수도 있다.
- **여러 클라우드 공급업체의 지원 및 이식성:** 주요 클라우드 제공업체에서 폭넓은 지원을 받고 있기 때문에 이는 하이브리드 클라우드 또는 하이브리드 멀티클라우드 환경에 애플리케이션을 배포하는 조직에 유리하다.
- **오픈 소스 도구의 에코시스템 성장:** 지속적으로 확장되는 사용성 및 네트워킹 도구를 보유하고 있다. 여기에는 컨테이너를 서버리스 워크로드로 실행할 수 있는 Knative와 오픈 소스 서비스 메시인 ISTIO가 포함된다.

---

굉장히 많은 설명들을 작성하였지만, 내가 생각하는 쿠버네티스는 단순하게 다음과 같다.

_여러 대의 물리적 서버를 하나의 서버처럼 통합·관리하고, 그 리소스를 활용해 다양한 컨테이너를 배포하고 운영할 수 있게 해주는 플랫폼_

참고로 쿠버네티스가 모든 상황에 정답은 아니다. 포스팅을 위해 이것저것 찾아보던 중, 흥미로운 [해외 블로그 번역글](https://brunch.co.kr/@delight412/750)을 보게 되었다. 내용을 요약하자면, 작성자는 팀의 상황과 목적에 맞춰 기술을 선택해야 한다는 것을 강조한다.

# 쿠버네티스 환경에서의 MSA

아무튼 다시 돌아와서, 기존에 Spring Cloud에서 제공하는 유레카(Eureka) 서버를 통해 MSA를 구축하였었다. 하지만 이러한 방식은 전체 서버 아키텍처가 스프링 생태계에 의존적이라는 단점을 가진다.

예를 들어, 새로운 마이크로서비스를 추가로 개발해야 하는 상황에서, 스프링 이외의 Node.js, Go, Python 등의 기술 스택이 더 적합하더라도, 해당 서비스가 유레카에 등록될 수단이 마땅치 않을 수 있다.

이는 마이크로서비스 아키텍처의 핵심 가치 중 하나인 이기종 기술 스택 간의 독립성과 유연성을 저해하는 결과를 초래할 수 있다. 이와 같은 상황에서 쿠버네티스를 활용하면, 게이트웨이는 쿠버네티스 API에 접근할 수 있는 최소한의 권한만으로도 서비스를 탐색하고 요청을 전달할 수 있다.

또한, 게이트웨이에서 별도의 로깅이나 인증/인가 처리가 필요 없는 경우, 쿠버네티스의 내부 서비스 디스커버리와 라우팅 기능만으로도 직접 통신이 가능 하다는 장점이 있다.

## 쿠버네티스 + Spring Cloud Gateway

이제 쿠버네티스 환경에서 동작하기 위해 스프링 클라우드 게이트웨이를 변경하도록 하자. 기존에는 해당 게이트웨이가 유레카 서버에 접근하여 서비스를 판별했다면, 이제는 쿠버네티스에 접근하여 서비스를 판별하게 된다.

### 의존성 설정

{% include code-header.html %}

```groovy
implementation 'org.springframework.cloud:spring-cloud-kubernetes-client-discovery:3.2.1'
```

쿠버네티스에서 디스커버리 역할을 해주기 때문에 유레카 관련 의존성을 제거하고 위와 같은 의존성을 설정하면 된다.

### 쿠버네티스 디스커버리 로그

로그 레벨을 `debug`로 한 다음에 어플리케이션을 실행해보면 게이트웨이에 존재하는 `KubernetesDiscoveryClientUtils`이 쿠버네티스의 모든 서비스를 판별한 것을 볼 수 있다.

```yaml
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'argocd-server' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'argocd-redis' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'calico-kube-controllers-metrics' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'nginx-test' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'cert-manager' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'spring-cloud-kubernetes-discoveryserver' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'ingress-nginx-controller' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'cert-manager-cainjector' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'jenkins' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'argocd-metrics' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'service-crew-helm-chart-crew' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'service-crew-helm-chart-crew' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'calico-typha' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'ingress-nginx-controller-admission' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'argocd-applicationset-controller' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'argocd-dex-server' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'service-video-helm-chart-video' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'calico-api' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'argocd-server-metrics' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'service-member-helm-chart-member' will match
s.c.k.c.d.KubernetesDiscoveryClientUtils : service labels from properties are empty, service with name : 'kubernetes' will match

```

이 중에서 `service-member-helm-chart-member`라는 서비스를 다음과 같이 라우팅 하였다.

### 게이트웨이 라우팅 설정

{% include code-header.html %}

```yaml
server:
  port: 8080
spring:
  application:
    name: gateway-service
  cloud:
    kubernetes:
      discovery:
        enabled: true
        all-namespaces: true
    bus:
      destination: spring-config-bus
    gateway:
      discovery:
        locator:
          enabled: true
          lower-case-service-id: true
      routes:
        - id: member-service
          # http://<서비스이름>.<네임스페이스>.svc.cluster.local:<포트>
          uri: http://service-member-helm-chart-member.service-member.svc.cluster.local:8080
          predicates:
            - Path=/api/member/**
          filters:
            - RewritePath=/api/member/(?<segment>.*), /$\{segment}
            - AuthorizationFilter
```

이러면 유레카 서버 없이 서비스가 쿠버네티스 환경에만 있으면 게이트웨이를 통해 라우팅을 시킬 수 있다.

### 아쉬운 부분

하지만 이는 반쪽 짜리 MSA인데, 그 이유는 서비스 이름으로 접근하는 것이 아닌 직접적으로 서비스, 네임 스페이스, 그리고 포트까지 명시하기 때문이다.

만약 member-service를 유레카 서버에 등록하고, 게이트웨이를 통해서 해당 서비스에 접근 한다면 다음과 같이 처리해주면 됐을 것이다.

{% include code-header.html %}

```yaml
routes:
  - id: member-service
    uri: lb://member-service
    predicates:
      - Path=/api/member/**
    filters:
      - RewritePath=/api/member/(?<segment>.*), /$\{segment}
      - AuthorizationFilter
```

쿠버네티스 역시 이러한 방식이 가능하다고는 하였지만, 우리 팀에서 시도했을 때 계속해서 오류가 발생하였고, 생각보다 쿠버네티스 환경에서 배포가 된 것이 많이 늦어졌기 때문에 게이트웨이 라우팅 설정을 여기서 더 발전시킬 수 없었던 것이 아쉬웠다.

---

마지막 프로젝트도 끝이 났다.

이번 프로젝트에서 쿠버네티스 환경에서 게이트웨이 라우팅 처리를 경험해 볼 수 있어서 좋았지만, 예상보다 초기 설정과 환경 구성에 시간이 많이 소요되었고, 그로 인해 일정이 점차 밀리면서 게이트웨이 설정에 충분한 시간을 들이지 못한 채 라우팅만 겨우 마무리하게 되었다. 그 결과, 완성도 높은 MSA 구조를 구성하지는 못했다.

지금 돌아보면, 이번 프로젝트에서는 MSA 자체를 깊게 파고들기보다는 당면한 기술적 문제를 해결하는 데 집중하는 편이 더 나았을지도 모르겠다. 실제로 MSA 구성과 관련된 설정 작업과 각종 잡무를 처리하는 사이, 내가 담당하려 했던 소켓 프로그래밍은 계속 뒤로 밀리게 되었다.

무엇보다도 프로젝트 초기에 MVP를 개발할 때, 내가 주요 기술 스택을 잘못 선택한 점이 치명적이었다. 뒤늦게 그 사실을 인지했을 때에는 이미 마감이 다가오고 있었고, 구조를 되돌릴 여유는 없었다. 결과적으로 프로젝트 전반이 다소 어설픈 형태로 마무리되고 말았다.

그래서 이번 프로젝트는 유난히 아쉬움이 많이 남는다. 물론 이런 경험을 동기부여도 얻게 되었지만, 완성도 부족의 원인이 내 결정에 있었던 만큼 착잡한 감정도 짙게 남게 되었다.

# 참고자료

[**컨테이너 오케스트레이션이란 무엇인가요?(AWS)**](https://aws.amazon.com/ko/what-is/container-orchestration/)

[**컨테이너 오케스트레이션이란 무엇인가요?(IBM)**](https://www.ibm.com/kr-ko/topics/container-orchestration)

[**쿠버네티스와 결별했더니 달라진 것들**](https://brunch.co.kr/@delight412/750)
