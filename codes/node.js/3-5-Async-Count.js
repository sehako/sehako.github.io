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