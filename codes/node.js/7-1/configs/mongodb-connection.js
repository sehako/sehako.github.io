const { MongoClient } = require("mongodb");

const uri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

//DB 커넥션 연결 함수 반환
module.exports = function(callback) {
    return MongoClient.connect(uri, callback);
}