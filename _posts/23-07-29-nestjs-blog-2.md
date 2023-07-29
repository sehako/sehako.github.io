---
title:  "[Node.js] 07 - NestJS로 블로그 만들기 - 2"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-29
---

# 파일에 정보를 저장하는 API

앞서 작성된 코드는 게시글이 서버에 메모리에 적재된다. 실제 웹 서버를 운영한다면 파일이나 DB에 저장되도록 해야한다.

레포지토리 파일을 만들고 그곳에 서비스에 구현해두었던 부분을 모두 구현한 다음 서비스는 단순하게 레포지토리를 사용하는 클래스로 만들 것이다.

**blog.repository.ts**

```ts
import { readFile, writeFile } from 'fs/promises';
import { PostDto } from './blog.model';

//블로그 리포지토리 인터페이스 정의
export interface BlogRepository {
    getAllPost(): Promise<PostDto[]>;
    createPost(postDto: PostDto);
    getPost(id: string): Promise<PostDto>;
    deletePost(id: string);
    updatePost(id: string, postDto: PostDto);
}

//구현 클래스, 파일 읽고 쓰기
export class BlogFileRepository implements BlogRepository {
    FILE_NAME = './src/blog.data.json';

    //파일 읽어 모든 게시글 불러오기
    async getAllPost(): Promise<PostDto[]> {
        const datas = await readFile(this.FILE_NAME, 'utf8');
        const posts = JSON.parse(datas);
        return posts;
    }
    //게시글 쓰기
    async createPost(postDto: PostDto) {
        const posts = await this.getAllPost();
        const id = posts.length + 1;
        const createPost = { id: id.toString(), ...postDto, createDt: new Date() };
        posts.push(createPost);
        await writeFile(this.FILE_NAME, JSON.stringify(posts));
    }
    //게시글 하나 가져오기
    async getPost(id: string): Promise<PostDto> {
        const posts = await this.getAllPost();
        const result = posts.find((post) => post.id === id);
        return result;
    }
    //게시글 하나 삭제
    async deletePost(id: string) {
        const posts = await this.getAllPost();
        const filteredPosts = posts.filter((post) => post.id !== id);
        await writeFile(this.FILE_NAME, JSON.stringify(filteredPosts));
    }
    //게시글 하나 수정
    async updatePost(id: string, postDto: PostDto) {
        const posts = await this.getAllPost();
        const index = posts.findIndex((post) => post.id === id);
        const updatePost = { id, ...postDto, updateDt: new Date() };
        posts[index] = updatePost;
        await writeFile(this.FILE_NAME, JSON.stringify(posts));
    }
}
```

읽고 쓸 파일을 **src/blog.data.json**으로 만들고 초기값으로 `[]`을 설정한다.

**blog.service.ts**

```ts
import { PostDto } from './blog.model';
//레포지토리 클래스 임포트
import { BlogFileRepository, BlogRepository } from './blog.repository';

export class BlogService {

    blogRepository: BlogRepository;

    constructor() {
        this.blogRepository = new BlogFileRepository;
    }

    async getAllPosts() {
        return await this.blogRepository.getAllPost();
    }

    async createPost(PostDto: PostDto) {
        return await this.blogRepository.createPost(PostDto);
    }

    async getPost(id): Promise<PostDto> {
        return await this.blogRepository.getPost(id);
    }

    delete(id) {
        return this.blogRepository.deletePost(id);
    }

    updatePost(id, postDto: PostDto) {
        this.blogRepository.updatePost(id, postDto);
    }
}
```

**blog.controller.ts**

```ts
//비동기를 지원하는 메소드로 시그니처 변경
@Get('/:id')
async getPost(@Param('id') id: string) {
    console.log(`[id: ${id}]게시글 하나 가져오기`);
    const post = await this.blogService.getPost(id);
    console.log(post)
    return post
}
```

서비스에서 사용하는 `getPost()` 메소드가 비동기로 변경되어 컨트롤러도 함께 변경해야 한다.

# 의존성 주입

현재까지는 생성자에서 각 단계에 필요한 객체를 만들었다. 클래스의 숫자가 많아지면 이 또한 문제가 된다. 이를 해결하고자 제어의 역전 원칙을 사용하여 객체 생성을 프레임워크에 맡기는 해결책이 등장하였다.

이 패턴이 바로 의조ㄴ성 주입이다. 객체를 직접 생성하지 않고 프레임워크가 생성한 컨테이너가 의존성을 관리한다. NestJS에서 의존성 주입은 `@Injectable()` 데코레이터를 사용하기만 하면 된다.

**blog.repository.ts**

```ts
import { Injectable } from '@nestjs/common/decorators';
//생략...
@Injectable()
export class BlogFileRepository implements BlogRepository
```

**blog.service.ts**

```ts
import { Injectable } from '@nestjs/common/decorators';

@Injectable()
export class BlogService {
    //생성자를 통한 의존성 주입
    constructor(private blogRepository: BlogFileRepository) {}
    // 생략...
```

생성자에 매개변수로 설정된 타입이 프로바이더로 설정된 타입 중 하나면, NestJS에서 자동으로 필요한 객체를 주입한다. `BlogRepository`는 인터페이스이므로 클래스를 생성할 수 없다. 따라서 의존성 주입을 할 때는 실제로 사용할 클래스를 타입으로 준다.

위 코드와 같은 기능의 다른 문법도 참고한다.

