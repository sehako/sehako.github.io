const express = require('express');
const handlebars = require('express-handlebars');
const app = express();
const mongodbConnection = require('./configs/mongodb-connection');
const postService = require('./services/post-service');
const { ObjectId } = require('mongodb');
let collection;

//req.body와 POST 요청을 해석하기 위한 미들웨어
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

//탬플릿 엔진으로 핸들바 등록
// app.engine('handlebars', handlebars.engine());
//커스텀 핸들바
app.engine(
    'handlebars',
    handlebars.create({
        helpers: require('./configs/handlebars-helpers'),
    }).engine,
);
//웹페이지 로드 시 사용할 템플릿 엔진 설정
app.set('view engine', 'handlebars');
//뷰 디렉터리를 views로 설정
app.set('views', __dirname + '/views');

//라우터 설정
app.get('/', async (req, res) => {
    // res.render('home', { title: '테스트 게시판', message: '만나서 반갑습니다!' });
    
    //리스트 페이지 추가부분
    const page = parseInt(req.query.page) || 1; //현재 페이지 데이터
    const search = req.query.search || ""; //검색어 데이터

    try {
        //글 목록과 페이지네이터를 가져옴
        const [posts, paginator] = await postService.list(collection, page, search);
        //리스트 페이지 렌더링
        res.render('home', { title: '테스트 게시판', search, paginator, posts });
    }
    catch(err) {
        console.error(err);
        //에러 발생하면 빈 값으로 렌더링
        res.render('home', { title: '테스트 게시판' });
    }
});

app.get('/write', (_, res) => {
    res.render('write', { title: '테스트 게시판', mode: 'create' });
});

app.get('/detail/:id', async (req, res) => {
    const result = await postService.getDetailPost(collection, req.params.id);
    res.render('detail', {
        title: '테스트 게시판',
        post: result.value,
    });
});

//수정 페이지로 이동
app.get('/modify/:id', async (req, res) => {
    const { id } = req.params.id;
    //게시글 id 불러오기
    const post = await postService.getPostById(collection, req.params.id);
    console.log(post);
    res.render('write', { title: '테스트 게시판', mode: 'modify', post });
});

//패스워드 체크
app.post('/check-password', async (req, res) => {
    const { id, password } = req.body;

    const post = await postService.getPostByIdAndPassword(collection, { id, password });
    if(!post) {
        return res.status(404).json({ isExist: false })
    }
    else {
        return res.json({ isExist: true })
    }
});

//게시글 수정 API
app.post('/modify/', async (req, res) => {
    const { id, title, writer, password, content } = req.body;

    const post = {
        title,
        writer,
        password,
        content,
        createDt: new Date().toISOString(),
    };

    //업데이트 결과
    const result = postService.updatePost(collection, id, post);
    res.redirect(`/detail/${id}`);
});

//글쓰기
app.post('/write', async (req, res) => {
    const post = req.body;
    //결과 반환
    const result = await postService.writePost(collection, post);
    //다큐먼트의 _id를 이용하여 상세 페이지로 이동
    res.redirect(`/detail/${result.insertedId}`);
});

app.post('/write-comment', async (req, res) => {
    const { id, name, password, comment } = req.body;
    const post = await postService.getPostById(collection, id);

    //게시글에 기존 댓글 리스트가 있으면 추가
    if(post.comments) {
        post.comments.push({
            idx: post.comments.length + 1,
            name,
            password,
            comment,
            createDt: new Date().toISOString(),
        });
    }
    else {
        post.comments = [
            {
                idx: 1,
                name,
                password,
                comment,
                createDt: new Date().toISOString(),
            },
        ];
    }

    //업데이트, 후에 상세페이지로 리다이렉트
    postService.updatePost(collection, id, post);
    return res.redirect(`/detail/${id}`);
});

//게시글 삭제
app.delete('/delete', async (req, res) => {
    const { id, password } = req.body;

    try {
        //collection의 deleteOne을 사용
        const result = await collection.deleteOne({ _id: ObjectId(id), password: password });
        //삭제 경과가 잘 못된 경우의 처리
        if(result.deletedCount !== 1) {
            console.log('삭제 실패');
            return res.json({ isSuccess: false });
        }
        return res.json({ isSuccess: true });
    }
    catch(err) {
        console.error(err);
        return res.json({ isSuccess: false });
    }
});

//댓글 삭제
app.delete('/delete-comment', async (req, res) => {
    const { id, idx, password } = req.body;

    //post의 comments 안에 있는 특정 댓글 데이터 찾기
    const post = await collection.findOne(
        {
            _id: ObjectId(id),
            comments: { $elemMatch: { idx: parseInt(idx). password } },
        },
        postService.projectionOption,
    );

    //데이터 없으면 false 반환 
    if(!post) {
        return res.json({ isSuccess: false });
    }

    post.comments = post.comments.filter((comment) => comment.idx != idx);
    postService.updatePost(collection, id, post);
    return res.json({ isSuccess: true });
});

app.listen(3000, async () => {
    console.log('Server started');
    const mongoClient = await mongodbConnection();
    collection = mongoClient.db().collection("post");
    console.log("DB connected");
});
