const WebSocket = require('ws');
const server = new WebSocket.Server({ port: 3000 });

// 접속 이벤트 핸들러
server.on('connection', ws => {
    // 클라이언트로 메시지 전송
    ws.send('[서버 접속 완료]');
    // 클라이언트에서 메시지 수신 시 이벤트 핸들러
    ws.on('message', message => {
        ws.send(`서버로부터 응답: ${message}`);
    });

    // 접속 종료 이벤트
    ws.on('close', () => {
        console.log('클라이언트 접속 해제');
    });
});