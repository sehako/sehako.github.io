const { MongoClient, ServerApiVersion } = require('mongodb');
const uri = "mongodb+srv://dhtpgkr1999:osh1012@cluster0.cmv5a8b.mongodb.net/?retryWrites=true&w=majority";

// Create a MongoClient with a MongoClientOptions object to set the Stable API version
const client = new MongoClient(uri, {
  serverApi: {
    version: ServerApiVersion.v1,
    strict: true,
    deprecationErrors: true,
  }
});

async function run() {
  try {
    // Connect the client to the server	(optional starting in v4.7)
    await client.connect();
    // Send a ping to confirm a successful connection
    await client.db("admin").command({ ping: 1 });

    //adminDB 인스턴스
    const adminDB = client.db('test').admin();
    //DB 정보 가져오기
    const listDatabases = await adminDB.listDatabases();
    console.log(listDatabases);
    
    console.log("Pinged your deployment. You successfully connected to MongoDB!");
    return "OK";
  } 
  finally {
    // Ensures that the client will close when you finish/error
    await client.close();
  }
}

run()
