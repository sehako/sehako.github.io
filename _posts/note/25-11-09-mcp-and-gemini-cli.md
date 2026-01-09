---
title: 우테코 프리코스 - 2주차 과제 회고

categories:
  - Note

toc: true
toc_sticky: true
published: true

date: 2025-11-09
last_modified_at: 2025-11-09
---

# 기존 AI의 한계점

이제는 LLM과 함께 코딩을 한다는 말이 더 이상 특별하지 않은 말이 되었다. 심지어 코딩이라는 분야를 넘어서 이제는 몇몇 사람들은 LLM에게 질문을 잘 하는 사람이 살아남을 것이라는 말을 하기도 한다. 이렇게 LLM는 현대 사회에서 가장 큰 화두이지만, 다음과 같은 한계가 존재한다.

- 업데이트의 난해함

ChatGPT를 보면 매번 모델을 업데이트 하는 것을 알 수 있다. 이는 GPT가 특정 기간까지의 지식만 학습할 수 있기 때문이다. 예를 들어 GPT-4는 2023년 4월까지의 데이터를 기반으로 학습했다. 새로운 모델을 학습하려면 많은 시간을 요구하고 아이러니하게도 그 기간동안 새로운 모델이 학습하는 데이터는 구식이 된다. 다르게 말하면 특정 기간 이후에 생긴 기술 변경에는 잘 대응하지 못한다.

- 부족한 전문 지식

보편적인 데이터셋을 훈련하기 때문에 사용자 특화된 답변을 주지 못한다. 예를 들어 하나의 클래스나 기능을 잘 설계할 수는 있지만 개발자 개개인의 도메인에 특화되어 있지 않기 때문에 문제가 발생할 수 있다.

- 외부 데이터에 엑세스하기 위한 표준의 부재

프로젝트 자체 파일이나 데이터베이스 등의 시스템과 연동을 할 수 있는 수단이 부족하다. 단순하게 테이블에 특화된 데이터를 조회하고자 하면 해당 테이블의 DDL을 넘겨주면서 ‘테이블 구조가 이런데 어떠어떠한 쿼리를 추천해줘’ 이런 식으로 사용자가 AI에게 외부 정보를 매번 프롬프트해야한다.

# MCP의 등장

MCP는 기존의 AI의 한계를 극복할 수 있는 수단으로써 주목받았다. MCP를 소개하면 다음과 같다.

> MCP는 LLM이 외부 데이터, 애플리케이션, 서비스와 통신할 수 있는 안전하고 표준화된 '언어'를 제공합니다. AI가 정적 지식을 넘어 현재 정보를 검색하고 조치를 취할 수 있는 동적 에이전트가 되도록 지원하는 브리지 역할을 하여 AI의 정확성, 유용성, 자동화를 높입니다.

정리하자면 기존의 웹에서 사용자와 통신하는 것을 넘어서, 다양한 외부 요소와 통신을 할 수 있다고 보면 될 것 같다. 이를 통해 얻을 수 있는 이점은 다음과 같다고 한다.

- 풍부한 사전 구축 통합

파일 시스템, 데이터베이스, 개발 도구 등 다수의 사전 제작된 서버 통합을 제공한다.

- LLM 제공자 간 유연한 전환

기존의 정보는 유지하면서 상황에 맞는 특화된 LLM 모델을 활용할 수 있다.

- 복잡한 AI 워크플로우 구축

복잡한 데이터베이스와 API 구조에 대한 처리에 특화된 에이전트와 워크플로우를 구축할 수 있게 한다.

## MCP 작동 방식

MCP에는 세 가지 핵심 역할이 있다. MCP 서버, MCP 클라이언트, 그리고 MCP 호스트다. 각각의 역할을 살펴보자.

### MCP 서버

![image.png](/assets/images/mcp-and-gemini-cli_01.png)

MCP 서버는 LLM이 사용할 수 있는 도구와 데이터 엑세스 기능을 제공한다. 로컬 또는 원격 서버에 배포될 수 있다.

### MCP 클라이언트

LLM과 MCP 서버를 연결하여 다음과 같은 일을 수행한다.

- LLM으로부터 요청 수신
- 적절한 MCP 서버로 요청 전달
- MCP 서버로부터 결과를 LLM에 반환

