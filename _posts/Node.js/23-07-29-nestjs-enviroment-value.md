---
title:  "[Node.js] 08 - NestJS 환경 변수 설정"
excerpt: " "

categories:
  - Nodejs

toc: true
toc_sticky: true
 
date: 2023-07-29
---

# 환경 변수

배포를 하는 환경, 코드에는 들어가면 안되는 값들은 최소한 환경 변수로 설정하거나 vault 같은 외부 저장소에 두어야 한다. 이런 작업들은 코드로 제어해서는 안되고, 별도의 파일이나 외부의 서버에 설정해서 읽어올 수 있도록 해야한다.

Nest.js에서 환경 변수 설정은 `ConfigModule`에서 할 수 있으며, 설정된 환경 변수를 다른 모듈에서 가져다 쓰려면 `ConfigService`를 주입받아서 사용한다.

# 프로젝트 생성 및 설정

```
nest new config-test
npm i @nestjs/config
```

`@nestjs/config`는 내부적으로는 `dotenv`를 사용한다. **.env**라는 이름의 파일에 환경 변수를 설정하고 불러올 수 있게 하는 JS로 만든 라이브러리다.

# Nest.js 설정 및 테스트

**app.module.ts**

```ts
import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
//환경 변수 설정을 위한 import
import { ConfigModule } from '@nestjs/config';

@Module({
  //ConfigModule 설정
  imports: [ConfigModule.forRoot()],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
```

모든 환경 변수는 `ConfigModule`에서 시작한다. 

`forRoot()`는 많은 옵션이 존재한다. 그 중 자주 사용하는 옵션은 다음과 같다.

|옵션|설명|
|:---:|:---:|
cache|메모리 환경 변수를 캐시할 지 여부, 어플리케이션 성능 향상
isGlobal|`true`면 전역 모듈로 등록되어 다른 모듈에서 `import`가 필요 없어짐
envFilePath|환경 변수 파일(들)의 경로

## .env 파일 생성

환경 변수 설정을 가장 간단하게 하는 방법으로 **.env** 파일만 만드는 것이다. 프로젝트의 루트 디렉터리에 **.env** 파일을 만들고 다음 환경 변수를 키=값으로 저장한다.

```
# 기본 환경 설정 파일 .env의 환경 변수
MESSAGE=hello nestjs
```

여담으로 시스템 환경 변수를 보고싶다면 리눅스 기반 OS에서는 `env`, 윈도우에서는 `set`을 터미널에서 실행하면 된다.

**app.controller.ts**에 코드를 추가한다.

```ts
import { Controller, Get } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';

@Controller()
export class AppController {
  //ConfigService 주입
  constructor(private readonly configService: ConfigService) {}

  @Get()
  getHello(): string {
    //환경 변수 읽어오기
    const message = this.configService.get('MESSAGE');
    return message;
  }
}
```

`npm run start:dev`를 실행해 서버를 기동하고 접속하면 환경 변수가 웹에 표시된다.

## 전역 모듈 설정

환경 변수를 읽어오려면 `ConfigService`를 사용할 수 있어야 하는데, 그러기 위해서는 `ConfigModule`을 해당 모듈에 설정해야 한다. 프로젝트가 커지면 모듈의 개수가 커지므로 효율적으로 만들어야 한다. 이럴 때 `isGlobal` 옵션으로 전역 모듈을 만들어 각 모듈에서 선언해야 하는 다음 코드를 생략한다.

```ts
import { ConfigModule } from '@nestjs/config'; 
```

**app.module.ts**

```ts
import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { ConfigModule } from '@nestjs/config';

@Module({
  //전역 모듈 설정 추가
  imports: [ConfigModule.forRoot({ isGlobal: true })],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
```

이를 테스트 하기위해 책에서는 날씨 정보에 관한 외부 API를 호출하는 가짜 모듈을 만들어 테스트하였다. 

**.env**

```
MESSAGE=hello nestjs
WEATHER_API_URL=https://api.openweathermap.org/data/3.0/onecall?lat={lat}&lon={lon}&exclude={part}&appid={API key}
WEATHER_API_KEY=api_key
```

이후 루트 디렉터리에서 Nest CLI로 다음을 실행하면 API에 관한 모듈과 컨트롤러 클래스가 각각 생성된다.

```
nest g module weather
nest g controller weather --no-spec
```

**weather.controller.ts**

```ts
import { Controller, Get } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';

@Controller('weather')
export class WeatherController {
    //의존성 주입
    constructor(private configService: ConfigService) {}

    @Get()
    public getWeather(): string {
        //환경 변수값 가져오기
        const apiUrl = this.configService.get('WEATHER_API_URL');
        const apiKey = this.configService.get('WEATHER_API_KEY');

        return this.callWeatherApi(apiUrl, apiKey);
    }

    private callWeatherApi(Url: string, Key: string): string {
        console.log('날씨 정보 가져오는 중...');
        console.log(Url);
        console.log(Key);
        return '날씨 정보'
    }
}
```

