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