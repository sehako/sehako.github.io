const http = require("http");
const url = require("url");
http
.createServer((req, res) => {
    const path = url.parse(req.url, true).pathname;
    res.setHeader("Content-Type", "text/html");
    
    //urlMap에 path가 있는지 확인
    if (path in urlMap) {
        //urlMap에 path값으로 매핑된 함수 실행
        urlMap[path](req, res);
    }
    else {
        notFound(req, res);
    }
}).listen("3000", () => console.log("라우터 리펙토링 예제"));

const user = (req, res) => {
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

const urlMap = {
    "/": (req, res) => res.end("HOME"),
    "/user": user,
    "/feed": feed,
};