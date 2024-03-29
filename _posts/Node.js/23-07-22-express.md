---
title:  "[Node.js] 02 - 익스프레스로 웹 서버 구현"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-22
---

# 라우터

일반적인 웹 서버는 경로에 따라서 다른 응답을 주는데, 이런 기능을 라우팅이라고 한다.

```js
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
```

`path` 변수는 요청으로 받은 url의 pathname을 얻어낸다. 이는 url이 `localhost:3000/user`이라면, pathname은 `/user`가 된다. 쿼리 스트링은 HTTP 요청을 보낼 때 원하는 값을 보내는 방식으로 `키=값`으로 구성되어 있고 여러 개의 쿼리 스트링이라면 `&`으로 구분해 추가한다. 

## 리펙토링

리펙토링은 동작의 결과를 변경시키지 않으면서, 코드의 구조를 재조정하는 작업이다. 가독성을 높이고 유지보수를 편하게 하기 위한 목적으로 주로 사용된다. 위 코드의 경우 `createServer()` 내에서 요청에 대한 응답을 컨트롤하는데, 이런 식의 코드 작성 방식은 작성량이 늘어날수록 가독성이 낮아지고 유지보수가 어려워진다.

```js
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
}).listen("3000", () => console.log("createServer 리펙토링 예제"));

const user = (req, res) => {
    res.end(`[user] name: john, age: 30`);
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
```

## 동적 응답

쿼리 스트링 데이터를 통해 변수에 값을 전달할 수 있고, 이를 웹서버에 출력되는 값으로 이용할 수도 있다. 다음은 `user` 콜백 함수를 조정한 결과다.

```js
const user = (req, res) => {
    //동적 응답을 위해서 request url를 쿼리한 다음 태그 값을 출력 문에 사용
    //?이후 키=값으로 추가
    //http://localhost:3000/user?name=mike&age=20
    const userInfo = url.parse(req.url, true).query;
    res.end(`[user] name: ${userInfo.name}, age: ${userInfo.age}`);
};
```

## 라우터 리펙토링

위 코드는 처리하는 함수의 개수가 많아질수록 요청에 사용되는 분기문(`if`)이 늘어난다.

```js
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
```

`in` 연산자는 객체의 키가 있는지 검사한다. 

```js
const ab = {"a":1, "b":2};
"a" in ab //true
"c" in ab //false
```

js에서는 함수가 일급 객체다. 일급 객체는 값으로 할당이 가능하고 함수의 결과로 반환받을 수도 있다.

`urlMap`이 가장 하단에 위치한 이유는 처리 함수들이 `const`로 선언되었기에 초기화 전에는 읽을 수 없어서 에러가 나기 때문이다.

hoisting 기술은 함수, 클래스, 변수를 끌어올려 선언 전에 사용하도록 하는 기능이다. js에서는 `var`로 선언한 변수, 함수 그리고 클래스 선언이 호이스팅이 가능하다. 

# Express 프레임워크

일반적인 웹서버가 제공하는 기능은 라우팅 뿐만 아니라 정적 파일 서비스, 탬플릿 엔진, 요청/응답 데이터 다루기, 파일 업로드, 쿠키 및 세션 지원, 리다이렉트, 에러 페이지, 마지막으로 미들웨어의 기능을 제공한다. 이런 작업들을 제공하는 Node.js 오픈 소스 웹 서버 중 Express가 가장 널리 사용된다.

```
npm install express
```

터미널에 다음 명령줄을 입력하여 익스프레스를 설치하면 된다.

```js
const express = require("express");
const app = express();
const port = 3000;

app.get("/", (req, res) => {
    res.set({ "Content-Type": "text/html; charset=utf-8"});
    res.end("Hello Express");
})

app.listen(port, () => {
    console.log(`Start Server : use ${port}`);
});
```

## 익스프레스로 라우터 서버 구현

