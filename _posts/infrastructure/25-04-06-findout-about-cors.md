---
title: CORS에 대해 알아보자

categories:
  - Spring
  - Infrastructure

toc: true
toc_sticky: true
published: true
 
date: 2025-04-06
last_modified_at: 2025-04-06
---

프로젝트를 진행하다보면 이 녀석을 한 번쯤 만나봤을 것이다. 오늘은 이 녀석이 어째서 등장하였고, 어떤 방식으로 동작하며, 어떻게 처리하는 지 알아보도록 하자.

# CORS란 무엇일까?

CORS(Cross-Origin Resource Sharing)는 서로 다른 출처(origin) 간의 리소스 공유를 가능하게 해주는 정책이다. 기본적으로 웹 브라우저는 '동일 오리진 정책(Same-Origin Policy)'을 따르는데, 이는 웹 애플리케이션이 자신이 속한 도메인(오리진) 외의 리소스에 접근하지 못하도록 제한하는 보안 정책이다. 쉽게 말하면, 자신이 속한 도메인으로부터만 데이터를 요청하고 받을 수 있다는 뜻이다.

하지만 REST API는 다양한 오리진(도메인)에서 접근되는 경우가 많다. 예를 들어, 카카오 맵 API나 OAuth 같은 서비스들은 여러 웹사이트나 앱에서 요청을 받게 되므로, 동일 오리진 정책만으로는 이를 처리할 수 없다.

## CORS 확인 과정

따라서 프론트엔드에서 도메인이 다른 서버로 요청을 보낼 경우, 브라우저는 해당 요청을 보내기 전에 해당 오리진이 서버로부터 허용되었는지를 먼저 검사한다.

이처럼 실제 요청을 보내기 전에 사전 검사를 위해 브라우저는 HTTP OPTIONS 메서드를 사용한 요청을 보내는데, 이를 preflight(사전 요청)요청 이라고 한다.

이 요청의 헤더에는 다양한 값이 있는데, 대표적으로는 실제 요청에 사용되는 메소드와 요청을 보낸 오리진(도메인)에 대한 정보이다. 

서버는 이 preflight 요청을 받으면 헤더에 담긴 정보를 바탕으로, 요청된 메서드와 오리진이 허용된 값인지를 확인한다. 이 중 하나라도 허용되지 않은 경우, 브라우저는 리소스 접근을 차단하며, 결과적으로 CORS 정책 오류가 발생하게 된다.

# CORS를 허용하는 방법

전통적인 웹 MVC 개발에서는 이를 크게 고려하지 않아도 된다. 그 이유는 뷰를 보여주는 오리진과 데이터를 요청하는 오리진이 일반적으로 동일하기 때문이다. 하지만 뷰가 분리되어 서버에서는 요청에 대한 데이터만 반환하면 되는 REST API 구조에서는 이러한 문제가 꼭 발생하게 되어 있다.

따라서 우리는 서버에서 CORS를 허용하는 방법을 알아두어야 한다. 하지만 그 전에 앞서 말한 오리진이라는 것이 무엇인지 실제 요청을 통해 확인해보도록 하자. 이를 위해 CORS가 발생하는 상황에 대해서 임의 요청 페이지를 만들어서 OPTIONS 요청을 살펴보도록 하자.

![OPTIONS 요청 예시](/assets/images/findout-about-cors_01.png)

잘 보면 `Access-Control-Request`와 `Origin`이 보일 것이다. 결국 서버에서는 이 두 값을 가지고 확인하여 결과적으로 허용 여부를 결정하게 되는 것이다. 참고로 `Origin`은 브라우저가 현재 사용하고 있는 주소값이다. 따라서 `http://localhost:5500`과 `http://127.0.0.1:5500`은 개발자 입장에서는 같은 경로라고 생각하지만 `Origin` 헤더에는 저 문자열이 그대로 담기기 때문에 둘 모두 허용 처리를 해줘야 한다. 

이런 preflight 요청을 허용하기 위한 방법으로는 두 가지의 방법이 존재하는데, 하나는 웹 서버에서 허용하는 방법이고, 다른 하나는 스프링 부트 같은 WAS에서 허용하는 방법이다. 

한 가지 주의해야 할 것은 두 방법 중 하나의 방법만 사용해야 한다는 것이다. 만약 웹 서버에서 CORS 처리를 해줬다면 WAS에서는 해줄 필요가 없고, 그 반대 역시 마찬가지이다.

## 웹 서버에서 허용(Nginx)

나는 nginx를 사용하여 어플리케이션을 리버스 프록시를 하는 방법만 알기 때문에 이를 기준으로 설명하도록 하겠다. nginx에서는 다음과 같이 작성하면 된다.

