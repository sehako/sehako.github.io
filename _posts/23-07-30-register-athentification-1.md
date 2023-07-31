---
title:  "[Node.js] 09 - 회원 가입과 인증 - 1"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-30
---

# 프로젝트 생성

기존의 `AppModule`을 포함하여 인증 모듈과 유저 모듈을 추가하여 프로젝트를 구성한다. 우선 `nest new`로 프로젝트 생성한다.

```
nest new project-name
```

그 후 다음 명령어로 유저 모듈을 추가한다

```
nest g module user
nest g controller user --no-spec
nest g service user --no-spec
```

프로젝트 디렉토리에 **src/user**가 생성된다.

# DB 설정

SQlite DB를 이용할 것이기 때문에 필요한 패키지를 설치한다.

```
npm install sqlite3 typeorm @nestjs/typeorm
```

`typeorm`은 객체 관계형 매핑 기술(ORM)을 지원하는 라이브러리로, 직접 SQL을 작성하지 않고도 DB와 통신이 가능하게 해준다.

**app.module.ts**에서 DB 설정을 한다.

```ts
import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { UserModule } from './user/user.module';
import { TypeOrmModule } from '@nestjs/typeorm';

@Module({
  imports: [UserModule,
  TypeOrmModule.forRoot({
    type: 'sqlite',
    database: 'nest-auth-test.sqlite',
    entities: [],
    // 개발용으로만 사용해야 함
    synchronize: true,
    logging: true,
  })],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
```

`synchronize` 옵션은 프로덕션 서버에서 사용하면 서버 실행 시 의도치 않은 DB 스키마 변경 위험이 있다.

## 엔티티 만들기

DB 테이블과 1:1로 매칭되는 객체다. 

**user.entity.ts**

```ts
import { Column, Entity, PrimaryGeneratedColumn } from 'typeorm';

@Entity()
export class User {
    @PrimaryGeneratedColumn()
    // pk 값
    id?: number;

    // 유니크 선언
    @Column({ unique: true })
    email: string;

    @Column()
    password: string;

    @Column()
    username: string;

    // 작동 안되는 코드... 1로 고정됨
    @Column({ default:true })
    createDt: Date = new Date();
}
```

pk값은 기본키면서 자동증가하는 값이 된다. pk값 선언에 붙은 `?`는 객체 생성 시 필수값이 아니란 의미다.

위 코드 중 날짜 관련 코드는 책과 다르게 작동이 안된다. 실제 서버를 실행해서 사용자를 생성해보면 1로 고정되어 입력되는 값이 되어버린다...

생성한 엔티티를 유저 서비스에 주입하고 CRUD를 구성한다.

**user.service.ts**

```ts
import { Injectable } from '@nestjs/common';
// 레포지토리 주입 데코레이터
import { InjectRepository } from '@nestjs/typeorm/dist';
import { User } from './user.entity';
import { Repository } from 'typeorm';

@Injectable()
export class UserService {
    // 레포지토리 주입
    constructor(
        @InjectRepository(User) private userRepository: Repository<User>,
    ) {}

    // 유저 생성 
    createUser(user) : Promise<User> {
        return this.userRepository.save(user)
    }

    // 한 명의 유저 정보 찾기
    async getUser(email: string) {
        const result = await this.userRepository.findOne({
            where: { email },
        });
        return result;
    }

    // 유저 정보 업데이트
    async updateUser(email, _user) {
        const user = await this.getUser(email);
        console.log(_user);
        user.username = _user.username;
        user.password = _user.password;
        console.log(user);
        this.userRepository.save(user);
    }

    // 유저 정보 삭제
    deleteUser(email: any) {
        return this.userRepository.delete({ email });
    }
}
```

유저 생성은 `Promise<User>` 타입을 반환한다. `await`로 실행하면 `User`를 반환 받을 수 있다. 

`save()` 메소드는 생성과 업데이트 두 기능에서 모두 사용된다.

`deleteUser`에서 `{ email }`은 `{ email: email }`을 줄여 쓴 것이다.

## 컨트롤러 만들기

**user.controller.ts**

