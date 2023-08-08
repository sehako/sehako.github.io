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
// const result = registerByPromise(myUser);
// result.then(console.log);

//동시에 여러 Primise 객체 호출
allResult = Promise.all([saveDB(myUser), sendEmail(myUser), getResult(myUser)]);
allResult.then(console.log);