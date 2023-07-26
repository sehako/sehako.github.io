---
title:  "[Node.js] 04 - MongoDB"
excerpt: " "

categories:
  - Node.js

toc: true
toc_sticky: true
 
date: 2023-07-25
---

# MongoDB

몽고디비는 NoSQL DB다. NoSQL DB는 모델에 따라서 키-벨류, 컬럼, 다큐먼트, 그래프 타입으로 분류한다. 몽고디비는 다큐먼트 타입이고, 이런 타입은 json과 유사한 형식의 객체를 담은 데이터를 저장한다.

## 특징

json을 바이너리 형식으로 저장하는 bson 데이터 형식을 사용해 데이터를 저장한다. 해당 형식은 기존 json에서는 지원하지 않는 자료형인 Date와 BinData 타입을 지원한다. 몽고디비의 장단점은 다음과 같다.

**장점**

- 스키마 미지정이 가능해 데이터 저장의 유연성이 있음
- 단일 문서 검색 시 여러 테이블을 조인하는 것보다 빠름
- 클러스터를 지원하기 때문에 스케일아웃이 쉬움
- 다른 NoSQL 대비 인덱스 지원이 잘되어 있음

**단점**

- 메모리를 많이 사용
- 디스크 저장 공간을 RDB에 비해 많이 사용
- 복잡한 조인은 사용하기 힘듬
- 트랜잭션 지원이 RDB에 비해 약함

# 몽고디비 사용해보기

몽고디비를 가입하고 기본적인 설정을 마치면 해당 연결 코드를 복사할 수 있다.

```js
const { MongoClient, ServerApiVersion } = require('mongodb');
const uri = "mongodb+srv://ID:password@clusterInfo/?retryWrites=true&w=majority";

// 안정적인 API버전을 위해 옵션을 추가한 클라이언트 객체
const client = new MongoClient(uri, {
  serverApi: {
    version: ServerApiVersion.v1,
    strict: true,
    deprecationErrors: true,
  }
});

async function run() {
  try {
    // DB 서버에 연결
    await client.connect();
    // 연결 성공 확인을 위해 ping 전송
    await client.db("admin").command({ ping: 1 });

    //adminDB 인스턴스
    const adminDB = client.db('test').admin();
    //DB 정보 가져오기
    const listDatabases = await adminDB.listDatabases();
    console.log(listDatabases);
    
    console.log("Pinged your deployment. You successfully connected to MongoDB!");
    return "OK";
  } 
  finally {
    // 에러나 작업 종료 시 클라이언트 종료는 필수
    await client.close();
  }
}

run()
```

코드 실행 전 `mongodb` 패키지 설치가 필요하다.

```
npm install mongodb
```

다음은 몽고디비에서 연결을 위한 예제 코드이다. `async` 비동기 함수로 실행된다는 것을 알 수 있다. 몽고디비를 불러와 uri값에 따라 연결하는 것을 볼 수 있다. `retry=Writes=true`는 연결을 찾을 수 없을 때 쓰기 작업을 자동으로 재시도하는 옵션이다. 

`w=majority`는 쓰기 시도 시 다수의 인스턴스에 쓰기 요청을 전달하고 성공 확인을 받는다. 예를 들어, DB가 3대 있다면 2대의 승인이 있어야 쓰기가 가능하다는 뜻으로, 이는 1대의 DB에만 데이터를 저장한다면, 해당 DB의 장애 발생 시 데이터의 유실 위험이 있기 때문이다.

책에 있는 예제는 좀 더 간단하다.

```js
const { MongoClient} = require('mongodb');
const uri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

//클라이언트 생성
const client = new MongoClient(uri);

async function run() {
    await client.connect();
    const adminDB = client.db('test').admin();
    //DB 정보 가져오기
    const listDatabases = await adminDB.listDatabases();
    console.log(listDatabases);
    return "OK";
}

run()
.then(console.log)
.catch(console.error)
.finally(() => client.close());
```