```ts
import { Body, Controller, Get, Post,
    Param, Put, Delete } from '@nestjs/common';
import { User } from './user.entity'
import { UserService } from './user.service';

@Controller('user')
export class UserController {
    constructor(private userService: UserService) {}

    @Post('/create')
    createUser(@Body() user: User) {
        return this.userService.createUser(user);
    }

    @Get('/getUser/:email')
    async getUser(@Param('email') email: string) {
        const user = await this.userService.getUser(email);
        console.log(user);
        return user;
    }

    @Put('/update/:email')
    updateUser(@Param('email') email: string, @Body() user: User) {
        console.log(user);
        return this.userService.updateUser(email, user);
    }

    @Delete('/delete/:email')
    deleteUser(@Param('email') email: string) {
        return this.userService.deleteUser(email);
    }
}
```

이제 레포지토리를 **user.module.ts**에 등록한다.

```ts
import { Module } from '@nestjs/common';
import { UserController } from './user.controller';
import { UserService } from './user.service';
import { TypeOrmModule } from '@nestjs/typeorm';
import { User } from './user.entity';

@Module({
  imports: [TypeOrmModule.forFeature([User])],
  controllers: [UserController],
  providers: [UserService]
})
export class UserModule {}
```

해당 작업을 해주지 않으면 서비스에서 레포지토리를 찾을 수 없어 서버 실행 시 에러가 난다.

이제 **app.module.ts**에 엔티티를 등록한다.

```ts
entities: [User]
```

이제 `typeorm`에서 해당 엔티티에 대한 메타 데이터를 읽을 수 있다.

서버를 실행하고 아래 HTTP 파일을 통해 기능이 동작하는지 확인한다.

```
### Create
POST http://localhost:3000/user/create
Content-Type: application/json

{
    "username": "test",
    "password": "t1234",
    "email": "test@gmail.com"
}

### GetUser
GET http://localhost:3000/user/getUser/test@gmail.com

### Update User
PUT http://localhost:3000/user/update/test@gmail.com
Content-Type: application/json

{
    "username": "test2",
    "password": "2345",
    "email": "test@gmail.com"
}

### Delete
DELETE http://localhost:3000/user/delete/test@gmail.com
```

# 파이프로 유효성 검증

잘못된 값 입력에 대한 검증은 필수적이다. 익스프레스에서는 컨트롤러나 별도의 라이브러리를 이용하여 검증을 구현했지만, Nest.js에서는 파이프를 사용해서 유효성 검증을 한다.

해당 프로젝트에서는 `ValidationPipe`를 사용하여 유효성 검증을 한다. 그러기에 앞서 두 개의 패키지를 설치한다.

```
npm install class-validator class-transformer
```

`transformer`는 json 정보를 클래스 객체로 변경한다. 요청을 반환한 클래스가 컨트롤러의 핸들러 메소드의 매개변수에 선언되어 있는 클래스와 같다면 유효성 검증을 한다.

`validator`는 데코레이터를 사용해 유효성 검증을 하는 라이브러리다.

이 프로젝트에서는 `User` 엔티티를 바로 사용하지만, 원래는 데이터 전송 객체를 따로 만들어 사용한다.

유효성 검증을 위해서 유효성 검사 파이프를 **main.ts**에 설정한다.

```ts
import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { ValidationPipe } from '@nestjs/common';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  // 전역 파이프에 유효성 검사 파이프 객체 추가
  app.useGlobalPipes(new ValidationPipe());
  await app.listen(3000);
}
bootstrap();
```

**user.dto.ts** 객체를 만든다.

```ts
import { IsEmail, IsString } from 'class-validator';

export class CreateUserDto {
    @IsEmail()
    email: string;

    @IsString()
    password: string;

    @IsString()
    username: string;
}

// 업데이트의 유효성 검증
export class UpdateUserDto {
    @IsString()
    username: string;

    @IsString()
    paaword: string;
}
```

데이터 필드만 있는 클래스를 만드는 것과 다르지 않다.

**user.controller.ts**에서 `createUser`, `updateUser`에 선언한 `User` 부분을 수정한다.

```ts
import { CreateUserDto, UpdateUserDto } from './user.dto';

@Controller('user')
export class UserController {
    constructor(private userService: UserService) {}

    @Post('/create')
    createUser(@Body() user: CreateUserDto) {
        return this.userService.createUser(user);
    }

    @Put('/update/:email')
    updateUser(@Param('email') email: string, @Body() user: UpdateUserDto) {
        console.log(user);
        return this.userService.updateUser(email, user);
    }
    // 생략...
}
```

이제 서버를 실행하고 앞서 작성한 HTTP 파일을 통해 테스트해본다. 이메일 형식이 아니거나 문자열이 아니면 오류가 출력된다.