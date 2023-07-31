---
title:  "[Node.js] 09 - 회원 가입과 인증 - 2"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-30
---

# 인증

인증은 정확성의 측면에서 사용자의 기존 정보를 기반으로 확인 후 인증 토큰을 발급하는 것이고, 시간 측면에서 사용자에게 부여된 인증 토큰은 특정 기간동안 유효하다는 것이다.

인증은 쿠키를 기반으로 만들거나 토큰을 기반(쿠키리스)으로 만들 수 있다. 쿠키는 서버에서 보내준 값을 클라이언트에 저장해 관리한다. 토큰은 서버에 상태를 저장할 필요가 없고 커스텀 HTTP 헤더를 사용한다.

**장점**

- 쿠키

    - 하위 도메인에서 같은 세션을 사용 가능
    - 저장 공간을 적게 차지
    - 브라우저에서 관리
    - `httpOnly` 설정을 하면 클라이언트에서 JS로 조작 불가능
- 토큰

    - 유연하고 간단한 사용
    - 크로스 플랫폼 대응
    - 다양한 프론트 앤드 어플리케이션에서 대응

**단점**

- 쿠키

    - 사이트 간 위조 공격(CSRF) 가능
    - 스케일링 이슈 존재
    - API 인증에는 부적합

- 토큰
    
    - 어려운 권한 삭제, 거부 목록을 따로 관리하면 무상태가 아니게 됨
    - 쿠키보다 더 많은 저장공간 차지
    - JWT 내부정보가 최신 데이터를 반영하지 않을 수 있음(토큰 생성 시 데이터)

# 인증 모듈 생성 및 설정

인증 모듈을 생성하기 위해 프로젝트 루트 디렉토리에서 다음 명령어를 입력한다.

```
nest g module auth
nest g service auth --no-spec
nest g controller auth --no-spec
```

유저 모듈을 외부에서 사용할 수 있도록 **user.module.ts**에 다음 설정을 추가한다.

```ts
exports: [UserService]
```

**auth.module.ts**에서 유저 모듈을 불러온다.

```ts
import { Module } from '@nestjs/common';
import { AuthService } from './auth.service';
import { AuthController } from './auth.controller';
import { UserModule } from 'src/user/user.module';

@Module({
  imports: [UserModule],
  providers: [AuthService],
  controllers: [AuthController]
})
export class AuthModule {}
```

# 회원 가입

기존 코드는 패스워드 암호화가 되어있지 않기에 암호화 코드를 추가한다. 암호화 모듈로 `bcrypt`를 사용한다. 프로젝트 루트 디렉토리에서 다음 명령어를 터미널에 입력한다.

```
npm install bcrypt
npm install -D @types/bcrypt
```

**auth.service.ts**

```ts
import { HttpException, HttpStatus, Injectable } from '@nestjs/common';
import { CreateUserDto } from 'src/user/user.dto';
import { UserService } from 'src/user/user.service';
import * as bcrypt from 'bcrypt';

@Injectable()
export class AuthService {
    constructor(private userService: UserService) {}

    async register(userDto: CreateUserDto) {
        // async 메소드이므로 await 사용
        const user = await this.userService.getUser(userDto.email);
        if(user) {
            throw new HttpException(
                '유저가 이미 존재함',
                HttpStatus.BAD_REQUEST,
            );
        }

        // 암호화 진행
        const encryptedPassword = bcrypt.hashSync(userDto.password, 10);

        // DB에 저장
        try {
            const user = await this.userService.createUser({
                ...userDto,
                password: encryptedPassword,
            });
            // 반환 값에서 password 부분을 익명처리하여 보안
            user.password = undefined;
            return user;
        } catch(err) {
            throw new HttpException('서버 에러', 500);
        }
    }
}
```

**auth.controller.ts**

```ts
import { Body, Controller, Get, Post } from '@nestjs/common';
import { CreateUserDto } from 'src/user/user.dto';
import { AuthService } from './auth.service';

@Controller('auth')
export class AuthController {
    constructor(private authService: AuthService) {}

    @Post('register')
    // 자동으로 유효성 검증
    async register(@Body() userDto: CreateUserDto) {
        // user 정보 저장
        return await this.authService.register(userDto);
    }
}
```

회원 가입을 하는 HTTP 파일을 작성하여 테스트해본다.

```
### 회원 가입
POST http://localhost:3000/auth/register
Content-Type: application/json

{
    "username": "test",
    "password": "1234",
    "email": "test@gmail.com"
}
```

유저가 이미 존재한다면 400에러가 발생할 것이다.

# 쿠키를 이용한 인증 구현

Nest.js에서 인증을 구현할 때는 인증용 미들웨어인 `Guard`를 함께 사용하여 권한, 롤, 엑세스컨트롤에서 받은 요청을 가드를 추가한 라우터에서 처리할지 결정하는 역할을 한다.

쿠키에 데이터를 추가하기 전에 유저의 데이터 검증 로직을 **auth.service.ts**에 구현한다.

```ts
async validateUser(email: string, password: string) {
    // 이메일로 유저 정보 가져옴
    const user = await this.userService.getUser(email);

    if(!user) {
        return null;
    }
    // 패스워드만 추출
    const { password: hashedPassword, ...userInfo } = user
    // 패스워드 일치하는지 비교
    if(bcrypt.compareSync(password, hashedPassword)) {
        return userInfo;
    }
    return null;
}
```

