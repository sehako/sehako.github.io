---
title:  "[Node.js] 11 - 체팅 구현"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-08-01
---

# 웹소캣

웹은 HTTP 프로토콜을 따르기 때문에 요청을 보내야지만 서버가 응답을 준다. 그렇기 때문에 실시간 응답을 위해서는 폴링이나 롱폴링 방법을 사용해야 한다.

하지만 하나의 TCP 서버와 클라이언트 간에 양방향 통신을 할 수 있게 만든 프로토콜인 웹소캣을 통해 양방향 통신을 구현할 수 있다. 웹소캣을 이용하여 브라우저 상에서 실시간성을 요구하는 어플리케이션을 구현할 수 있다.

정리하자면, 웹소캣은 양방향 통신을 지원하여 실시간 네트워킹을 구현하는 것이 용이하다. 

## 동작 방법

핸드 쉐이크와 데이터 전송으로 나눌 수 있다. 쉐이크는 서버와 클라이언트가 커넥션을 맺는 과정으로 한 번만 일어난다. 이 단계가 완료되면 HTTP는 ws로, HTTPS는 wws로 변경되어 데이터를 전송할 수 있게 된다.

# 웹소캣 테스트

웹소캣 테스트를 하기 위한 간단한 예제이다. 우선 다음 패키지를 설치한다.

```
npm install ws
```

그 다음 **server.js**를 작성한다.

```js
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
```

클라이언트 측 구현을 위해 html 파일을 하나 작성한다.

**client.html**

```html
<style>
    .message {
        width:300px; color:white; background-color: black; 
        margin-top: 5px; padding: 5px
    }
</style>

<body>
    <textarea id="message" cols="50" rows="5"></textarea>
    <br>

    <button onclick="sendMessage()">전송</button>
    <button onclick="WebSocketClose()">종료</button>
    <div id="messages"></div>
</body>

<script src="websocket.js"></script>
```

html 파일에서 사용하는 **websocket.js**

```js
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
```

# Nest.js로 웹소캣 어플 제작

socket.io는 웹 소캣을 기반으로 서버와 클라이언트의 양방향 통신을 지원하는 라이브러리다. 웹소캣과 롱폴링 방식을 지원하고 재접속, 브로드캐스팅, 멀티플렉싱(체팅방) 기능도 제공한다.

간단한 체팅 기능을 구현할 것이다. 터미널에서 프로젝트 생성 후 다음 패키지를 설치한다.

```
npm install @nestjs/websockets @nestjs/platform-socket.io
```

정적 파일 서비스를 위해 html 파일을 불러오도록 **main.ts** 설정

```ts
import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { NestExpressApplication } from '@nestjs/platform-express';
import { join } from 'path';

async function bootstrap() {
  // 인스턴스 생성
  const app = await NestFactory.create<NestExpressApplication>(AppModule);
  // 파일 경로 지정
  app.useStaticAssets(join(__dirname, '..', 'static'));
  await app.listen(3000);
}
bootstrap();
```

## 게이트웨이

웹소캣을 사용한 통신을 받아주는 클래스다. 프로토콜이 HTTP라면 컨트롤러부터 요청을 받고 프로토콜이 ws라면 게이트웨이로부터 요청을 받는다.

**src/app.gateway.ts**를 작성한다.

```ts
import {
    WebSocketGateway,
    WebSocketServer,
    SubscribeMessage,
} from '@nestjs/websockets';
import { Server, Socket } from 'socket.io';

@WebSocketGateway()
export class ChatGateway {
    @WebSocketServer() server: Server;

    // message 이벤트 구독
    @SubscribeMessage('message')
    handleMessage(socket: Socket, data: any): void {
        // 접속한 클라이언트들에 메시지 전송
        this.server.emit('message', `client-${socket.id.substring(0, 4)} : ${data}`);
    }
}
```