```ts
private blogRepository: BlogFileRepository;

constructor(blogRepository: BlogFileRepository) {
    //클래스 맴버 변수에 주입받은 blogRepository 할당
    this.blogRepository = blogRepository;
}
```

**blog.controller.ts**

```ts
export class BlogController {
constructor(private blogService: BlogService) {}
//생략...
}
```

이 상태로 실행하면 NestJS에서 의존성 주입을 하는 타입인지 모르기에 에러가 발생한다.

**app.module.ts**

```ts
import { BlogFileRepository } from './blog.repository';

@Module({
    //생략...
    // 프로바이더 설정
    providers: [BlogService, BlogFileRepository],
})
```

# DB 연동

NestJS에서 몽고디비를 연동하려면 TypeORM을 사용해 `connector`를 몽고디비로 사용하거나, Mongoose를 사용하는 방법이 있다.

의존성 설치 -> 스키마 제작 -> DB를 사용하는 레포지토리 추가 -> 서비스 코드 변경 -> 모듈에 DB 설정 & 프로바이더 설정 순서로 이루어진다.

의존성 패키지로 `@nestjs/mongoose`와 `mongoose`를 설치한다.

```
npm install @nestjs/mongoose mongoose
```

## 스키마 만들기

RDB의 테이블과 비슷한 역할을 한다. `@Schema` 데코레이터를 이용하여 만든다.

**blog.schema.ts**

```ts
import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document } from 'mongoose';

export type BlogDocument = Blog & Document;

@Schema() 
export class Blog {
    //스키마의 프로퍼티
    @Prop()
    id: string;

    @Prop()
    title: string;

    @Prop()
    content: string;

    @Prop()
    name: string;

    @Prop()
    createDt: Date;

    @Prop()
    updateDt: Date;
}

//스키마 생성
export const BlogSchema = SchemaFactory.createForClass(Blog);
```

`type`에서 두 개의 타입을 `&`로 연결하여 교차타입을 만들었다. `AND`의 의미로 두 프로퍼티를 가지고 있어야 한다는 뜻이다. 반대로 `OR`의 의미를 지닌 `|`도 있다.

**blog.repository.ts**

```ts
import { InjectModel } from '@nestjs/mongoose/dist/common';
import { Model } from 'mongoose';
import { Blog, BlogDocument } from './Blog.schema';

//블로그 리포지토리 인터페이스 정의
export interface BlogRepository {
    //생략...
}

@Injectable()
//몽고디비용 리포지토리
export class BlogMongoRepository implements BlogRepository {
    //Model<BlogDocument> 타입인 blogModel 주입
    constructor(@InjectModel(Blog.name) private blogModel: Model<BlogDocument>) {}

    //모든 게시글 가져오기
    async getAllPost(): Promise<PostDto[]> {
        return await this.blogModel.find().exec();
    }
    //게시글 작성
    async createPost(postDto: PostDto) {
        const createPost = {
            ...postDto,
            createDt: new Date(),
            updateDt: new Date(),
        };
        this.blogModel.create(createPost);
    }

    async getPost(id: string): Promise<PostDto> {
        return await this.blogModel.findById(id);
    }
    //게시글 하나 삭제하기
    async deletePost(id: string) {
        await this.blogModel.findByIdAndDelete(id);
    }

    //게시글 업데이트

    async updatePost(id: string, postDto: PostDto) {
        const updatePost = { id, ...postDto, updateDt: new Date() };
        await this.blogModel.findByIdAndUpdate(id, updatePost);
    }
}
```

서비스에서 인터페이스 타입을 사용한다면 큰 변경 없이 사용할 수 있는 장점이 있다.

**blog.service.ts**

```ts
import { BlogMongoRepository } from './blog.repository';

constructor(private blogRepository: BlogMongoRepository) {}
```

서비스 코드는 레포지토리의 의존성만 변경하면 된다.

**app.module.ts**

```ts
import { Module } from '@nestjs/common';
import { BlogController } from './blog.controller';
import { BlogService } from './blog.service';
import { BlogMongoRepository } from './blog.repository';
import { MongooseModule } from '@nestjs/mongoose/dist/mongoose.module';
import { Blog, BlogSchema } from './blog.schema';

@Module({
  imports: [
    MongooseModule.forRoot(
      'mongodb+srv://ID:password@clusterInfo/?retryWrites=true&w=majority',
    ),
    MongooseModule.forFeature([{ name: Blog.name, schema: BlogSchema }]),
  ],
  controllers: [BlogController],
  providers: [BlogService, BlogMongoRepository],
})
export class AppModule {}
```

책에서는 

```ts
import { BlogFileRepository, BlogMongoRepository } from './blog.repository';
```

이렇게 불러왔는데 코드 수정 과정에서 `BlogFileRepository`이건 없어지지 않았나 싶어 지웠고, 문제없이 잘 작동되었다.

또한 해당 주소 설정 마지막 부분에서 `/blog`를 선언했는데, 이렇게 하니 오류가 발생하여 그냥 지웠고 문제없이 잘 작동되었다.

서버를 실행하고 REST로 테스트하면 다음 json 값이 만들어진다. 앞서 id를 통해 검색, 삭제, 수정을 진행하였으므로 DB에 저장된 해당 id 값으로 테스트하면 된다.
```
[
  {
    "_id": "64c339489e05b5379df866af",
    "title": "Hello",
    "content": "Nice to meet you",
    "name": "Test",
    "createDt": "2023-07-28T03:43:04.104Z",
    "updateDt": "2023-07-28T03:43:04.104Z",
    "__v": 0
  }
]
```