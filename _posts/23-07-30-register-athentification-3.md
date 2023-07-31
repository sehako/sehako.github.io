---
title:  "[Node.js] 09 - 회원 가입과 인증 - 3"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-30
---

# 패스포트와 세션을 사용한 인증

쿠키만으로 인증하면 위변조와 탈취의 위험이 있다. 좋은 방법은 서버에서 인증을 하고 해당 정보를 서버의 특정 공간에 저장해두는 것이다. 이때 세션을 사용한다. 

쿠키는 세션 아이디 값 같은 세션을 찾는 정보만 저장하고, 중요 정보는 세션에 모두 넣는 것이다. 이 방법은 서버의 자원을 사용하지만 위조 및 변조와 탈취가 불가능하므로 보안이 향상된다. 

프로젝트에서 인증 로직 구현은 패스포트라는 라이브러리를 사용한다. 해당 라이브러리 사용 시 인증 로직은 **strategy** 파일을 생성하여 처리한다.

세션 사용 시 데이터를 저장하고 읽어올 **session serializer** 파일도 필요하다. 

정리하자면 가드에서 검증을 위해 strategy로 데이터를 넘기고 strategy는 session serializer로 넘겨 세션 정보 해석을 요청한다. 그 후 값을 다시 거슬러 올라가 가드로 전달한다.

## 라이브러리 설치 및 설정

```
npm i @nestjs/passport passport passport-local express-session
npm i -D @types/passport-local @types/express-session
```

**main.ts**에 다음 코드를 추가한다.

```ts
import * as session from 'express-session';
import * as passport from 'passport';

async function bootstrap() {
  app.use(
    session({
      secret: 'very-important-secret',
      resave: false,
      saveUninitialized: false,
      cookie: { maxAge: 3600000 },
    }),
  );
}
```

## 가드 구현

로그인에 사용할 가드와 로그인 후 인증에 사용할 가드를 별도로 만들어서 사용한다.

**auth.guard.ts**에 다음 코드를 추가한다.

```ts
import { CanActivate, ExecutionContext, Injectable } from '@nestjs/common';
import { AuthService } from './auth.service';
import { AuthGuard } from '@nestjs/passport';

@Injectable()
// AuthGuard 상속, 로컬 strategy 사용
export class LocalAuthGuard extends AuthGuard('local') {
    // passport-local의 로직을 구현한 메소드 실행
    async canActivate(context: ExecutionContext): Promise<boolean>{
        const result = (await super.canActivate(context)) as boolean;
        const request = context.switchToHttp().getRequest();
        // 로그인 처리, 해당 코드에서는 세션 저장
        await super.logIn(request);
        return result;
    }
}

@Injectable()
// 로그인 후 인증이 되었는지 확인
export class AuthenticatedGuard implements CanActivate {
    canActivate(context: ExecutionContext): boolean {
        const request = context.switchToHttp().getRequest();
        return request.isAuthenticated();
    }
}
```

로컬 strategy 이외에 `passport-jwt`와 `passport-google-oauth20`등이 있다. 세션을 저장하고 정보를 읽어오는 부분은 **session.serializer.ts**에 작성한다.

```ts
import { Injectable } from '@nestjs/common';
import { PassportSerializer } from '@nestjs/passport';
import { UserService } from 'src/user/user.service';


@Injectable()
// 상속 
export class SessionSerializer extends PassportSerializer {
    constructor(private userService: UserService) {
        super();
    }

    // 세션에 정보를 저장
    serializeUser(user: any, done: (err: Error, user: any) => void): any {
        done(null, user.email);
    }

    // 세션에서 가져온 정보로 유저 정보 반환
    async deserializeUser(
        payload: any,
        done: (err: Error, payload: any) => void,
    ): Promise<any> {
        const user = await this.userService.getUser(payload);
        // 유저 정보가 없는 경우 done() 함수에 에러 전달
        if(!user) {
            done(new Error('유저 없음'), null);
            return;
        }
        const { password, ...userInfo } = user;

        // 유저 정보 반환
        done(null, userInfo);
    }
}
```

`PassportSerializer`는 `getPassportInstance()` 메소드 또한 제공한다. 해당 메소드는 패스포트 인스턴스를 가져온다.

`payload`는 세션에서 꺼내온 값이다. 위 코드에서는 이메일 정보만 전달된다. 

실제 인증 로직을 담는 파일은 **local.strategy.ts**다. id, password로 인증하는 기능은 `passport-local` 패키지에서 제공한다.

```ts
import { Injectable } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy } from 'passport-local';
import { AuthService } from './auth.service';

@Injectable()
export class LocalStrategy extends PassportStrategy(Strategy) {
    // 믹스인
    constructor(private authService: AuthService) {
        // 기본값이 username이기 때문에 email로 변경
        super({ usernameField: 'email' });
    }

    // 유효성 검증
    async validate(email: string, password: string): Promise<any> {
        const user = await this.authService.validateUser(email, password);
        if(!user) {
            // 401에러 유도
            return null;
        }
        // 유저 정보 반환
        return user;
    }
}
```

클래스의 일부만 확장하고 싶을 땐 믹스인을 사용한다. 

**auth.module.ts**에 다음 코드를 추가한다.

```ts
import { PassportModule } from '@nestjs/passport';
import { SessionSerializer } from './session.serializer';
import { LocalStrategy } from './local.strategy';

@Module({
    // 패스포트 모듈 추가
    imports: [UserModule, PassportModule.register({ session: true })],
    // 프로바이더 설정 추가
    providers: [AuthService, LocalStrategy, SessionSerializer],
    controllers: [AuthController]
})
export class AuthModule {}
```

마지막으로 **auth.controller.ts**에 다음 코드를 추가한다.

```ts
import { AuthenticatedGuard, LocalAuthGuard, LoginGuard } from './auth.guard';

@UseGuards(LocalAuthGuard)
@Post('login3')
login3(@Request() req) {
    return req.user;
}

@UseGuards(AuthenticatedGuard)
@Get('test-guard2')
testGuardWithSession(@Request() req) {
    return req.user;
}
```

HTTP 파일에 다음 테스트를 추가하고 테스트한다. 틀린 패스워드 입력 시 401 에러가 반환되고 인증이 성공하면 유저 정보를 볼 수 있다.

```
###로그인3
POST http://localhost:3000/auth/login3
Content-Type: application/json

{
    "email": "test@gmail.com",
    "password": "1234"
}


### 인증 테스트
GET http://localhost:3000/auth/test-guard2
```

현재에는 세션 정보가 메모리에 저장되므로 서버를 재부팅하면 초기화된다.