{% include code-header.html %}
```
 http {
		# ...

    map $http_origin $allowed_origin {
        default "";
        "http://localhost:5173" $http_origin;
        "http://127.0.0.1:5500" $http_origin;
    }

    server {
        listen 80 default_server;
        listen [::]:80 default_server;
        server_name {SERVER_DOMAIN_NAME};
        # 모든 HTTP 요청을 HTTPS로 리디렉트
        return 301 https://$host$request_uri;
    }
    
    server {
        listen 443 ssl;
        listen [::]:443 ssl;
        server_name www.drawaing.site;

        location /api/ {
            if ($request_method = 'OPTIONS') {
                add_header 'Access-Control-Allow-Origin' "$allowed_origin" always;
                add_header 'Access-Control-Allow-Credentials' 'true' always;
                add_header 'Access-Control-Allow-Methods' 'GET, POST, PUT, DELETE, OPTIONS' always;
                add_header 'Access-Control-Allow-Headers' 'Origin, Content-Type, Accept, Authorization' always;
                add_header 'Access-Control-Max-Age' 1728000;
                add_header 'Content-Type' 'text/plain charset=UTF-8';
                add_header 'Content-Length' 0;
                return 204;
            }

            rewrite ^/service/game/(.*) /$1 break;
            proxy_pass http://{SPRING_BOOT:PORT}/;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
            proxy_redirect off;

            # WebSocket 지원 (Spring Boot에서 필요할 경우 추가)
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection "Upgrade";
		        proxy_read_timeout 3600;
            proxy_send_timeout 3600;

            add_header 'Access-Control-Allow-Origin' "$allowed_origin" always;
            add_header 'Access-Control-Allow-Credentials' 'true' always;
            add_header 'Access-Control-Allow-Methods' 'GET, POST, PUT, DELETE, OPTIONS' always;
            add_header 'Access-Control-Allow-Headers' 'Origin, Content-Type, Accept, Authorization' always;
        }
    }
}

```

여기서 `map` 부분에 명시된 값들이 바로 허용할 오리진의 목록이다. 하나는 리액트를 실행시키면 나오는 주소 값이고, 다른 하나는 내가 테스트를 위해 띄웠던 Live Server의 주소 값이다. 

이때 `Access-Control-Allow-Credentials`라는 것이 보일텐데 이 헤더는 클라이언트와 서버 간 자격 증명을 포함한다는 옵션이다. 이 옵션은 클라이언트가 쿠키, HTTP 인증 헤더(Authorization), TLS 인증서 등 이용한 인증 정보 전송을 허용할 것인가에 대한 옵션이라고 보면 된다.

이 옵션을 사용하려면 `Access-Control-Allow-Origin` 옵션이 와일드 카드, 즉 `*`이 되면 안된다. 이 둘은 CORS 표준 스펙을 위반하는 처리가 되기 때문이다. 아마 자격 증명을 누구에게나 허용하는 것은 보안상 위험하다는 판단인 것 같다. 따라서 초기에 프론트엔드 개발자와 상의하여 CORS를 제대로 명시하고 시작하도록 하자.

## WAS에서 허용

이번에는 스프링 부트 같은 WAS에서 CORS 처리를 하는 방법이다. 모놀로식 어플리케이션과 마이크로 서비스 아키텍처 어플리케이션에서 CORS 처리를 하는 방법은 약간 다른데, 코드를 통해 알아보도록 하자. 

### 모놀로식 어플리케이션

{% include code-header.html %}
```java
@Configuration
public class CorsConfig implements WebMvcConfigurer {
    @Override
    public void addCorsMappings(CorsRegistry registry) {
	    // 전체 주소에 대한 CORS 정책 등록 예시
      registry.addMapping("/**")
              .allowedOrigins(
                      "http://localhost:5173",
                      "http://127.0.0.1:5500")
              .allowCredentials(true)
              .allowedHeaders("*")
              .allowedMethods("GET", "POST", "PUT", "DELETE", "OPTIONS");
    }
}

```

모놀로식 어플리케이션은 전통적으로 내부적으로 톰캣 서블릿을 사용하기 때문에 `WebMvcConfigurer`를 구현하여 `assCorsMappings()`를 재정의한 클래스를 설정 정보로 등록하면 된다. 

### 마이크로 서비스 아키텍처

마이크로 서비스 아키텍처에서 사용하는 게이트웨이는 내부적으로 이벤트 루프 기반의 Netty라는 녀석으로 구현되어 있고, 이러한 게이트웨이를 통해서 여러 마이크로 서비스에 접근하도록 만드는 구조이다. 

물론 톰캣 서블릿 기반의 게이트웨이도 있긴 하지만, 게이트웨이는 수 많은 요청을 중계해야 하기 때문에 대체로 Netty 기반의 게이트웨이를 채택한다고 한다. 아무튼 클라이언트는 이러한 게이트웨이에 최초로 요청을 보내기 때문에 게이트웨이에서만 CORS 설정을 해주면 된다. 코드를 통해 살펴보자.