입력받은 패스워드 값과 패스워드 해시값을 비교하여 일치하면 유저정보를 반환한다.

**auth.controller.ts**에 라우터를 추가한다.

```ts
import { Body, Controller, Get, Post, 
    Request, Response, UseGuards } from '@nestjs/common';

// 생략...
@Post('login')
async login(@Request() req, @Response() res) {
    // 유저 정보 획득
    const userInfo = await this.authService.validateUser(
        req.body.email,
        req.body.password,
    );

    // 유저 정보가 있으면 쿠키 정보를 응답에 저장
    if(userInfo) {
        res.cookie('login', JSON.stringify(userInfo), {
            // 브라우저에서 읽을 수 있도록 설정
            httpOnly: false,
            // 쿠키 유효 시간 밀리초 단위로 계산됨
            maxAge: 1000* 10,
        });
    }
    return res.send({ message: 'login success' })
}
```

`login()`은 요청과 응답을 모두 사용하기 때문에 `@Body`나 `@Param`이 아닌 `@Request`를 직접 사용한다. 서비스에 구현했던 함수(`authService`)를 호출해 받은 유저 정복 ㅏ있다면 쿠키를 설정한다. 

`httpOnly`는 `true`로 설정하면 브라우저에서 쿠키를 읽지 못한다. 이는 XSS 공격의 방지를 위해 보통은 `true`로 설정한다.

HTTP 파일에 로그인 테스트를 작성하고 테스트해본다.

```
###로그인
POST http://localhost:3000/auth/login
Content-Type: application/json

{
    "email": "test@gmail.com",
    "password": "1234"
}
```

반환값으로 `Set-Cookie`가 존재해야 한다.

## 인증 검사

가드는 `@Injectable()`가 붙어 있고 `CanActivate` 인터페이스를 구현한 클래스다. 클라이언트의 요청을 핸들러에 넘기기 전에 인증에 관련된 처리가 가능하다. `CanActivate`의 `canActivate()`를 구현하여 사용하는데, 해당 메소드는 `Promise<boolean>` 또는 `boolean`을 반환하며 `true`인 경우 핸들러를 실행하고 아니면 403 에러를 반환한다.

HTTP 요청 헤더에 있는 쿠키를 읽는 코드가 필요하다. 프로젝트 루트 디렉토리에서 다음 명령어를 터미널에 입력한다.

```
npm install cookie-parser
```

**main.ts** 에서 미들 웨어를 설정한다.

```ts
import * as cookieParser from 'cookie-parser';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  app.useGlobalPipes(new ValidationPipe());
  app.use(cookieParser());
  await app.listen(3000);
}
bootstrap();
```

요청 객체에서 쿠키를 읽어오는데 사용한다. 

**auth.guard.ts**

```ts
import { CanActivate, Injectable } from '@nestjs/common';
import { AuthService } from './auth.service';

@Injectable()
// 인터페이스 구현
export class LoginGuard implements CanActivate {
    constructor(private authService: AuthService) {}

    // 메소드 구현
    async canActivate(context: any): Promise<boolean> {
        // 요청 정보 가져옴
        const request = context.switchToHttp().getRequest();

        // 쿠키가 존재하는지 검사
        if(request.cookies['login']) {
            return true;
        }

        // 쿠키와 body 정보 둘 다 없는 경우
        if(!request.body.email || !request.body.password) {
            return false;
        }

        // 요청에서 이메일 패스워드 가져옴
        const user = await this.authService.validateUser(
            request.body.email,
            request.body.password,
        );

        // 요청 값 없는 경우
        if(!user) {
            return false;
        }

        // 요청에 유저 정보 저장
        request.user = user;
        return true;
    }
}
```

위 코드에서 `async`와 `await`를 사용하므로 추상 메소드 구현은 `Promise<boolean>`으로 한다.

가드는 내부에서 응답에 쿠키를 설정할 수 없다. 가드는 모든 미들웨어의 실행이 끝난 다음 실행되며 필터나 파이프보다는 먼저 실행된다.

**auth.controller.ts**에 핸들러 추가

```ts
import { Body, Controller, Get, Post, 
    Request, Response, UseGuards } from '@nestjs/common';
import { LoginGuard } from './auth.guard';

// 생략...
@UseGuards(LoginGuard)
@Post('login2')
async login2(@Request() req, @Response() res) {
    if(!req.cookies['login'] && req.user) {
        res.cookie('login', JSON.stringify(req.user), {
            httpOnly: true,
            maxAge: 1000 * 10,
        });
    }
    return res.send({ message: 'login2 성공' });
}

@UseGuards(LoginGuard)
@Get('test-guard')
testGuard() {
    return '로그인 해야 이 글이 보임'
}
```

인증에 성공 시 `request.user`에 유저 정보를 할당했다. 쿠키 정보가 없으나 유저 정보가 있는 경우 로그인을 진행한 것으로 간주하여 쿠키를 설정한다.

HTTP 요청에 다음 테스트를 추가하고 테스트해본다.

```
###로그인2 
POST http://localhost:3000/auth/login2
Content-Type: application/json

{
    "email": "test@gmail.com",
    "password": "1234"
}

###Guard 테스트
GET http://localhost:3000/auth/test-guard
```

`POST` 요청을 실행하고 `GET` 요청을 바로 보내면 정상적으로 문구가 보이지만, 10초(위 코드에서 설정한 쿠키 지속 시간) 이후에 보내면 403 에러를 반환한다.