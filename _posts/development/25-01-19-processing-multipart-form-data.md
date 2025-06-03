---
title: multipart/form-data 처리하기

categories:
  - Spring

toc: true
toc_sticky: true
published: true
 
date: 2025-01-19
last_modified_at: 2025-01-26
---

HTTP 통신 프로토콜에서 클라이언트가 파일을 보낼 때 어떻게 서버에 전송하는 지 간단하게 알아보고 스프링에서 이런 파일을 처리하는 두 가지 방법을 소개하고자 한다.

# 클라이언트에서 파일 처리

일반적으로 클라이언트에서 파일 처리를 위해서는 `<form>` 양식에 `enctype="multipart/form-data"` 속성을 추가한다.

```html
<form action="/send" method="post" enctype="multipart/form-data">
    <input type="text" name="name">
    <input type="file" name="profile">
</form>
```

그러면 HTTP 메시지는 다음과 같이 요청된다.

```
POST /send HTTP/1.1
Host: example.com
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryXyZ123456789
Content-Length: [본문의 길이]

------WebKitFormBoundaryXyZ123456789
Content-Disposition: form-data; name="name"

[입력된 이름]
------WebKitFormBoundaryXyZ123456789
Content-Disposition: form-data; name="profile"; filename="[파일이름]"
Content-Type: [파일의 MIME 타입]

[파일의 바이너리 데이터]
------WebKitFormBoundaryXyZ123456789--
```

잘 보면 각각의 항목이 구분되어 전송 되는 것을 볼 수 있다.

# 서버에서 파일 처리

그렇다면 서버, 특히 스프링에서는 어떻게 이런 데이터를 처리할 수 있을까? 우선 스프링은 요청 매핑 어노테이션에서 `consumes`라는 속성을 지원한다. 이는 단어 뜻 그대로 **클라이언트에서 보낸 요청 형식 중에서 어떠한 특정 요청 형식을 소비한다**는 뜻이다. 

이는 명시하지 않으면 기본적으로 클라이언트에서 보내준 메시지 형식에 맞춰서 요청 메시지의 폼 데이터를 클래스 또는 레코드로 변환하는 리졸버가 동작한다. 이때 `multipart/form-data` 형식으로 요청을 보내면 해당 폼의 파일 부분은 `MultipartFile`이라는 클래스로 변활하여 처리할 수 있다. 

스프링에서는 이를 처리하기 위한 어노테이션으로 `@ModelAttribute`와 `@RequestPart`가 있다. 이 두 어노테이션을 소개하기 전에 요청의 전체 크기와 파일 크기를 먼저 설정한다.

```yaml
spring:
  servlet:
    multipart:
      max-request-size: 150MB  # (1) 전체 HTTP 요청의 최대 크기 제한 (멀티파트 포함)
      max-file-size: 100MB     # (2) 단일 파일 업로드의 최대 크기 제한
```

요청의 크기는 파일을 포함한 전체 요청 크기이기 때문에 조금 더 넉넉하게 잡아야 한다. 아직 실무를 경험해보지 않아서 그냥 파일의 크기보다 대강 1.5배로 잡았다.

## @ModelAttribute

`multipart/form-data`는 다음과 같은 요청 형식을 사용한다.

```
POST /api/user-request/upload/record HTTP/1.1
Host: localhost:8080
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW

------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="image"; filename="Players.pdf"
Content-Type: application/pdf

[PDF 파일 바이너리 데이터]
------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="title"

테스트 타이틀
------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="content"

테스트 콘텐츠
------WebKitFormBoundary7MA4YWxkTrZu0gW--
```

스프링에서 위와 같은 `multipart/form-data`를 `@ModelAttribute`로 처리하면 받은 요청에 대해서 내부적으로 기본 생성자 + `setter`를 호출하여 값을 매핑한다. 

```java
@Setter
@Getter
@ToString
public class MultipartRequestNoRecord {
    private String title;
    private String content;
}
```

```java
@PostMapping(value = "/upload/model-attribute")
public void uploadModelAttribute(
        @ModelAttribute MultipartRequestNoRecord request,
        @RequestPart("image") List<MultipartFile> images
) {
    log.info("request = {}", request);
    log.info("images = {}", images);
}
```

하지만 이 방식은 개인적으로 마음에 들지 않는데, 그 이유는 값을 설정하기 위해서 `setter`를 활욯해야 하기 때문이다. 실제로 `@Setter` 롬복 어노테이션을 제거하고 요청 값을 받으면 기본 생성자만 생성된 다음에 필드는 `null`이 되는 것을 볼 수 있다. 

개인적으로 개발을 할 때 `@RequestBody`를 이용하여 개발을 하기 때문에 `record`를 통해 요청 값들을 매핑한다. 이 방식의 장점은 내부적으로 JSON 데이터 처리를 위해 Jackson 라이브러리를 사용하여 값을 매핑하는데, 이 과정에서 `setter`가 없는 경우에 자동으로 리플랙션을 사용하여 객체에 값을 매핑한다. 이 덕분에 `setter`를 사용하지 않아도 되어 비즈니스 로직에서 혹시 모를 `setter` 호출을 아예 방지할 수 있다.

