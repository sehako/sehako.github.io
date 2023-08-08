const ws = new WebSocket('ws://localhost:3000');

// 메시지 발송
function sendMessage() {
    ws.send(document.getElementById('message').value);
}

function WebSocketClose() {
    console.log("종료 누름");
    ws.close();
}

ws.onopen = function() {
    console.log('클라이언트 접속 완료');
};

// 이벤트 핸들러, 서버에서 메시지 수신 시 실행
ws.onmessage = function(event) {
    // 엔터 키를 <br>로 변경
    let message = event.data.replace(/(\r\n|\n|\r)/g, '<br>');
    let el = document.createElement('div');
    el.innerHTML = message;
    el.id = 'message';
    document.getElementById('messages').append(el);
};

ws.onclose = function(e) {
    console.log('종료');
    document.getElementById('message').append('서버 접속 종료');
}