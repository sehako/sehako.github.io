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