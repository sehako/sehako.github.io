import { NestFactory } from "@nestjs/core";
import { HelloModule } from "./hello.module";

//NestJS 시작 함수
async function bootstrap() {
    //객체 생성
    const app = await NestFactory.create(HelloModule);

    await app.listen(3000, () => { console.log("Server Start"); });
}

bootstrap();