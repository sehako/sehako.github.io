---
title:  "[Node.js] 03 - 비동기 처리"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-24
---

비동기 프로그래밍은 이전 작업의 완료 유무에 관계 없이 다음 작업이 실행된다. js는 런타임에서 싱글 스레드로 동작한다. 하지만 `Callback`, `Promise`, `async`를 이용하여 비동기 처리를 할 수 있다.

# Callback

실행 가능한 함수를 인자로 전달하여 호출하는 방식이다. 

```js
const DB = [];

//register > saveDB > sendEmail > getResult 순으로 함수 실행
function register(user) { //삼중 중첩 콜백
    return saveDB(user, function (user) {
        return sendEmail(user, function (user) {
            return getResult(user);
        });
    });
}

//DB 리스트에 저장 후 콜백 실행
function saveDB(user, callback) {
    DB.push(user);
    console.log(`save ${user.name} to DB`);
    return callback(user);
}

function sendEmail(user, callback) {
    console.log(`email to ${user.email}`);
    return callback(user);
}

function getResult(user) {
    return `success register ${user.name}`;
}

const result = register({ email: "test@gmail.com", password: "1234", name: "test" });
console.log(result);
```

책에서는 커비 주문을 예시로 들었다. 커피를 주문하고 고객이 다른 일을 하고 있을 때, 커피 제작이 완료되면 직원이 고객을 호출(callback)한다는 것이다.

# Promise

콜백의 문제는 콜백의 단계가 늘어날수록, 코드의 가독성이 떨어진다는 것이다. 

```js
//콜백의 문제를 해결하기 위한 Promise
const DB = [];

function saveDB(user) {
    const oldDBSize = DB.length;
    DB.push(user);
    console.log(`save ${user.name} to DB`);
    //Promise 객체 반환, 성공시 resolve 실패시 reject를 실행
    return new Promise((resolve, reject) => {
        if (DB.length > oldDBSize) {
            resolve(user);
        }
        else {
            reject(new Error("Save DB Error!"));
        }
    });
}

function sendEmail(user) {
    console.log(`email to ${user.email}`);
    return new Promise((resolve) => {
        resolve(user);
    });
}

function getResult(user) {
    return new Promise((resolve, reject) => {
        resolve(`success register ${user.name}`);
    });
}

function registerByPromise(user) {
    //연속 호출할 함수 지정
    const result = saveDB(user)
    .then(sendEmail)
    .then(getResult)
    //에러 처리
    .catch(error => new Error(error))
    //성공 실패 여부에 관계 없이 실행
    .finally(() => console.log("완료"));
    console.log(result);
    return result;
}

const myUser = {email: "test@gmail.com", password: "1234", name: "test"};
const result = registerByPromise(myUser);
result.then(console.log);
```

`Promise`는 비동기 실행을 동기화하는 구문으로 사용한다. 이행(resolve), 거절(reject), 대기 세 가지 상태를 가질 수 있는 객체로 `new` 연산자로 인스턴스를 생성할 수 있다. 비동기 실행이지만 `then`메소드로 호출 순서를 매길 수 있다.

**then()의 구조**

```js
then(onFulfilled, onRejected)

then(
    (value) => { },
    (reason) => { }
)
```

매개변수로 함수를 이행 또는 거절 시 실행할 함수를 선언한다.

## 동시에 여러 Promise 객체 호출
위 코드에 다음 코드를 추가하면 동시에 여러 `Promise` 객체를 호출해 결과값을 받는다. 나열된 순서와 상관 없이 동시에 실행되고 배열을 결과로 반환받는다.

```js
// 동시에 여러 Primise 객체 호출
allResult = Promise.all([saveDB(myUser), sendEmail(myUser), getResult(myUser)]);
allResult.then(console.log);
```

## 복잡한 Promise 예제

```js
const axios = require("axios");
const url = "https://raw.githubusercontent.com/wapj/jsbackend/main/movieinfo.json";

axios
//매개변수의 정보를 가져옴
.get(url)
.then((result) => {
    if(result.status != 200) {
        throw new Error("요청 실패");
    }

    if(result.data) {
        return result.data;
    }

    throw new Error("데이터 없음");
})
.then((data) => {
    if(!data.articleList || data.articleList.size == 0) {
        throw new Error("데이터 없음");
    }
    return data.articleList;
})
//제목과 순위 정보로 분리
.then((articles) => {
    return articles.map((article, idx) => {
        return {title:article.title, rank: idx + 1};
    });
})
.then((result) => {
    for(let movieInfo of result) {
        console.log((`[${movieInfo.rank}위] ${movieInfo.title}`));
    }
})
//then 중간에 발생한 에러 처리
.catch((error) => {
    console.log("에러 발생");
    console.error(error);
})
```

`axios`로 기존 웹 주소에 존재하는 영화 데이터 20개를 긁어와 오름차순으로 보여주는 코드다.

# async

js에서 가장 최근에 도입된 비동기 처리 방식이다. `async`가 붙은 함수는 `Promise`를 반환한다.

```js
// 프로미스를 반환하는 함수 키워드 async
async function myName() {
    return "Name";
}

async function showName() {
    const name = await myName();
    console.log(name);
}

console.log(showName());
```

여기서 `await` 키워드는 `async`와 짝지어 사용하는 키워드이다. 해당 키워드는 `Proimise` 객체의 실행이 완료되기를 기다린다. `await`는 `async`를 사용한 함수 내에서만 사용할 수 있다.

## 10까지 세는 프로그램

```js
function waitOneSecond(msg) {
    return new Promise((resolve, _) => {
        //1초 후 메시지 출력
        setTimeout(() => resolve(`${msg}`), 1000);
    });
}

async function countOneToTen() {
    for(let x of [...Array(10).keys()]) {
        let result = await waitOneSecond(`${x + 1}초`);
        console.log(result);
    }
    console.log("완료");
}

countOneToTen();
```

여기서 눈여겨 볼 것은 `setTImeout()`은 `Promise`를 반환하지 않기 때문에 비동기 함수에서 `await` 키워드를 사용하기 위해서 직접 객체를 생성하여 반환한 것이다.

## async로 만든 영화 순위 출력

```js
const axios = require("axios");
async function getTop20Movies() {
    const url = "https://raw.githubusercontent.com/wapj/jsbackend/main/movieinfo.json";
    try {
        //데이터 받기 위해서 대기
        const result = await axios.get(url);
        const {data} = result;

        if(!data.articleList || data.articleList.size == 0) {
            throw new Error("데이터 없음");
        }

        const movieInfos = data.articleList.map((article, idx) => {
            return {title:article.title, rank: idx + 1};
        });

        for(let movieInfo of movieInfos) {
            console.log(`[${movieInfo.rank}위] ${movieInfo.title}`);
        }
    }
    catch (err) {
        throw new Error(err);
    }
}

getTop20Movies();
```

`then`으로 과도하게 연결된 코드를 개선했고. 예외처리는 `try catch`를 통해 구현하였다.

`setTimeout()`을 사용하거나, 여러 태스크를 동시에 실행해야 하는 경우에는 `Promise`를, 그 외에는 대부분 `await`를 사용한다고 한다. 