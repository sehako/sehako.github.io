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