![image.png](/assets/images/mcp-and-gemini-cli_02.png)

### MCP 호스트

Claude Code, Gemini CLI이 바로 여기에 해당한다. 사용자가 LLM과 상호작용 할 수 있는 인터페이스를 제공함과 동시에, MCP 클라이언트 통합하여 MCP 서버가 제공하는 도구를 사용한다.

![image.png](/assets/images/mcp-and-gemini-cli_03.png)

---

위 세 가지 요소가 결합하여 다음과 같이 동작하는 것이 바로 MCP이다.

![image.png](/assets/images/mcp-and-gemini-cli_04.png)

## RAG와의 차이점

RAG(검색 증강 생성)은 AI 답변 품질의 향상(할루시네이션 방지, 사용자 특화된 답변 등)을 위해 검색을 통한 외부 정보를 조회하고 이를 참고하여 답변을 주는 방식이다. MCP와 RAG가 어떤 차이점을 가지고 있는지 비교해보면 다음과 같다.

| **기능**  | MCP                                                                                                                      | **RAG**                                                                                                                         |
| --------- | ------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------- |
| 기본 목표 | 외부와의 상호작용을 통한 작업 수행을 위한 양방향 통신 표준화                                                             | 신뢰할 수 있는 정보 조회를 통한 LLM 답변 신뢰도 향상                                                                            |
| 메커니즘  | 표준화된 프로토콜을 정의하여 작업 및 동적 컨텍스트 통합을 지원                                                           | 사용자의 질문으로 정보를 검색하고 검색 결과는 LLM의 프롬프트를 보강한다.                                                        |
| 상호작용  | 외부 시스템에서 활발한 상호작용과 작업 실행을 위해 설계되었으며, LLM이 외부 기능을 '사용'할 수 있는 '문법'을 제공한다.   | 주로 텍스트 생성을 위한 정보의 수동 검색에 사용되며, 일반적으로 외부 시스템 내에서 작업을 실행하는 데는 사용되지 않는다.        |
| 표준화    | AI 애플리케이션이 LLM에 컨텍스트를 제공하는 방식에 대한 개방형 표준으로, 통합을 표준화하고 커스텀 API의 필요성을 줄인다. | LLM을 개선하기 위한 기법 또는 프레임워크이지만, 여러 공급업체 또는 시스템 전반에서 도구 상호작용을 위한 범용 프로토콜은 아니다. |
| 사용 사례 | AI 에이전트는 작업(예: 항공편 예약, CRM 업데이트, 코드 실행)을 수행, 실시간 데이터를 조회, 고급 통합.                    | 질의 응답 시스템, 최신 사실 정보를 제공하는 챗봇, 문서 요약, 텍스트 생성 시 할루시네이션 감소                                   |

정리하자면 MCP는 외부 요소와의 상호작용을 할 수 있는 수단으로, 해당 요소를 기반으로 답변을 주는 것을 넘어서 외부 요소를 직접 조작할 수도 있는 양방향 통신 프로토콜이고, RAG는 답변 생성 이전에 신뢰성을 높이기 위해 외부 자료를 검색하여 검색 결과를 답변에 참고하는 기법이다.

# Gemini CLI

MCP 호스트 중 하나인 Gemini CLI는 로컬 터미널에서 직접 Gemini에 엑세스할 수 있는 오픈소스 AI 에이전트이다. 이를 기반으로 사용자의 코딩에 도움을 줄 수 있는 것이 바로 Gemini Code Assist이다. 원래는 클로드 코드를 사용해보고 싶었지만, Gemini CLI가 하루 1,000회 무료 요청이 가능하기 때문에 이를 직접 사용해보며 MCP가 무엇인지 알아보도록 하자. 다음 명령어로 Gemini CLI를 설치하자.

```bash
npm install -g @google/gemini-cli
```

설치가 완료되면 다음 명령어로 Gemini CLI를 실행하면 된다.

```bash
gemini
```

그러면 다음과 같이 로그인 할 수 있는 방법을 제안한다.

```
1. Login with Google
2. Use Gemini API Key
3. Vertex AI
```

나는 간편하게 구글로 로그인을 했다.

