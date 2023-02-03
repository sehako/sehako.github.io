const fs = require('fs') //파일 시스템을 불러오는 모듈(?)

fs.readFile('sample.txt', 'utf8', function(err, data) {
    console.log(data);
});