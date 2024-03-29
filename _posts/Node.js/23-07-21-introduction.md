---
title:  "[Node.js] 01 - 백엔드 입문하기"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-21
---

[Node.js 백앤드 개발자되기](https://product.kyobobook.co.kr/detail/S000201457949)를 정리한 문서

# Node.js

서버 측 JS 런타임 환경으로, V8과 libuv(C++ 라이브러리, HTTP, 파일, 소켓 통신 IO 기능 등을 제공)으로 구성되어 있다.

## 특징

### 싱글 스레드

Node.js는 하나의 콜스택으로 작업을 처리한다.

```js
function func1() {
    console.log("1");
    func2();
    return;
}

function func2() {
    console.log("2");
    return;
}

func1();
```

이 경우 `func1()`이 적재 `console.log("1")`이 다음으로 적재, `console.log("1")`이 pop되는 형테로 작업을 처리한다.

### 이벤트 기반 아키텍쳐

싱글 스레드는 동시에 0.1초가 걸리는 작업에 대해서 100개의 요청이 도달하면 마지막 사람은 10초 뒤에 응답을 받을 수 있다. 이런 단점을 Node.js는 이벤트 기반 아키텍처로 해소하였다.

```js
console.log("1");
//I/O  처리가 필요한 코드이므로 콜스택에서 이벤트 루프로 넘김
//이벤트 루프는 OS또는 스레드 워커에 처리
//처리에 대한 반환값을 이벤트 루프가 받아 콜스택에 넘김
setTimeout( () => console.log(2), 1000);
console.log("3");
```

주석의 설명대로, Node.js에서는 I/O 처리가 필요한 작업은 데스크 큐를 거처 libuv가 담당하는 이벤트 루프로 보내고 이벤트 루프는 해당 작업을 OS 또는 스레드 워커에게 처리를 맡긴다. 그 후 처리가 완료된 값을 이벤트 루프가 받아 콜 스택에 추가한다.

해당 `setTimeout()`을 0으로 설정하여도 `2`가 맨 마지막에 출력되는 것은 똑같다. Node.js에서의 API 영역에서 대기하는 시간이 0초일 뿐, 데스크 큐에 추가하고 이벤트 루프를 통해 콜 스택에 추가하는 것은 똑같기 때문이다.

# 첫 서버 프로그램 제작

```js
//http 모듈 로드
const http = require("http"); 
let count = 0;
//서버 인스턴스 생성 인수로는 콜백 함수를 받음
const server = http.createServer((req, res) => { 
    log(count);
    //요청에 대한 상태 코드, HTTP 프로토콜에서 요청 처리 성공의 의미
    res.statusCode = 200;
    //요청 응답에 대한 부가 정보, header에 설정
    res.setHeader("Content-Type", "text/plain");
    //응답
    res.write("hello\n");
    //2초 후 "Node.js를 응답으로 주고 HTTP 커넥션(연결)을 끝내는 동작"
    setTimeout(() => {
        res.end("Node.js");
    }, 2000);
});

function log(count) {
    console.log((count += 1));
}

//포트 번호 지정, IP 생략 시 localhost로 접근
server.listen(8000);
```

## curl
일반적으로 **localhost:8000**을 통해 접근하면 두 출력이 한 번에 나오는 듯 보이지만, 명령 프롬프트에서

```
curl localhost:8000
```

다음 명령으로 확인해보면 실제로는 `hello` 출력 이후 `Node.js`가 출력된다는 것을 알 수 있다

## K6

개발한 서버가 어느정도의 성능을 내주는지 테스트 하기 위해서 사용하는 여러 테스트 도구 중 하나다. 

```js
import http from "k6/http";

export const options = {
    vus: 100,
    duration: "10s",
};

export default function() {
    http.get("http://localhost:8000");
}
```

8000번 포트에 존재하는 서버를 테스트 하기 위한 JS 파일이다, `vus`는 100명의 사용자를 가정한다는 뜻으로, 100명의 사용자가 10초 동안 동시에 계속해서 요청을 보낸다는 의미이다. 

```
scenarios: (100.00%) 1 scenario, 100 max VUs, 40s max duration (incl. graceful stop):
           * default: 100 looping VUs for 10s (gracefulStop: 30s)


     data_received..................: 93 kB 9.1 kB/s
     data_sent......................: 40 kB 4.0 kB/s
     http_req_blocked...............: avg=1.61ms   min=0s    med=0s     max=11.08ms p(90)=8.95ms  p(95)=8.95ms
     http_req_connecting............: avg=880.62µs min=0s    med=0s     max=9.96ms  p(90)=7.44ms  p(95)=7.44ms
     http_req_duration..............: avg=2.01s    min=2s    med=2.01s  max=2.05s   p(90)=2.02s   p(95)=2.03s
       { expected_response:true }...: avg=2.01s    min=2s    med=2.01s  max=2.05s   p(90)=2.02s   p(95)=2.03s
     http_req_failed................: 0.00% ✓ 0         ✗ 500
     http_req_receiving.............: avg=2s       min=1.99s med=2s     max=2.02s   p(90)=2.01s   p(95)=2.01s
     http_req_sending...............: avg=42.53µs  min=0s    med=0s     max=1.5ms   p(90)=0s      p(95)=0s
     http_req_tls_handshaking.......: avg=0s       min=0s    med=0s     max=0s      p(90)=0s      p(95)=0s
     http_req_waiting...............: avg=9.38ms   min=0s    med=5.94ms max=35.24ms p(90)=26.99ms p(95)=32.15ms
     http_reqs......................: 500   49.382488/s
     iteration_duration.............: avg=2.01s    min=2s    med=2.01s  max=2.06s   p(90)=2.03s   p(95)=2.04s
     iterations.....................: 500   49.382488/s
     vus............................: 100   min=100     max=100
     vus_max........................: 100   min=100     max=100


running (10.1s), 000/100 VUs, 500 complete and 0 interrupted iterations
default ✓ [======================================] 100 VUs  10s
```

테스트 결과 출력 문장이다. 

```
scenarios: (100.00%) 1 scenario, 100 max VUs, 40s max duration (incl. graceful stop):
           * default: 100 looping VUs for 10s (gracefulStop: 30s)
```

위 시간을 보면 100명이 최대 40초 동안 테스트 하는 시나리오로 나온다. 실 테스트 시간은 10초이지만 gracefulStop의 기본값 30초를 더해서 그렇다. gracefulStop 옵션은 가상 유저를 테스트 중에 변경하는 시나리오에서 갑자기 유저를 변경하면 데이터 급변 현상이 일어나므로 최소 30초 동안은 기존 유저값이 유지된다는 의미다.

그 외 눈여겨 볼 것들은 다음과 같다.

- http_req_duration: HTTP 요청 기간에 대한 결과
- http_req_failed: 요청 실패 비율
- http_reqs: 요청 발생 횟수
- http_iteration_duration: 요청이 완료되고 다시 시작될 때까지 걸리는 시간