클라이언트 객체 생성시 API버전을 지정해주는 부분이 없다.

## CRUD API 만들기 

```js
const { MongoClient, ServerApiVersion } = require('mongodb');
const uri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

//클라이언트 생성
const client = new MongoClient(uri, {
    serverApi: {
      version: ServerApiVersion.v1,
      strict: true,
      deprecationErrors: true,
    }
    ,useNewUrlParser: true
});

//CRUD(Create, Read, Update, Delete)
async function main() {
    try {
        //커넥션을 생성하고 연결 시도
        await client.connect();

        console.log('MongoDB 접속 성공');

        //person 컬렉션 가져오기
        const collection = client.db('test').collection('person');

        await collection.insertOne({name: 'Halo', age: 30});
        console.log('문서 추가 완료');

        const documents = await collection.find({name: 'Halo'}).toArray();
        console.log('찾은 문서: ', documents);

        await collection.updateOne({name: 'Halo'}, {$set: {age: 31}});
        console.log('문서 업데이트');

        const updatedDocuments = await collection.find({name: 'Halo'}).toArray();
        console.log('갱신된 문서: ', updatedDocuments);

        // await collection.deleteOne({name: 'Halo'});
        // console.log('문서 삭제');

        await client.close();
    }
    catch(err) {
        console.erroe(err);
    }
}

main();
```

`connect()` 함수를 사용해 서버에 연결할 때는 `useNewUrlParser` 옵션을 사용해야 한다고 나와있지만 안써도 기능은 잘 동작하였다...

`db()` 함수는 지정된 DB를 사용한다는 뜻이고 `collection()`은 지정된 컬랙션을 사용한다는 뜻이다. DB가 생성되어 있지 않으면 새로 생성한다. 

문서 하나 추가 시 `insertOne()`을 사용하고 인수로는 json 형식의 객체를 넣는다. `find()`로 문서를 찾을 수 있다. 결과값이 여러 개인 경우 `toArray()`를 이용하면 배열로 반환해준다. 문서 갱신은 찾는 데 사용할 json 객체를 첫 번째 인수에, 두 번째 인수에 `$set`으로 업데이트할 값을 넣는다. 그 외에도 다양한 쿼리 연산자가 있다.

- 콤파스: DB에 문서들이 의도대로 수정되었는지 확인을 위한 GUI 프로그램이다. 설치 후 코드의 `uri`값을 복사 붙여넣기 하면 이용할 수 있다.

# 몽구스

몽구스는 네이티브 드라이버인 `mongodb` 패키지보다 좀 더 편리한 기능을 제공하는 라이브러리다. 객체를 도규먼트로 매핑하는 ODM 기능이 대표적이다. 그리고 몽고디비에는 스키마를 지정하는 기능이 없지만, 몽구스를 이용하여 스키마를 지정할 수 있다.

```
npm install mongoose
```

```js
//파일 이름은 4-4-Mongoose.js
var mongoose = require("mongoose");
var Schema = mongoose.Schema;

//스키마 객체 생성
const personSchema = new Schema({
    name: String,
    age: Number,
    email: {type: String, required: true},
});

//모델 객체 생성
module.exports = mongoose.model('Person', personSchema);
```

몽구스는 몽고디미 컬렉션과 연동되어 CRUD를 수행할 수 있다. `exports`로 내보내기를 했기 때문에 다른 파일에서 `require()`로 불러올 수 있다.

