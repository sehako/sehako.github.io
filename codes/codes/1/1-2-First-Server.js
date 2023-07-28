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