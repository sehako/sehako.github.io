const http = require("http");
//url 모듈 로딩
const url = require("url");
http
.createServer((req, res) => {
    //path 이름 할당, true는 쿼리 스트링도 함께 파싱할건지 설정
    const path = url.parse(req.url, true).pathname;
    res.setHeader("Content-Type", "text/html");

    //path 이름에 따라서 다른 처리를 하도록 구현
    //localhost:3000/user
    if(path == "/user") {
        res.end("[user] name: john, age: 30");
    }
    //localhost:3000/feed
    else if(path == "/feed") {
        res.end(`<ul>
        <li>pic1</li>
        <li>pic2</li>
        <li>pic3</li>
        </ul>
        `);
    }
    else {
        //해당되는 path 이름이 없는 경우 에러 메시지
        res.statusCode = 404;
        res.end("404 page not found");
    }
}).listen("3000", () => console.log("라우터 예제"));