// 프로미스를 반환하는 함수 키워드
async function myName() {
    return "Name";
}

async function showName() {
    const name = await myName();
    console.log(name);
}

console.log(showName());