## 명령어 모음

기본적으로 `gemini` 명령어로 실행 후 대화를 하면 되지만, 다음과 같은 명령어를 사용해서 모델 선택이나 다양한 설정을 할 수 있다.

**기본 명령행 옵션**

- 도움말 확인: `gemini --help`
- 모델 선택: `gemini -m`
- 단일 프롬프트 실행: `gemini -p "설명할 코드 입력"`

**대화형 세션 & 슬래시 명령어**

- 대화 기록 초기화: `/clear`
- 세션 종료: `/quit` 또는 `/exit`
- 대화 기록 관리: `/chat list`, `/chat save`, `/chat resume`
- 컨텍스트 요약: `/compress`
- 사용 가능한 도구 목록: `/tools`
- 세션 통계 확인: `/stats`
- 테마 변경: `/theme`
- 인증 방법 변경: `/auth`
- 버전 정보 확인: `/about`

**파일 및 폴더 참조**

- 특정 파일, 폴더 컨텍스트 추가:
  프롬프트에서 `@src/myFile.java`처럼 경로 입력
  예시: `@src/utils/helper.java 이 코드 리팩터링 방법 설명해줘`

**특수/고급 명령**

- 쉘 명령 직접 실행: `!npm run start`, `!ls -la`
- 쉘 모드(연속 실행): `!` 명령 단독 실행 후 쉘 명령 연속 ارسال
- 자동승인 모드: `y, --yolo`로 실행(파일 변경/생성 자동 적용)
- 디버그 모드: `d, --debug` 옵션으로 실행(과정 상세 출력)
- 샌드박스 실행: `-sandbox` 플래그 이용(격리 환경에서 명령 적용)

## 활용해보기

간단하게 MCP가 어떤 방식으로 동작하는지 알아보도록 하자. 내가 작성한 깃헙 블로그 마크다운 파일로 내 역량을 한 번 파악해보도록 하겠다. 우선 기본적으로 실행된 디렉터리의 하위 파일들을 모두 포함하도록 설정되어 있기 때문에 블로그 프로젝트 루트에서 Gemini CLI를 실행하여 다음과 같이 질문해봤다.

```
현재 블로그 작성 마크다운 파일을 기반으로 포트폴리오에 작성할만한 내 역량을 리스트 형식으로 알려줘
```

이에 대한 답변이 너무 길어서 다시 한 번 이렇게 요청해봤다.

```
너가 정리한 것을 txt 파일로 만들어 줄 수 있어?
```

이후에 사용자 시스템에 쓰기 작업을 실행할 것인가에 대한 허용 여부를 결정할 수 있다.

```
1. Yes, allow once
2. Yes, allow always
3. No, suggest changes (esc)
```

또한 MCP는 내 프로젝트의 파일 구조를 읽어서 적절한 위치에 파일을 생성하기도 한다. 호기심이 들어서 다음과 같이 질문을 해봤다.

```
간단한 테스트 코드에 대한 학습 블로그 내용을 마크다운 파일로 작성해줄 수 있어?
```

어디에 저장하라는 말도 없었지만, Gemini CLI는 내 블로그 프로젝트의 구조를 기반으로 적절한 위치에 파일을 저장해줬다. 좀 더 개발자스럽게 한 번 코드를 수정해보자. 우테코 프리코스 3주차의 로또 발매기의 입력 기능에 대해서 다음과 같은 코드의 중복이 발생했었다.

```java
private LottoCount inputPurchaseAmount() {
    while (true) {
        String purchasePrice = inputView.inputPurchasePrice();
        try {
            return LottoCountParser.parse(purchasePrice);
        } catch (IllegalArgumentException error) {
            outputView.printExceptionMessage(error);
        }
    }
}

private Lotto inputWinningNumbers() {
    while (true) {
        try {
            String winningNumberInput = inputView.inputWinningNumbers();
            return WinningNumberParser.parse(winningNumberInput);
        } catch (IllegalArgumentException error) {
            outputView.printExceptionMessage(error);
        }
    }
}

private WinningLottoInformation inputBonusNumber(Lotto winningLotto) {
    while (true) {
        try {
            String bonusNumberInput = inputView.inputBonusNumber();
            int bonusNumber = BonusNumberParser.parse(bonusNumberInput);
            return WinningLottoInformation.of(winningLotto, bonusNumber);
        } catch (IllegalArgumentException error) {
            outputView.printExceptionMessage(error);
        }
    }
}
```