`localhost:3000/weather`에서 결과를 확인할 수 있다.

### API 사용에 관한 간단한 예제 코드

```ts
import { Controller, Get } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';
import axios from 'axios';

@Controller('weather')
export class WeatherController {
    constructor(private configService: ConfigService) {}

    @Get()
    async getWeather() {
      const apiUrl = this.configService.get('WEATHER_API_URL');
      const apiKey = this.configService.get('WEATHER_API_KEY');
  
      // 날씨 API 호출
      return await this.callWheatherApi(apiUrl, apiKey);
    }
  
    async callWheatherApi(apiUrl: string, apiKey: string): Promise<string> {
      console.log('날씨 정보 가져오는 중...');
      console.log(apiUrl);
      const url = `${apiUrl}${apiKey}`;
      const result = await axios.get(url)
      const weather = result.data
      const mains = weather.weather.map((el) => el.main)
      return mains.join(" and ");
    }
}
```

# 여러 환경 변수 파일 사용

실무에서는 개발용(dev), QA용(qa), 베타 서비스용(beta), 실제 서비스용(prod | real)등 여러 가지 환경 변수를 사용한다. Node.js는 일반적으로 `NODE_ENV` 환경 변수에 용도 별 환경 변수를 정의해 사용한다. **package.json**의 `scripts` 부분에서 서버 실행에 관련된 키의 값을 수정한다.

```json
  "scripts": {
    "start": "set NODE_ENV=local&& nest start",
    "start:dev": "set NODE_ENV=dev&& nest start --watch",
    "start:prod": "set NODE_ENV=prod&& node dist/main",
  }
```

리눅스 기반 OS는 `set`을 생략하고 적으면 된다.

서버 가동시 확인을 위해 **app.module.ts**에 `console.log()`를 하나 작성한다.

```ts
//서버 실행 시 환경 변수 출력
console.log('env : ' + process.env.NODE_ENV)

@Module({
  //생략...
})
```

## local, dev, prod 환경 변수 생성

