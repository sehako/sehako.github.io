//list() 함수 추가
const paginator = require('../utils/paginator');
const { ObjectId } = require('mongodb');

async function writePost(collection, post) {
    post.hits = 0;
    post.createDt = new Date().toISOString();
    return await collection.insertOne(post);
}

async function list(collection, page, search) {
    const perPage = 10;
    //title이 search와 일치하는지 확인
    const query = { title: new RegExp(search, 'i') };
    //10개만 가져오고 설정된 개수만큼 건너 뜀
    const cursor = collection.find(query, { limit: perPage, skip: (page - 1) * perPage }).sort({
        createDt: -1,
    });
    // 검색되는 게시물의 총 합
    const totalCount = await collection.count(query);
    const posts = await cursor.toArray();
    //페이지네이터 생성
    const paginatorObj = paginator({ totalCount, page, perPage: perPage });
    return [posts, paginatorObj];
}

const projectionOption = {
    //결과값에서 일부만 가져올 때 사용
    projection: {
        password: 0,
        "comments.password": 0,
    },
};

async function getDetailPost(collection, id) {
    return await collection
    .findOneAndUpdate({ _id:ObjectId(id) }, { $inc: { hits: 1 } }, projectionOption);
}

async function getPostByIdAndPassword(collection, { id, password }) {
    //findOne 함수 사용
    return await collection.findOne({ _id: ObjectId(id), password: password },
    projectionOption)
}

//id로 데이터 불러오기
async function getPostById(collection, id) {
    return await collection.findOne({ _id: ObjectId(id) }, projectionOption);
}

//게시글 수정
async function updatePost(collection, id, post) {
    const toUpdatePost = {
        $set: {
            ...post,
        },
    };
    return await collection.updateOne({ _id: ObjectId(id) }, toUpdatePost);
}

module.exports = {
    list,
    writePost,
    getDetailPost,
    getPostById,
    getPostByIdAndPassword,
    updatePost,
};