```js
const express = require("express");
const bodyParser = require("body-parser");
const mongoose = require("mongoose");
const Person = require("./4-3-Mongoose")

mongoose.set("strictQuery", false);

const app = express();
app.use(bodyParser.json());
app.listen(3000, async () => {
    console.log("서버 시작");
    const mongodbUri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

    mongoose
    .connect(mongodbUri, {useNewUrlParser: true})
    .then(console.log("Connected to MongoDB"));
});

//모든 데이터 출력
app.get("/person", async (req, res) => {
    const person = await Person.find({});
    res.send(person);
});

//특정 이메일로 찾기
app.get("/person/:email", async (req, res) => {
    const person = await Person.findOne({email: req.params.email});
    res.send(person);
});

//데이터 추가
app.post("/person", async (req, res) => {
    const person = new Person(req.body);
    await person.save();
    // const result = await Person.create(req.body);
    // res.send(result)
    res.send(person);
});

//데이터 수정
app.put("/person/:email", async (req, res) => {
    const person = await Person.findOneAndUpdate(
        {email: req.params.email},
        {$set: req.body},
        {new: true}
    );
    // const person = await Person.updateOne({ email: req.params.email },
    //     { $set: req.body });
    console.log(person);
    res.send(person);
});

//데이터 삭제
app.delete("/person/:email", async (req, res) => {
    await Person.deleteMany({email: req.params.email});
    res.send({success: true});
});
```

`find({})`는 모든 값을 쿼리하는 방법이다. 하지만 이게 문제가 되는 경우가 많아 에러를 내도록 유도한다. 이 에러를 무시하기 위해 `strictQuery`를 `false`로 설정한다. 

`body-parser` 미들웨어를 추가해야만 HTTP에서 `Body`를 파싱할 수 있다.

`connect`로 몽고 디비와 커넥션을 맺는다. 마찬가지로 아틀라스 사용 시 `useNewUrlParser: true`로 설정해준다.

`find`는 다음 형태로 구성되어 있다. `find(filter, projection, option)` filter는 json 형식으로 키값을 넣어 해당 데이터를 가져온다. projection은 결과에 표시할 데이터 필드를 지정한다. 1로 필드를 포함하거나 0으로 필드를 제외할 수 있다. option은 정렬(sort)하거나 결과 문서 수를 지정(limit)하거나 결과 문서 중 앞에서 제외(skip)할 문서 수를 지정할 수 있다.

`findOne`은 `find`와 같지만 하나의 결과값을 반환한다.

문서 추가 시 `new`로 모델 생성 후 `save()` 또는 `create()` 메소드를 호출하면 DB에 저장한다.

하나의 문서만 수정하는 경우 `findOneAndUpdate()` 또는 `updateOne()`을 사용한다. 여러 개를 동시에 수정할 때는 `updateMany()`를 사용한다. 인수는 `updateOne()`과 동일하다.

하나만 삭제하고 문서를 결과값으로 받고 싶은 경우 `findOneAndDelete()`, 하나만 삭제할 때는 `deleteOne()`, 여러 개를 삭제할 때는 `deleteMany()`를 사용한다. json 형식을 인수로 넣어주면 된다.

## REST 클라이언트로 API 테스트

http 파일에 여러 테스트 요청을 작성할 수 있고 각 요청은 3개 이상의 `###`으로 구분한다. @변수명 = 값으로 변수 선언이 가능하며 {{변수명}}으로 사용한다. 실제 vscode에서 이 파일을 작성하면 ###과 요청 사이에 `Send Request`라는 버튼이 생긴다. 버튼을 클릭하면 요청이 시작되고, 결과값을 vscode의 새 창과 터미널에서 보여준다.

```http
@server = http://localhost:3000

###GET 요청 전송
GET {{server}}/person

### POST 요청 전송
POST {{server}}/person
Content-Type: application/json

{
    "name": "TEST",
    "age": 30,
    "email": "test@gmail.com"
}

### 생성 문서 확인
GET {{server}}/person/test@gmail.com 

### 문서 수정
PUT {{server}}/person/test@gmail.com
Content-Type: application/json

{
    "age": 32
}

### 문서 삭제
DELETE {{server}}/person/test@gmail.com
```

REST는 HTTP를 테스트하는 편리한 도구이기 때문에 익숙해지면 좋다.