이 코드의 리팩토링을 Gemini CLI에게 요청해봤다.

```
@src/main/java/lotto/presentation/LottoController.java 이 코드에서 입력 부분이 너무 중복되는데 한 번 수정해줄 수 있어?
```

그리고 다음과 같은 결과를 얻었다.

```java
private LottoCount inputPurchaseAmount() {
    return readInputWithValidation(() -> {
        String purchasePrice = inputView.inputPurchasePrice();
        return LottoCountParser.parse(purchasePrice);
    });
}

private Lotto inputWinningNumbers() {
    return readInputWithValidation(() -> {
        String winningNumberInput = inputView.inputWinningNumbers();
        return WinningNumberParser.parse(winningNumberInput);
    });
}

private WinningLottoInformation inputBonusNumber(Lotto winningLotto) {
    return readInputWithValidation(() -> {
        String bonusNumberInput = inputView.inputBonusNumber();
        int bonusNumber = BonusNumberParser.parse(bonusNumberInput);
        return WinningLottoInformation.of(winningLotto, bonusNumber);
    });
}

private <T> T readInputWithValidation(Supplier<T> supplier) {
    while (true) {
        try {
            return supplier.get();
        } catch (IllegalArgumentException error) {
            outputView.printExceptionMessage(error);
        }
    }
}
```

제네릭과 `Supplier`클래스를 적극적으로 활용해 `LottoController.java`가 깔끔하게 개선된 것을 볼 수 있다. 물론 이 리팩터링은 코드를 복사해 GPT에게 붙여넣어도 얻을 수 있는 결과다.

하지만 MCP를 사용하면 코드를 일일이 복사할 필요 없이, `@`를 사용해 파일 경로만 전달해서 LLM이 실제 파일을 직접 읽고 다른 클래스와의 연관까지 고려해 리팩터링을 수행한다.

즉, GPT 환경에서는 매번 여러 파일을 복사 붙여넣으며 맥락을 전달해야 하지만, MCP를 활용하면 단순히 내 파일 시스템의 경로만 명시하면 되고, 나머지 분석과 수정은 AI가 자동으로 처리한다.

---

MCP가 무엇인지 알아보고, Gemini CLI를 활용해 실제로 어떻게 동작하는지 살펴보았다. 만약 구글 생태계에 관심이 있다면 IntelliJ의 Google Code Assistant 플러그인도 함께 사용해보는 것을 추천한다. 파일 시스템 기반의 리팩터링은 CLI에서 편리하지만, 코드 작성·리뷰·자동 수정 같은 IDE 친화적 작업은 Code Assistant가 더 강하기 때문이다. 두 도구를 함께 활용하면 개발 생산성을 더 크게 높일 수 있다.

요즘 AI를 활용하여 개발 생산성을 높일 수 있는 방법에 대해 관심이 있어서 MCP에 대해 한 번 찾아보았다. AI의 결과물을 검토하기 위한 기술적 기반은 여전히 필요하지만 동시에 학습 과정부터 코드 작성까지 생산성을 높일 수 있다면 AI를 배척하는 것만이 답은 아니라는 생각이 든다.

# 참고자료

[**MCP (모델 컨텍스트 프로토콜)이란 무엇이고 어떻게 작동하는가**](https://blog.logto.io/ko/what-is-mcp)

[**MCP란 무엇이며 어떻게 작동하나요?**](https://cloud.google.com/discover/what-is-model-context-protocol?hl=ko)

[**What is the Model Context Protocol (MCP)?**](https://modelcontextprotocol.io/docs/getting-started/intro)

[**Gemini CLI**](https://docs.cloud.google.com/gemini/docs/codeassist/gemini-cli?hl=ko)

[**google-gemini/gemini-cli**](https://github.com/google-gemini/gemini-cli)

[**[IntelliJ] IntelliJ에서 Gemini Code Assist 사용하기**](https://soonmin.tistory.com/132)
