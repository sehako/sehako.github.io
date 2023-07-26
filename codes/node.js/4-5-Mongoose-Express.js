const express = require("express");
const bodyParser = require("body-parser");
const mongoose = require("mongoose");
const Person = require("./4-3-Mongoose")

mongoose.set("strictQuery", false);

const app = express();
app.use(bodyParser.json());
app.listen(3000, async () => {
    console.log("서버 시작");
    const mongodbUri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

    mongoose
    .connect(mongodbUri, {useNewUrlParser: true})
    .then(console.log("Connected to MongoDB"));
});

//모든 데이터 출력
app.get("/person", async (req, res) => {
    const person = await Person.find({});
    res.send(person);
});

//특정 이메일로 찾기
app.get("/person/:email", async (req, res) => {
    const person = await Person.findOne({email: req.params.email});
    res.send(person);
});

//데이터 추가
app.post("/person", async (req, res) => {
    const person = new Person(req.body);
    await person.save();
    // const result = await Person.create(req.body);
    // res.send(result)
    res.send(person);
});

//데이터 수정
app.put("/person/:email", async (req, res) => {
    const person = await Person.findOneAndUpdate(
        {email: req.params.email},
        {$set: req.body},
        {new: true}
    );
    // const person = await Person.updateOne({ email: req.params.email },
    //     { $set: req.body });
    console.log(person);
    res.send(person);
});

//데이터 삭제
app.delete("/person/:email", async (req, res) => {
    await Person.deleteMany({email: req.params.email});
    res.send({success: true});
});
