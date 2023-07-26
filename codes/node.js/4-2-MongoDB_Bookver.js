const { MongoClient} = require('mongodb');
const uri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

//클라이언트 생성
const client = new MongoClient(uri);

async function run() {
    await client.connect();
    const adminDB = client.db('test').admin();
    //DB 정보 가져오기
    const listDatabases = await adminDB.listDatabases();
    console.log(listDatabases);
    return "OK";
}

run()
.then(console.log)
.catch(console.error)
.finally(() => client.close());