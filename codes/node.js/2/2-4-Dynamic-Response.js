const http = require("http");
const url = require("url");
http
.createServer((req, res) => {
    const path = url.parse(req.url, true).pathname;
    res.setHeader("Content-Type", "text/html");

    if(path == "/user") {
        user(req, res);
    }
    else if(path == "/feed") {
        feed(req, res);
    }
    else {
        notFound(req, res);
    }
}).listen("3000", () => console.log("동적 응답 예제"));

const user = (req, res) => {
    //동적 응답을 위해서 request url를 쿼리한 다음 태그 값을 출력 문에 사용
    //?이후 키=값으로 추가
    //http://localhost:3000/user?name=mike&age=20
    const userInfo = url.parse(req.url, true).query;
    res.end(`[user] name: ${userInfo.name}, age: ${userInfo.age}`);
};

const feed = (req, res) => {
    res.end(`<ul>
    <li>pic1</li>
    <li>pic2</li>
    <li>pic3</li>
    </ul>
    `);
};

const notFound = (req, res) => {
    res.statusCode = 404;
    res.end("404 page not found");
};