이쯤되면 '그냥 `setter`를 사용하지 않으면 되는 것 아닌가?' 싶지만, 개인적으로 사용할 수 있는 데 사용하지 않는 것과 아예 사용할 수 없는 것에는 큰 차이가 있다고 생각한다. OOP에서 내가 생각 했을 때 주요 철학은 **객체의 상태를 유지하면서도 개발자에게 일말의 실수할 기회조차 주지 않으며, 객체의 상태와 행위를 캡슐화하여 의도된 방법으로만 상호작용하도록 제한하는 것이 객체지향의 핵심 철학** 이라고 생각하기 때문이다. 

**개인적인 생각** 

물론 이는 고객이 요구하는 프로젝트의 규모와 기한에 따라서 다르게 설정해야 한다. 만약 단순한 기능에 변경 가능성이 단 1이라도 없다면, 과연 **컨트롤러-서비스-레포지토리**의 3개 계층으로 나누어야 하는가? 심지어 굳이 스프링을 써야 하는가? 까지의 물음도 해봐야 한다고 생각하고 있다. 

OOP의 철학이 위와 같다면 개발자는 위의 철학 이전에 **고객이 원하는 소프트웨어를 주어진 시간 내에 고품질로 어떻게 납품할 수 있을까?**를 먼저 고려해야 한다고 보기 때문이다. 

만약 납기일이 얼마 남지 않았는데 TDD라던가 유지보수를 위한 여러 규약등을 지키는 것에 대한 문제로 골머리를 썩고 있다면, 내 주변 개발자들은 그런 나를 보며 골머리를 썩을 것이다.

## @RequestPart

따라서 `record`로 `multipart/form-data`의 데이터를 처리할 방법을 생각해야 한다. 이때 `@RequestBody`는 `application/json` 형태를 기대하므로 이를 사용하면 에러가 발생한다. 스프링은 `@RequestPart`를 사용하여 JSON 값은 리플랙션으로 처리하고 파일은 `MultipartFile`로 처리할 수 있게 하였다. 요청 메시지는 다음과 같다.

```
POST /api/user-request/upload/record HTTP/1.1
Host: localhost:8080
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW

------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="data"; filename="blob"
Content-Type: application/json

{"title": "테스트 타이틀", "content": "테스트 콘텐츠"}
------WebKitFormBoundary7MA4YWxkTrZu0gW
Content-Disposition: form-data; name="image"; filename="Players.pdf"
Content-Type: application/pdf

[PDF 파일 바이너리 데이터]
------WebKitFormBoundary7MA4YWxkTrZu0gW
```

```java
@PostMapping(value = "/upload/record")
public void upload(
        @RequestPart(value = "data", required = false) MultipartRequest request,
        @RequestPart("image") List<MultipartFile> images
) {
    log.info("request = {}", request);
    log.info("images = {}", images);
}
```

이렇게 함으로써 모든 DTO를 `record`로 관리할 수 있게 되어 통일성과 혹시 모를 `setter` 호출을 방지할 수 있다. 하지만 이 방식은 클라이언트 입장에서 추가적인 처리가 필요하다. `title`과 `content`라는 변수로 사용자 입력 값을 받았다고 가정하면 클라이언트에서 요청을 보내기 전에 다음과 같이 값들을 정리해야 한다.

```js
const jsonData = JSON.stringify({ title: title, content: content });
const jsonBlob = new Blob([jsonData], { type: "application/json" });

// FormData 객체 생성
const formData = new FormData();
formData.append("data", jsonBlob); // JSON 데이터를 "data" 필드로 추가
formData.append("image", file);   // 파일을 "image" 필드로 추가
```

그 이유는 `Blob`으로 JSON 타입을 직렬화한 문자열과 해당 문자열이 JSON(`application/json`) 타입이라는 것을 명시해주어야 하기 때문이다. 따라서 `@RequestPart`를 사용하기 전에 클라이언트에게 API 명세서 등으로 적절하게 일러두어야 작업을 할 때 클라이언트가 개발 할 때 헷갈리지 않을 것이다.

### 수정 사항 추가 (2025-05-11)

파일 관련 API 테스트를 진행하는데, 위 방법이 지극히 JS 중심적인 것을 깨달아 버렸다. 또한 최근 스프링 부트가 업데이트 되어서 그런 건지 잘 모르겠는데 굳이 저런 식으로 하지 않고 단순히 multipart/form-data로 보내고 컨트롤러에서 다음과 같이 선언하면 요청을 잘 받는 것을 알게 되었다.

```java
@PostMapping(value = "/upload/record")
public void upload(
        MultipartRequest request,
        @RequestPart("image") List<MultipartFile> images
) {
    log.info("request = {}", request);
    log.info("images = {}", images);
}
```

이렇게 선언하면 multipart/form-data로 보낸 요청에 대해서 해당 요청 메소드가 제대로 처리하였다.

**[전체 코드 참고](https://github.com/sehako/playground/tree/feature/13)**

---

이렇게 간단하게 `multipart/form-data`를 처리하는 다양한 방법을 알아보았다. 개인적으로 요청 리졸버를 직접 정의해서 어떻게 할 수 있지 않을까... 싶긴 하지만 이는 너무 복잡할 것 같아서 굳이 알아보진 않았다. 추후에 여유가 된다면 한 번 해보도록 하겠다.