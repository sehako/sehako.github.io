const { MongoClient, ServerApiVersion } = require('mongodb');
const uri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

//클라이언트 생성
const client = new MongoClient(uri, {
    serverApi: {
      version: ServerApiVersion.v1,
      strict: true,
      deprecationErrors: true,
    }
    ,useNewUrlParser: true
});

//CRUD(Create, Read, Update, Delete)
async function main() {
    try {
        //커넥션을 생성하고 연결 시도
        await client.connect();

        console.log('MongoDB 접속 성공');

        //person 컬렉션 가져오기
        const collection = client.db('test').collection('person');

        await collection.insertOne({name: 'Halo', age: 30});
        console.log('문서 추가 완료');

        const documents = await collection.find({name: 'Halo'}).toArray();
        console.log('찾은 문서: ', documents);

        await collection.updateOne({name: 'Halo'}, {$set: {age: 31}});
        console.log('문서 업데이트');

        const updatedDocuments = await collection.find({name: 'Halo'}).toArray();
        console.log('갱신된 문서: ', updatedDocuments);

        // await collection.deleteOne({name: 'Halo'});
        // console.log('문서 삭제');

        await client.close();
    }
    catch(err) {
        console.erroe(err);
    }
}

main();