루트 디렉터리에서 **envs/**를 만들어 해당 디렉터리에 환경별 다른 이름을 가진 **.env** 파일을 생성하고 각각 아래와 같이 작성한다.

```
# local.env
SERVICE_URL=http://localhost:3000
# dev.env
SERVICE_URL=http://dev.config-test.com
# prod.env
SERVICE_URL=http://config-test.com
```

그리고 **app.module.ts**의 `imports` 부분에 `envFilePath` 옵션을 추가하고 다음과 같이 작성한다.

```ts
//import 생략...

console.log('env : ' + process.env.NODE_ENV)
console.log('current working dir : ' + process.cwd())
console.log(`${process.cwd()}/envs/${process.env.NODE_ENV}.env`);

@Module({
  // 전역 모듈 설정 추가
  imports: [ConfigModule.forRoot({ isGlobal: true,
  // 현재 실행 되는 프로젝트의 루트 디렉터리 반환후 경로 설정
  // 이후 실행 옵션에 따라서 .env파일 이름 설정
    envFilePath: `${process.cwd()}/envs/${process.env.NODE_ENV}.env`,
  }), 
  WeatherModule
],
  //생략...
})
export class AppModule {}
```

**app.controller.ts**에 다음 `@Get()` 핸들러 함수를 추가한다.

```ts
// 생략...
@Get('/service-url')
getServiceUrl(): string {
  // SERVICE_URL 환경 변수 반환
  return this.configService.get('SERVICE_URL');
}
```

`localhost:3000/service-url`에 진입하면 각 실행 옵션에 따른 환경 변수 값이 출력된다.

# 커스텀 환경 설정 파일 사용

**.env** 확장자를 가지는 환경 설정 파일 이외에 **.ts**를 확장자로 가지는 파일도 환경 설정 파일로 사용할 수 있다. 해당 설정 파일은 복잡한 설정이 필요할 때 사용한다. 예를 들어, 환경 변수 파일에 공통으로 넣을 환경 변수 설정, YAML 파일을 환경 변수로 사용, 설정에 대한 유효성 검증 등이 있다.

**srcc**에서 **configs/**를 만들고 **common.ts**를 생성하여 작성한다.

```ts
// 공통 환경 변수 반환
export default {
    logLevel: 'info',
    apiVersion: '1.0.0',
    MESSAGE: 'hello',
};
```

그 후 **local / dev / prod / config.ts**를 각각 작성한다. 

```ts
// 로컬 개발의 환경 변수
export default {
    //DB 접속 정보
    dbInfo: 'http://localhost:3452',
};
```

```ts
//개발에서의 환경 변수
export default {
    logLevel: 'debug',
    dbInfo: 'http://prod-mysql:3452',
};
```

```ts
//프로덕션에서의 환경 변수
export default {
    logLevel: 'error',
    dbInfo: 'http://prod-mysql:3452',
};
```

```ts
import common from './common';
import local from './local';
import dev from './dev';
import prod from './prod';

//NODE_ENV값 저장
const phase = process.env.NODE_ENV;

//phase 값에 따라 적절한 환경 변수값을 저장
let conf = {};

if(phase === 'local') {
    conf = local;
}
else if(phase === 'dev') {
    conf = dev;
}
else if(phase === 'prod') {
    conf = prod;
}

//common과 conf에서 받은 값을 합쳐 반환
export default () => ({
    ...common,
    ...conf,
});
```

`export` 에서 스프레드 연산자(`...`)로 두 값을 합친다. `load()` 옵션에서는 `() => ({})`의 형태로 값을 감싸줘야 하기에 `({})`로 한 번 객체를 감싸주었다.

## ConfigModule에 load 옵션 추가

커스텀 파일 설정을 위해 `load` 옵션을 추가해야 한다. 

**app.module.ts**에서 옵션을 추가한다.

```ts
//생략...
import config from './configs/config';

@Module({
  imports: [ConfigModule.forRoot({ isGlobal: true,
    envFilePath: `${process.cwd()}/envs/${process.env.NODE_ENV}.env`,
    //커스텀 설정 파일 설정
    load: [config],
  }), 
  //생략...
  ]
})
```

**app.controller.ts**에 다음 핸들러를 추가한다.

```ts
@Get('db-info')
getTest(): string {
  console.log(this.configService.get('logLevel'));
  console.log(this.configService.get('apiVersion'));

  //브라우저에 dbInfo 출력
  return this.configService.get('dbInfo');
}
```

서버의 시작 명령에 따라서 웹과 로그에 출력되는 값이 달라진다.

# YAML 파일로 환경 변수 설정

최근에는 YAML을 사용하는 경우가 많다. 쿠버네티스, 스프링, 엔서블 등에서는 지원하고 있기 때문에 알아두면 좋다.

YAML은 간결한 문법과 json에서 표현하는 모든 데이터를 표현할 수 있다. 주석도 지원한다.

우선 Nest.js에서 사용하기 위해 패키지를 설치한다.

```npm
npm i js-yaml
npm i -D @types/js-yaml
```

**src/configs/**에서 **config.yaml**을 생성하여 작성한다.

```yaml
http:
  port: 3000

redis:
  host: 'localhost'
  port: 3452
```

YAML 파일은 커스텀 설정 파일로 취급하므로 **config.ts**에 설정을 추가한다. 

```ts
//생략...
import { readFileSync } from 'fs';
import * as yaml from 'js-yaml';
//생략...

//yaml 파일 로딩
const yamlConfig: Record<string, any> = yaml.load(
    readFileSync(`${process.cwd()}/envs/config.yaml`, 'utf8'),
);

export default () => ({
    ...common,
    ...conf,
    ...yamlConfig
});
```

**app.controller.ts**에 핸들러를 추가한다.

```ts
@Get('redis-info')
getRedisInfo(): string {
  return `${this.configService.get('redis.host')}:${this.configService.get('redis.port')}`;
}
```

`localhost:3000/redis-info`에 접속하면 결과를 확인할 수 있다.

# 캐시 사용하기

설정 파일은 서버 실행 뒤에는 변경되지 않기에 캐시를 사용하여 성능에서 이득을 볼 수 있다.

**app.module.ts**에 다음 옵션을 추가한다.


```ts
ConfigModule forRoot({
  // 생략...
  cache: true
  // 생략...
})
```

이제 `ConfigService.get()` 함수를 사용할 때 캐시에서 먼저 불러오기에 성능 향상을 기대할 수 있다.

# 확장 변수 사용

이미 선언된 변수를 다른 변수에 `&{변수명}`으로 할당하는 기능이다.

```
# 환경 변수 선언
SERVER_DOMAIN=localhost
SERVER_PORT=3000

# 확장 변수 기능을 사용한 변수 선언
SERVICE_URL=http://${SERVER_DOMAIN}:${SERVER_PORT}
```

내부적으로는 dotenv-expand 패키지를 사용한다.

**app.module.ts**에 확장 변수 사용을 위한 설정을 해준다.

```ts
ConfigModule forRoot({
  // 생략...
  expandVariables: true,
  // 생략...
})
```

앞서 만들었던 `localhost:3000/service-url`로 확인한다.

# main.ts에서 환경 변수 사용

가장 먼저 실행이 되므로 해당 파일에서 `NestFactory.create()`를 호출하기 전에는 `ConfigModule`이 활성화되지 않는다. 또한 클래스가 아니라 `bootstrap()` 함수만 있으므로 의존성 주입을 받을 수 없다. 따라서 `app.get()` 메소드에 `ConfigService` 클래스를 인수로 주고. 반환값을 받는 방식을 사용한다.

```ts
import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { ConfigService } from '@nestjs/config';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  //ConfigService를 app.get()에 추가
  const configService = app.get(ConfigService);
  // configService 사용하여 SERVER_PORT(3000)을 불러옴
  await app.listen(configService.get("SERVER_PORT"));
}
bootstrap();
```