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