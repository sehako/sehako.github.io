console.log("1");
//I/O  처리가 필요한 코드이므로 콜스택에서 이벤트 루프로 넘김
//이벤트 루프는 OS또는 스레드 워커에 처리
//처리에 대한 반환값을 이벤트 루프가 받아 콜스택에 넘김
setTimeout( () => console.log(2), 1000);
console.log("3");