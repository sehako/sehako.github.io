const url = require("url");
const express = require("express");
const app = express();
const port = 3000;

app.listen(port, () => {
    console.log("익스프레스로 라우터 리펙토링");
});

//get 메소드로 라우팅 설정
app.get("/", (_, res) => res.end("HOME"));
app.get("/user", user);
app.get("/feed", feed);

//호이스팅을 위해 함수로 변환
function user(req, res) {
    const user = url.parse(req.url, true).query;
    //응답을 json 타입으로 출력, 자동으로 utf-8 설정
    res.json(`[user] name: ${user.name}, age: ${user.age}`);
}

function feed(_, res) {
    res.json(`<ul><li>pic1</li><li>pic2</li><li>pic3</li></ul>
    `);
}