`@SubscribeMessage`는 클라이언트에서 해당되는 이벤트로 데이터가 전송되는 경우에 데이터를 받고 처리한다.

**app.module.ts**에 등록한다.

```ts
import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { ChatGateway } from './app.gateway';

@Module({
  imports: [],
  controllers: [AppController],
  providers: [AppService, ChatGateway],
})
export class AppModule {}
```

## html 작성

**static/index.html**과 **static/script.js** 작성

```html
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <title>체팅 예제</title>
    </head>
    <body>
        <h2>간단한 체팅 웹</h2>
        <div id="chat"></div>

        <input type="text" id="message" placeholder="메시지 입력">
        <button onclick="sendMessage()">전송</button>
    </body>

    <script src="https://code.jquery.com/jquery-3.6.1.slim.js"></script>
    <script src="http://localhost:3000/socket.io/socket.io.js"></script>
    <script src="script.js"></script>
</html>
```

```js
const socket = io('http://localhost:3000');

function sendMessage() {
    const message = $('#message').val();
    socket.emit('message', message);
}

// 접속 이벤트 처리
socket.on('connect', () => {
    console.log('서버 연결됨');
});

socket.on('message', (message) => {
    $('#chat').append(`<div>${message}</div>`);
});
```

# 체팅방 기능 구현

`socket.io`는 체팅방을 만드는 room 기능을 제공한다. 체팅방끼리 메시지 통신이 필요하기 때문에 네임스페이스도 사용한다. 네임스페이스는 하나의 연결을 로직을 통해 나누어 사용할 수 있게 만든다.

**app.gateway.ts**

```ts
@WebSocketGateway({ namespace: 'chat' })
export class ChatGateway {
// ...
}
```

그 후 **script.js**의 `socket` 변수 대입값을 수정한다.

```js
const socket = io('http://localhost:3000/chat');
```

## 닉네임 추가

체팅방 기능 구현 전, 사용자 구분을 위해 닉네임 기능 먼저 구현한다.

```ts
const socket = io('http://localhost:3000/chat');
const nickname = prompt('닉네임 입력');

function sendMessage() {
    const message = $('#message').val();
    $('#chat').append(`<div>나: ${message}</div>`);
    socket.emit('message', {message, nickname});
}
// ...
```

이제 게이트웨이를 수정한다.

**app.gateway.ts**

```ts
export class ChatGateway {
    // ...
    handleMessage(socket: Socket, data: any): void {
        // 메시지와 닉네임 분리
        const { message, nickname } = data;
        // 닉네임을 포함하여 메시지 전송
        socket.broadcast.emit('message', `${nickname}: ${message}`);
    }
}
```

`server.emit()`은 나를 포함한 모든 클라이언트에게 전송하고 `server.broadcast.emit()`은 전송 요청 클라이언트는 제외하고 전송한다.

## 체팅방 생성

html 파일을 추가한다.

**index.html**

```html
<!-- ... -->
<div>
    <h2>체팅방 목록</h2>
    <ul id="rooms"></ul>
</div>

<input type="text" id="message" placeholder="메시지 입력">
<button onclick="sendMessage()">전송</button>
<button onclick="createRoom()">방 생성</button>
<!-- ... -->
```

그 후 **script.js**에 방 생성 관련 이벤트를 추가한다.

```js
// ...
const roomSocket = io('http://localhost:3000/room');
let currentRoom = '';

// ...
function createRoom() {
    const room = prompt('생성할 방의 이름 입력');
    roomSocket.emit('createRoom', { room, nickname });
}

roomSocket.on('rooms', (data) => {
    console.log(data);
    $('#rooms').empty();
    // 서버에서 전달한 데이터를 html 형식과 함께 추가
    data.forEach((room) => {
        $('#rooms').append(`<li>${room} <button onclick="joinRoom('${room}')">입장</button>`);
    });
});
// ...
```

게이트웨이를 추가하고 모듈에 등록한다.

