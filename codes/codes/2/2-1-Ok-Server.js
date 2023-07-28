const http = require("http");
const server = http.createServer((req, res) => {
    //텍스트를 html로 해석
    res.setHeader("Content-Type", "text/html");
    //숫자나 파일 같은 데이터도 가능
    res.end("OK");
});

server.listen("3000", () => console.log("OK 서버 시작"));