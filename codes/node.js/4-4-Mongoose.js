var mongoose = require("mongoose");
var Schema = mongoose.Schema;

//스키마 객체 생성
const personSchema = new Schema({
    name: String,
    age: Number,
    email: {type: String, required: true},
});

//모델 객체 생성
module.exports = mongoose.model('Person', personSchema);