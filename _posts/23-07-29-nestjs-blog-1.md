---
title:  "[Node.js] 07 - NestJS로 블로그 만들기 - 1"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-29
---

NestJS로 블로그를 만들어볼 것이다. NestJS를 이용한 웹 서버 제작은 크게 API 작성 -> 의존성 주입 -> DB연동의 3단계로 나눌 수 있다.

# 프로젝트 생성과 설정 

nest-cli를 사용하면 한 번에 프로젝트에 필요한 아키텍처를 생성할 수 있다. 터미널에 다음 명령어를 실행한다.

```
npm install -g @nestjs/cli
```

`-g`는 글로벌 옵션으로 어디서든지 사용할 수 있도록 설치하는 옵션이다.

이제 블로그를 만들 디렉터리로 이동하여 터미널에서 다음 명령어를 실행한다.

```
nest new testblog
```

testblog라는 이름의 프로젝트가 만들어진다.

프로젝트에는 다음 파일과 폴더가 생성된다.

```
.eslintrc.js
//git에서 관리하지 않는 파일과 디렉터리 정의
.gitignore 
//코드 포매팅 관련 설정 파일
.prettierrc
dist
//모노레포 등의 기능 사용 시 설정
nest-cli.json
node_modules
package-lock.json
package.json
//실행 가능한 명령어를 명시
README.md
//컨트롤러 & 테스트 코드, 모듈, 서비스, 서비스 메인 파일 디렉터리
src
test
tsconfig.build.json
//타입스크립트 설정
tsconfig.json
```

모노레포 기능이란 한 프로젝트 안에 여러 프로젝트를 포함하는 것을 말한다.

개발시 주로 실행하는 다음 3개의 명령어가 README.md에 명시되어 있다.

```
npm run start
//파일 변경에 대한 실시간 적용
npm run start:dev
//프로덕션 환경에서 실행
npm run start:prod
```

src 디렉터리의 `main.ts`와 `app.module.ts`를 제외한 모든 파일을 삭제하고 `blog.---.ts`이런 식의 파일로 만들어주고 진행하였다.

# 컨트롤러 만들기

사용자가 보낸 HTTP 요청을 어떤 코드에서 처리할 지 정하는 역할을 한다. 요청에는 헤더, URL 매개변수, 쿼리, 바디 등의 정보가 있다.

아래 파일들을 수정 및 작성하였다.

**app.module.ts**

```ts
import { Module } from '@nestjs/common';
import { BlogController } from './blog.controller';
import { BlogService } from './blog.service';

@Module({
  imports: [],
  controllers: [BlogController],
  providers: [BlogService],
})
export class AppModule {}
```

**blog.controller.ts**

```ts
export class BlogController {}
```

**blog.service.ts**

```ts
export class BlogService {}
```

# API 작성

글 목록 가져오기, 글 작성, 게시글 하나 가져오기, 글 삭제, 글 수정의 기능을 만들 것이다.

**blog.controller.ts**

```ts
import { Controller, Param, Body, Delete, Get, Post, Put } from '@nestjs/common';

@Controller('blog')
export class BlogController {
    @Get()
    getAllPosts() {
        console.log('모든 게시글 가져오기');
    }

    //POST 요청 처리
    @Post()
    //HTTP 요청의 body 내용을 post에 할당
    createPost(@Body() post: any) {
        console.log('게시글 작성');
        console.log(post);
    }

    //GET의 URL 매개변수에 id가 있는 요청 처리
    @Get('/:id')
    getPost(@Param('id') id: string) {
        console.log(`[id: ${id}]게시글 하나 가져오기`);
    }

    //DELETE 방식에 URL 매개변수로 id가 있는 요청 처리
    @Delete()
    deletePost() {
        console.log('게시글 삭제');
    }

    //PUT 방식에 URL 매개변수로 전달된 id가 있는 요청 처리
    @Put('/:id')
    updatePost(@Param('id') id, @Body() post: any) {
        console.log(`[${id}] 게시글 업데이트`);
        console.log(post);
    }
}
```