**app.gatewat.ts**

```ts
import {
    // ...
    MessageBody,
} from '@nestjs/websockets';
// ...
@WebSocketGateway({ namespace: 'room' })
export class RoomGateway {
    rooms = [];

    @WebSocketServer() server: Server;

    @SubscribeMessage('createRoom')
    handleMessage(@MessageBody() data) {
        const { nickname, room } = data;
        this.rooms.push(room);
        this.server.emit('rooms', this.rooms);
    }
}
```

**app.module.ts**

```ts
import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { ChatGateway, RoomGateway } from './app.gateway';

@Module({
  imports: [],
  controllers: [AppController],
  providers: [AppService, ChatGateway, RoomGateway],
})
export class AppModule {}
```

## 공지 사항과 입장 구현

**index.html**

```html
<!-- ... -->
<button onclick="createRoom()">방 생성</button>

<div>
    <h2>공지 사항</h2>
    <div id="notice"></div>
</div>
<!-- ... -->
```

**script.js**

```js
// ...
function joinRoom(room) {
    // 서버 측 joinRoom 이벤트 발생
    roomSocket.emit('joinRoom', { room, nickname, toLeaveRoom: currentRoom });
    // 현재 들어 있는 방의 값을 변경
    currentRoom = room;
}

// notice 이벤트 처리
socket.on('notice', (data) => {
    $('#notice').append(`<div>${data.message}</div>`);
});
// ...
```

서버에서 `chat` 네임스페이스의 `notice` 이벤트로 요청이 오는 경우 해당 데이터를 처리한다. 

**app.gateway.ts**

```ts
// ...
@WebSocketGateway({ namespace: 'room' })
export class RoomGateway {
    constructor(private readonly chatGateway: ChatGateway) {}
    // ...

    @WebSocketGateway({ namespace: 'room' })
    export class RoomGateway {
        @SubscribeMessage('createRoom')
        handleMessage(@MessageBody() data) {
            const { nickname, room } = data;
            this.chatGateway.server.emit('notice', {
                message: `${nickname}이 ${room} 생성`,
            });
            // ...
        }

        @SubscribeMessage('joinRoom')
        handleJoinRoom(socket: Socket, data) {
            const { nickname, room, toLeaveRoom } = data;
            socket.leave(toLeaveRoom);
            this.chatGateway.server.emit('notice', {
                message: `${nickname}이 ${room}에 입장`,
            });
            socket.join(room);
        }
        // ...
    }
}
```

이미 작성된 `ChatGateway`를 `RoomGateway`에 의존성 주입하여 사용하였다. 해당 인스턴스는 공지를 처리하는 데 사용한다. `join`과 `leave`로 방을 나가고 들어온다.

## 체팅방끼리의 대화 구현

**script.js**

```js
// ...
function sendMessage() {
    if(currentRoom === '') {
        alert('체팅방 선택');
        return;
    }
    const message = $('#message').val();
    const data = { message, nickname, room: currentRoom };
    $('#chat').append(`<div>나: ${message}</div>`);
    roomSocket.emit('message', data);
    return false;
}
// ...
roomSocket.on('message', (data) => {
    console.log(data);
    $('#chat').append(`<div>${data.message}</div>`);
});
// ...
function joinRoom(room) {
    roomSocket.emit('joinRoom', { room, nickname, toLeaveRoom: currentRoom });
    // 체팅방 이동 시 기존 메시지 삭제
    $('#chat').html('');
    currentRoom = room;
}
```

**app.gateway.ts**

```ts
@WebSocketGateway({ namespace: 'room' })
export class RoomGateway {
    // ...
    @SubscribeMessage('message')
    handleMessageToRoom(socket: Socket, data) {
        const { nickname, room, message } = data;
        console.log(data);
        socket.broadcast.to(room).emit('message', {
            message: `${nickname}: ${message}`,
        });
    }
}
```