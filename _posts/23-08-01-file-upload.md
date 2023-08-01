---
title:  "[Node.js] 10 - 파일 업로드"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-08-01
---

# 프로젝트 생성

```
nest new project-name
cd project-name
npm install --save @types/multer
```

# 파일 업로드 API

**app.controller.ts** 작성

```ts
import {
  Controller,
  Get,
  Post,
  UploadedFile,
  UseInterceptors,
} from '@nestjs/common';
import { FileInterceptor } from '@nestjs/platform-express';
import { AppService } from './app.service';

@Controller()
export class AppController {
  constructor(private readonly appService: AppService) {}

  @Get()
  getHello(): string {
    return this.appService.getHello();
  }

  // 업로드 핸들러 추가
  @Post('upload')
  @UseInterceptors(FileInterceptor('file'))
  fileUpload(@UploadedFile() file: Express.Multer.File) {
    console.log(file.buffer.toString('utf-8'));
    return `file upload`;
  }
}
```

파일 업로드는 `POST`로 진행한다. `Interceptor`는 요청과 응답 사이 로직을 추가하는 미들웨어다. `FileInterceptor()`는 클라이언트의 요청에 따라 파일명이 `file`인 파일이 있는지 확인한다.

여러 파일을 업로드 할 때는 `@UploadedFiles()` 데코레이터를 사용한다. 각 파일의 타입을 `Express.Multer.File` 타입이다.

여담으로 `console.log(file.buffer.toString('utf-8'));` 이 부분은 책과 다르게 에러가 발생한다. 따라서 현재 단계에서는 아래 작성된 HTTP 테스트로 확인이 불가능했다.

```
### 파일 업로드
POST http://localhost:3000/upload
Content-Type: multipart/form-data; boundary=test-file-upload

--test-file-upload
Content-Disposition: form-data; name="file"; filename="test.txt"

텍스트 파일 내용 입력
--test-file-upload--
```

`multipart/form-data` 타입은 파일과 각종 데이터를 동시에 보낼 때 사용한다. `boundary`는 각 매개변수를 구분하는 역할을 하며 7비트의 아스키만 허용한다.

`Content-Disposition`은 전송하려는 매개변수가 어떤 데이터인지 정의하는 공간이다.

## 업로드한 파일 저장

`FileInterceptor`의 두 번째 인수는 `multer`에서 제공하는 저장 옵션을 사용할 수 있다.

**multer.options.ts**을 생성하여 옵션 파일을 작성한다.

```ts
import { randomUUID } from "crypto";
import { diskStorage } from "multer";
import { extname, join } from "path";

export const multerOption = {
  // 디스크 스토리지 사용
  storage: diskStorage({
    // 파일 저장 경로 설정
      destination: join(__dirname, '..', 'uploads'),
      // 파일명 설정
      filename: (req, file, cb) => {
          cb(null, randomUUID() + extname(file.originalname));
      },
  }),
};
```

프로젝트 루트 디렉토리의 **uploads**에 저장되도록 설정하였다.

**app.controller.ts** 수정

```ts
import {
  Controller,
  Get,
  Post,
  UploadedFile,
  UseInterceptors,
} from '@nestjs/common';
import { FileInterceptor } from '@nestjs/platform-express';
import { AppService } from './app.service';
import { multerOption } from './multer.options';

@Controller()
export class AppController {
  constructor(private readonly appService: AppService) {}

  @Get()
  getHello(): string {
    return this.appService.getHello();
  }

  @Post('upload')
  @UseInterceptors(FileInterceptor('file', multerOption))
  fileUpload(@UploadedFile() file: Express.Multer.File) {
    // 파일 정보 출력
    console.log(file);
    return `file upload`;
  }
}
```

이 이후부터는 테스트 또한 가능하다. HTTP 테스트를 진행하면 텍스트 파일이 프로젝트 루트 디렉터리의 **uploads**에 저장된다.

# 정적 파일 서비스

텍스트, 이미지, 동영상 같은 파일은 한 번 저장되면 변경이 없으므로 정적 파일이라고 부른다. 실습을 위해 해당 패키지가 필요하다.

```
npm install @nestjs/serve-static
```

**app.module.ts**에 설정을 추가한다.

```ts
import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { ServeStaticModule } from '@nestjs/serve-static';
import { join } from 'path';

@Module({
  imports: [
    ServeStaticModule.forRoot({
      // 실제 파일이 있는 디렉터리 지정
      rootPath: join(__dirname, '..', 'uploads'),
      // url뒤에 붙을 경로 지정
      serveRoot: '/uploads',
    }),
  ],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
```

`serveRoot` 옵션이 없으면 `서버이름/파일명`으로 접근이 가능하다.

HTTP 테스트 파일을 작성한다.

```
### 사진 업로드
POST http://localhost:3000/upload
Content-Type: multipart/form-data; boundary=image-file-upload

--image-file-upload
Content-Disposition: form-data; name="file"; filename="cat.jpg"
Content-Type: image/jpeg

< cat.jpg
--image-file-upload--
```

`<` 옵션으로 파일 경로를 지정할 수 있다. 이 경우 현재 HTTP 테스트 파일 디렉터리에 존재하는 **cat.jpg** 파일을 업로드한다.

## HTML 폼으로 업데이트

실제 사용자 서비스 환경에서는 웹 브라우저를 통해 업데이트를 해야한다. 이를 위해 HTML 폼을 만들어 업로드 기능을 제공한다. 

**main.ts**를 수정한다.

```ts
import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { join } from 'path';
import { NestExpressApplication } from '@nestjs/platform-express';

async function bootstrap() {
  const app = await NestFactory.create<NestExpressApplication>(AppModule);
  app.useStaticAssets(join(__dirname, '..', 'static'));
  await app.listen(3000);
}
bootstrap();
```

`useStaticAssets()`에서 경로만 지정해주면 정적 파일 서비스가 가능하다. 해당 미들웨어는 익스프레스에 있기 때문에 `NestExpressApplication` 타입으로 인스턴스를 생성했다. 익스프레스의 미들웨어를 사용하려면 인스턴스를 생성할 때 `NestExpressApplication`을 선언해 주어야 한다.

이제 프로젝트 루트 디렉터리에 **static/**을 생성하고 그곳에 HTML 파일을 작성한다.

**form.html**

```html
<!DOCTYPE html>
<html>
    <body>
        <form action="upload" method="post" enctype="multipart/form-data">
            <input type="file" name="file">
            <input type="submit" value="upload">
        </form>
    </body>
</html>
```

그 다음 **app.controller.ts**의 업로드 핸들러의 `return`문을 다음과 같이 수정한다.

```ts
return `${file.originalname} File Uploaded check http://localhost:3000/uploads/${file.filename}`;
```

`서버 이름/form.html`에 접속하면 파일을 업로드할 수 있고, 업로드하면 위 반환 문구에 따라서 경로가 반환된다. 해당 경로를 주소에 붙여넣으면 파일 내용이 브라우저에 표시된다.