{% include code-header.html %}
```java
import java.util.List;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.cors.reactive.CorsWebFilter;
import org.springframework.web.cors.reactive.UrlBasedCorsConfigurationSource;

@Configuration
public class CorsConfig {
    @Bean
    public CorsWebFilter corsWebFilter() {
        CorsConfiguration config = new CorsConfiguration();
        config.setAllowCredentials(true);
        config.setAllowedOrigins(List.of(
                "http://localhost:5500",
                "http://localhost:5173",
                "https://www.drawaing.site"
        ));
        config.setAllowedHeaders(List.of("*"));
        config.setAllowedMethods(List.of("GET", "POST", "PUT", "DELETE", "OPTIONS"));

        UrlBasedCorsConfigurationSource source = new UrlBasedCorsConfigurationSource();
        source.registerCorsConfiguration("/**", config);
        return new CorsWebFilter(source);
    }
}

```

일부러 `import`도 보여주었다. 자세히 보면 우선적으로 보이는 차이점은 `WebMvcConfigurer`를 구현하는 구조가 아니라는 것이다. 또한 `web.cors.reactive` 패키지에 있는 두 클래스를 이용하여 CORS 설정을 한다는 것을 볼 수 있다.

앞서 게이트웨이는 Netty 기반으로 동작한다고 하였다. 이는 곧 게이트웨이가 Spring WebFlux 기반으로 동작한다고도 말할 수 있는 것이다. 따라서 `WebMvcConfigurer`를 구현하는 것이 아닌  `web.cors.reactive` 패키지에 있는 클래스를 이용하여 리액티브 필터를 정의하는 것이다. 이렇게 설정하면 게이트웨이에서 이미 CORS가 허용되기 때문에 각 마이크로 서비스에서는 CORS를 신경쓰지 않아도 된다.

### 참고 - STOMP 웹 소켓 CORS 설정

참고로 STOMP를 사용한 웹 소켓 CORS 설정도 있다.

{% include code-header.html %}
```java
@Configuration
@EnableWebSocketMessageBroker
@RequiredArgsConstructor
public class WebSocketConfig implements WebSocketMessageBrokerConfigurer {
    @Override
    public void registerStompEndpoints(StompEndpointRegistry stompEndpointRegistry) {
        stompEndpointRegistry.addEndpoint("/ws")
                .setAllowedOrigins("*");
    }
}

```

이는 HTTP CORS 설정과는 별도로 동작하므로 만약에 소켓 서버가 HTTP 요청도 겸한다면 HTTP CORS 설정도 추가적으로 해줘야 한다. ChatGPT에 따르면 이러한 설정은 엄밀히 말하면 CORS 설정 보다는 접근 제어에 가까운 개념이라고 한다. 

참고로 저 `setAllowedOrigins()`를 자세하게 명시했었는데, 해당되는 오리진에서 웹 소켓 연결을 하고 비즈니스 로직을 진행하다보니 제대로 연결이 안되는 문제가 발생하였다. 그래서 그냥 와일드 카드로 선언하도록 바꾸니까 또 잘 되었다. 그래서 그냥 STOMP 웹 소켓을 사용할 때는 인증/인가를 철저하게 하는 게 더 나은 것 같다는 생각이 든다.

### 어디에서 CORS 설정을 해줘야 할까?

CORS 설정은 요청이 처음 도달하는 곳에만 설정해주면 된다. 따라서 일반적으로 인프라 담당자나 게이트웨이 담당자가 처리하는 것이 이상적이다. 

하지만 막상 그렇게 처리해보니 Nginx 설정 정보가 너무 길고 복잡해져 가독성이 떨어졌다. 따라서 나의 경우에는 어플리케이션 레벨에서 CORS를 처리하는 것이 더 좋지 않을까 생각한다.

---

1주일 1 블로그를 목표로 하였는데 요새 취업 준비나 프로젝트로 바빠서 블로그 소재만 모으고 있다. 이번 프로젝트에서는 마이크로 서비스 아키텍처를 도입해보면서 발생하는 다양한 문제를 해결해보고 인프라 부분도 모니터링을 직접 적용해봤다. 

최근에는 쿠버네티스도 학습하고 있기 때문에 나에게 있어 남는 게 많은 프로젝트였던 것 같지만 앞서 도입한 새로운 아키텍처에서 오는 다양한 문제와 함께 각자의 취업 준비로 프로젝트 완성도 부분에서는 아쉬웠기 때문에 팀원들에게 미안한 마음도 있다… 

마지막이 되는 다음 프로젝트 때는 테스트 코드와 소켓 처리를 해보고 싶다. 이 부분을 잘 학습하면 아마 실무에서도 크게 민폐를 끼치지 않는 그런 사람이 되지 않을까 생각한다.

# 참고 자료

[**The Ultimate CORS Crash Course**](https://konghq.com/blog/learning-center/what-is-cors-cross-origin-resource-sharing)

[**교차 출처 리소스 공유 (CORS)**](https://developer.mozilla.org/ko/docs/Web/HTTP/Guides/CORS)