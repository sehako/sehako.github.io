---
title: 우테코 프리코스 - 3주차 과제 회고

categories:
  - Note

toc: true
toc_sticky: true
published: true

date: 2025-11-12
last_modified_at: 2025-11-12
---

정말로 굉장히 늦은 회고지만… 아무튼 프리코스 3주차가 끝났다. 이번 과제는 로또 시뮬레이션을 구현하는 문제였는데, VO를 다루는 것에 초점이 맞춰져 있다고 생각한다. 그 동안 DTO와 VO의 차이점에 대해서 명확히 알지 못했는데 이번 과제를 통해서 어느정도 알게 된 것 같다. 이 부분은 나중에 좀 더 학습해서 정리해두려고 한다.

# 기능 요구 사항

- 로또 번호의 숫자 범위는 1~45까지이다.
- 1개의 로또를 발행할 때 중복되지 않는 6개의 숫자를 뽑는다.
- 당첨 번호 추첨 시 중복되지 않는 숫자 6개와 보너스 번호 1개를 뽑는다.
- 당첨은 1등부터 5등까지 있다. 당첨 기준과 금액은 아래와 같다.
  - 1등: 6개 번호 일치 / 2,000,000,000원
  - 2등: 5개 번호 + 보너스 번호 일치 / 30,000,000원
  - 3등: 5개 번호 일치 / 1,500,000원
  - 4등: 4개 번호 일치 / 50,000원
  - 5등: 3개 번호 일치 / 5,000원
- 로또 구입 금액을 입력하면 구입 금액에 해당하는 만큼 로또를 발행해야 한다.
- 로또 1장의 가격은 1,000원이다.
- 당첨 번호와 보너스 번호를 입력받는다.
- 사용자가 구매한 로또 번호와 당첨 번호를 비교하여 당첨 내역 및 수익률을 출력하고 로또 게임을 종료한다.
- 사용자가 잘못된 값을 입력할 경우 `IllegalArgumentException`을 발생시키고, "[ERROR]"로 시작하는 에러 메시지를 출력 후 그 부분부터 입력을 다시 받는다.
  - `Exception`이 아닌 `IllegalArgumentException`, `IllegalStateException` 등과 같은 명확한 유형을 처리한다.

# 내가 정의한 오류 상황

| 코드 | 상황                                            | 예시                                  | 오류 메시지                                                  |
| ---- | ----------------------------------------------- | ------------------------------------- | ------------------------------------------------------------ |
| E1   | 숫자 이외의 값                                  | `이만원`, `사십`, `일, 2, 3, 4, 5, 6` | [ERROR] 숫자만 입력 가능합니다.                              |
| E2   | 음수                                            | `-1000`, `0`                          | [ERROR] 음수는 입력할 수 없습니다.                           |
| E3   | 0으로 시작하는 숫자                             | `01`, `002`                           | [ERROR] 숫자는 0으로 시작할 수 없습니다.                     |
| E4   | 1 ~ 45 사이가 아닌 숫자                         | `0`, `46`                             | [ERROR] 1에서 45 사이의 숫자만 입력 가능합니다.              |
| M1   | 1000원 단위가 아닌 금액                         | `1200`, `12000`, `12100`, `900`       | [ERROR] 구입금액은 1,000원 단위로 입력해주세요.              |
| M2   | 20억 이상의 금액                                | `2,000,000,001`                       | [ERROR] 구입금액은 최대 20억 원까지 입력 가능합니다.         |
| M3   | -2,147,483,648 ~ 2,147,483,647 범위를 넘은 금액 | `-2,147,483,649`, `2,147,483,648`     | [ERROR] 구매금액은 1000부터 20억 사이의 금액을 입력해주세요. |
| W1   | 당첨 번호가 6개가 아님                          | `1,2,3,4,5,6,7` , `1, 2, 3, 4`        | [ERROR] 로또 번호는 6개여야 합니다.                          |
| W2   | 중복된 당첨 번호                                | `1, 2, 3, 4, 4, 5`                    | [ERROR] 당첨 번호는 중복될 수 없습니다.                      |
| B1   | 당첨 번호와 중복                                | 당첨 번호 입력 존재하는 숫자 입력     | [ERROR] 보너스 번호는 당첨 번호와 중복될 수 없습니다.        |

프리코스를 하면서 점점 오류 상황에 대해서 많은 생각을 하게 되는 것 같다. 1주차 때에는 과제의 구현에만 치중해서 오류 상황을 면밀하게 생각해보지 못했던 것에 비하면 갈수록 오류 상황이 점점 많아지는 것 같다.

# 회고

