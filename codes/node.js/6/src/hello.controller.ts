import { Controller, Get } from "@nestjs/common";

//데코레이터
@Controller()
export class HelloController {
    //GET 요청 처리
    @Get()
    hello() {
        return "Hello World!";
    }
}