앞서 작성했던 라우터 서버를 익스프레스로 만들면 다음과 같다.

```js
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
```

코드가 훨씬 더 간결해졌다. 심지어 `get()` 메소드로 함수를 매핑해주기만 하면 되기에 유지보수도 수월해질 것이다. 다만, 응답을 json 타입으로 출력하는 `json` 메소드는 `html` 형식이 깨진다. 책에서는 이에 대한 설명이 없다...

처리 함수를 `function`으로 선언한 이유는 호이스팅을 위해서고 `feed` 부분에 `_` 기호는 사용하지 않는 변수지만, 인터페이스 구조상 넣을 수밖에 없을 때의 관례이다.

## 익스프레스로 간단한 API 서버 만들기

익스프레스를 이용하여 게시글을 작성 및 삭제하는 간단한 게시판 API를 만든다. 해당 코드는 DB가 아닌 메모리 기반 동작이다.

```js
const express = require("express");
const app = express();
//게시글 저장 용도로 사용할 리스트
let posts = [];

//req.body를 사용하기 위한 미들웨어 호출
app.use(express.json());
//POST 요청 시 컨텐트 타입이 application/x-www-form-urlencoded인 경우 파싱
app.use(express.urlencoded({ extended: true}));

app.get("/", (req, res) => {
    res.json(posts);
});

app.post("/posts", (req, res) => {
    //HTTP의 body 데이터를 변수에 할당
    const { title, name, text } = req.body;

    //posts에 새로운 게시글 정보 추가
    posts.push({ id: posts.length + 1, title, name, text, createdDt: Date() });
    res.json({ title, name, text });
});

app.delete("/posts/:id", (req, res) => {
    //요청 path 정보에서 id값을 가져옴
    const id = req.params.id;
    //게시글 삭제
    const filteredPosts = posts.filter((post) => post.id !== +id);
    //삭제 확인
    const isLengthhChanged = posts.length !== filteredPosts.length;
    posts = filteredPosts;

    //삭제 확인에 대한 처리문
    if(isLengthhChanged) {
        res.json("OK");
        //빠른 반환
        return;
    }
    res.json("Not Changed");
});

app.listen(3000, () => {
    console.log("Welcome Posts Start!");
});
```

익스프레스에서 미들웨어는 요청과 응답 사이에 로직을 추가하여 전후 처리를 지원하는 함수를 제공한다.

`urlencoded` 부분은 컨텐트 타입이 application/x-www-form-urlencoded인 겨우 파싱해준다. POST 요청은 대부분 이 타입이라. `express.json`과 함께 사용한다. 해당 타입은 `req.body`에 **키=값&키2=값2** 조합 형태의 데이터를 말한다.

`end()`가 아닌 `json()`을 사용한 이유는 전자는 인수로 문자열과 바이트 버퍼 형식만 넣을 수 있기 때문이다. 따라서 리스트와 json 데이터를 처리할 수 있는 후자를 사용한다.

최종적으로 POST 요청이 오는 경우 **title=타이틀&name=이름&text=내용** 형식 데이터를 `urlencoded` 미들 웨어가 객체로 변환하여 `req.body`에 추가한다. 이 값을 아이디와 생성시간을 덧붙여 게시판에 추가한다.

js에서는 주로 `filter()`를 이용하여 배열의 특정 요소를 삭제한다. 그 외에도 `splice()`, `map()`, `reduce()`로 특정 인덱스를 삭제하는 로직을 구현하기도 한다.

### 게시판 테스트

게시판 테스트를 위해서 curl을 사용한다. 

- POST 요청으로 게시글을 등록

```
curl -X POST -N "Content-Type: application/x-www-form-urlencoded" -d "title=title1$name=name1$text=Hello" http://localhost:3000/posts
```

- 게시글 삭제
```
curl -X DELETE http://localhost:3000/posts/1
```
id값을 이용하여 삭제하므로 `/posts/id값`으로 요청해야 한다.