2주차 회고의 Try에 작성한 객체의 책임, 커밋 범위를 유념하면서 과제를 진행해왔다.

## Keep

**Enum 활용법의 발견**

`LottoRank`와 `ErrorMessage`를 `enum`으로 관리했었다. 이렇게 하니 비즈니스에서 요구하는 조건을 좀 더 깔끔하게 처리할 수 있었다. 예를 들면 오류 메시지 앞에 항상 `[ERROR]`를 붙여야 했다. 이를 일반 상수로 관리하면 항상 하드코드를 해야했지만 `enum`으로 관리하니까 다음과 같이 처리할 수 있었다.

```java
public enum ErrorMessage {
    BONUS_NUMBER_DUPLICATION("보너스 번호는 당첨 번호와 중복될 수 없습니다."),
    ;

    private static final String ERROR_MESSAGE_FORMAT = "[ERROR] %s";

    private final String message;

    ErrorMessage(String message) {
        this.message = message;
    }

    public String getMessage() {
        return String.format(ERROR_MESSAGE_FORMAT, message);
    }
}
```

예외를 던지는 쪽은 단순히 `BONUS_NUMBER_DUPLICATION.getMessage()`를 호출하면 된다.

## Problem

**중복된 코드 작성**

이번 과제에서는 입력이 실패하면 다시 입력을 받도록 해야 했다. 이 과정에서 중복 코드가 발생했다.

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

이는 MCP 관련 포스팅에서도 작성했듯이 `Supplier<T>`를 활용해서 다음과 같이 깔끔하게 만들 수 있다.

```java
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

이 함수를 아래와 같이 호출하면 된다.

```java
private WinningLottoInformation inputBonusNumber(Lotto winningLotto) {
    return readInputWithValidation(() -> {
        String bonusNumberInput = inputView.inputBonusNumber();
        int bonusNumber = BonusNumberParser.parse(bonusNumberInput);
        return WinningLottoInformation.of(winningLotto, bonusNumber);
    });
}
```

그 외에도 파서와 VO의 유효성 검증에 관해서 중복된 코드가 있었다. 나는 기능 구현 이후의 리팩터링에 대해서 많이 약한건가 생각이 들었다.

**Set을 활용한 검증**

나는 로또 번호가 6개인지 검사하기 위해서 `Set` 자료구조를 활용했다. `List`를 `Set`으로 바꿔서 검사한 것이다.

```java
private void validateLottoNumberUnique(List<Integer> numbers) {
    if (new HashSet<>(numbers).size() != numbers.size()) {
        throw new IllegalArgumentException(LOTTO_NUMBER_DUPLICATION.getMessage());
    }
}
```

하지만 이를 `Stream`을 활용하면 다음과 같이 만들 수 있었다.

```java
private void validateLottoNumberUnique(List<Integer> numbers) {
    if (numbers.stream().distinct().count() != numbers.size()) {
        throw new IllegalArgumentException(LOTTO_NUMBER_DUPLICATION.getMessage());
    }
}
```

`Stream`에 대해서 좀 더 알아봐야 겠다는 생각이 들었다.

**컨트롤러의 책임 범위**

현재는 컨트롤러가 `LottoMachine`과 `WinningStatistics`를 호출하고 있다. 나는 두 객체 모두 서비스라고 생각하고 처리했었지만, 생각해보니 컨트롤러가 너무 많은 책임 범위를 가지게 되었다고 생각했다. 이를 해결하려면 `LottoService`로 통합해서 처리해야 하는건지 고민하게 되었다.

## Try

**글을 읽자!**

이게 무슨 말인가 싶지만 과제 종료 이후에 어느순간 모던 자바 인 액션을 잠시 읽게 되었다. 챕터 3 정도 읽었을 때 동작 파라미터화를 통해서 메서드를 전달한다는 것을 알게 되었다. 물론 알고 있었지만, 단지 코드를 보고 ‘동작 파라미터를 전달했구나’ 해석하고 넘겨와서 그런지 머리에 내가 만드는 메서드도 인자를 넘길 수 있다는 생각이 없었던 것 같다. 추가로 `Stream`을 활용한 중복 처리도 이에 관련한 개념이 부족해서 일어난 일인 것 같다는 생각을 하게 되었다. 앞으로 하루에 한 챕터라도 기술서를 읽어야겠다.

---

사실 과제가 끝나고 코드리뷰를 적극적으로 해서 2일 이전에는 회고를 진행했어야 했는데… 오픈 미션 관련해서 이런저런 생각을 하다보니 3주차 회고를 뒤로 미루게 되었고 전체적으로 회고 질이 많이 떨어졌다.