필요한 데코레이터들을 `@nestjs/common`으로 불러와 사용한다. `@Controller('blog)`는 `서버주소/blog` 이하의 요청을 처리한다는 것이다.

`@Body`와 `@Param`은 매개변수에 붙이는 데코레이터다. 각각 함수의 `body`로 오는 값과 `URL param`의 값을 매개변수에 할당한다.

# 메모리에 데이터를 저장하는 API

컨트롤러는 HTTP 요청을 특정 함수가 실행하도록 하는 것이다. 따라서 실제 로직은 서비스에 구현한다.

**blog.model.ts**

```ts
//게시글의 타입을 인터페이스로 정의
export interface PostDto {
    id: string;
    title: string;
    content: string;
    name: string;
    createDt: Date;
    updateDt: Date;
}
```

**blog.service.ts**

```ts
import { PostDto } from './blog.model';

export class BlogService {
    posts = [];

    getAllPosts() {
        return this.posts;
    }

    createPost(PostDto: PostDto) {
        const id = this.posts.length + 1;
        this.posts.push({ id: id.toString(), ...PostDto, createDt: new Date() });
    }

    getPost(id) {
        const post = this.posts.find((post) => {
            return post.id === id;
        });
        console.log(post);
        return post;
    }

    delete(id) {
        const filteredPosts = this.posts.filter((post) => post.id !== id);
        this.posts = [...filteredPosts];
    }

    updatePost(id, postDto: PostDto) {
        let updateIndex = this.posts.findIndex((post) => post.id === id);
        const updatePost = { id, ...postDto, updateDt: new Date() };
        this.posts[updateIndex] = updatePost;
        return updatePost;
    }
}
```

**blog.controller.ts**

```ts
import { Controller, Param, Body, Delete, Get, Post, Put } from '@nestjs/common';
import { BlogService } from './blog.service';

@Controller('blog')
export class BlogController {
    blogService: BlogService;
    //생성자
    constructor() {
        this.blogService = new BlogService();
    }

    @Get()
    getAllPosts() {
        console.log('모든 게시글 가져오기');
        return this.blogService.getAllPosts();
    }

    //POST 요청 처리
    @Post()
    //HTTP 요청의 body 내용을 post에 할당
    createPost(@Body() postDto) {
        console.log('게시글 작성');
        this.blogService.createPost(postDto);
        return 'success';
    }

    //GET의 URL 매개변수에 id가 있는 요청 처리
    @Get('/:id')
    getPost(@Param('id') id: string) {
        console.log(`[id: ${id}]게시글 하나 가져오기`);
        return this.blogService.getPost(id);
    }

    //DELETE 방식에 URL 매개변수로 id가 있는 요청 처리
    @Delete('/:id')
    deletePost(@Param('id') id: string) {
        console.log('게시글 삭제');
        this.blogService.delete(id);
        return 'success';
    }

    //PUT 방식에 URL 매개변수로 전달된 id가 있는 요청 처리
    @Put('/:id')
    updatePost(@Param('id') id: string, @Body() postDto) {
        console.log(`게시글 업데이트`, id, postDto);
        return this.blogService.updatePost(id, postDto);
    }
}
```

모든 블로그 조작은 `blogService` 객체를 통해서 진행한다. 

이제 `npm run start:dev`로 서버를 실행하고 REST를 이용하여 테스트해본다.

```
@server = http://localhost:3000

# 게시글 조회
GET {{server}}/blog

### 게시글 생성
POST {{server}}/blog
Content-Type: application/json

{
    "title": "Hello",
    "content": "Nice to meet you",
    "name": "Test"
}

### 특정 게시글 조회
GET {{server}}/blog/<ID값>

###게시글 삭제
DELETE {{server}}/blog/<ID값>

###게시글 수정
PUT {{server}}/blog/<ID값>
Content-Type: application/json

{
    "title": "타이틀 수정",
    "content": "본문 수정",
    "name": "json"
}
```

`<ID값>`은 가변적이다. 현재는 1, 2등의 숫자이지만 나중에 DB와 연동하면 해당 DB에 저장된 ID값을 사용해야 한다.