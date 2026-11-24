---
title: 인터프리터 패턴 (구조와 규칙 엔진 구현)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-12-01

permalink: "/object-oriented-design/interpreter-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 행동 패턴 편입니다. 이번에는 인터프리터 패턴을 다룹니다. 모니터링 경고 규칙 엔진 예시로 구조와 구현 방법을 살펴보고, 인터프리터 패턴의 장단점과 도입 기준을 정리하겠습니다.

# 인터프리터 패턴이란

특정 언어의 문법 규칙을 정의하고, 그 문법에 따라 작성된 문장(표현식)을 해석하는 인터프리터를 구축하는 행동 디자인 패턴입니다.

여기서 언어는 자연어뿐 아니라 프로그래밍 언어, 산술 수식, 정규식, 모니터링 경고 규칙처럼 정보와 규칙을 전달하는 모든 형식 언어를 뜻합니다.

# 활용 상황

인터프리터 패턴은 주로 다음 상황에서 사용됩니다.

- 특정 도메인의 간단한 미니 언어(DSL)나 표현식 평가기를 직접 구축해야 하는 경우

복잡한 비즈니스 룰, 사용자 정의 수식 계산기, 정규식 매칭처럼 프로그램 내부에서 문장을 해석하고 실행해야 할 때 사용합니다. 문법 규칙을 클래스 계층 구조로 매핑하면 파싱 로직을 한 곳에 몰아넣지 않고 체계적으로 나눌 수 있습니다.

- 규칙의 조건이나 연산자 조합이 런타임에 바뀌거나 추가되는 규칙 엔진을 만드는 경우

모니터링 알림 임계치, 결제 할인 조건, 이상 거래 탐지 규칙처럼 관리자가 조건을 문자열로 입력하고 시스템이 이를 실시간으로 평가해야 할 때 적합합니다. 새 규칙이 들어와도 기존 코드를 바꾸지 않고 표현식 객체의 조합만 바꾸면 됩니다.

- 문법이 비교적 단순하고 자주 확장되지 않는 경우

문법이 복잡해지면 표현식 클래스가 너무 많아져 관리하기 어렵습니다. 문법 자체는 단순하면서 다양한 데이터에 반복해서 적용되는 시나리오에 알맞습니다.

# 구조

인터프리터 패턴은 복합체 패턴과 비슷하게 표현식을 트리로 조합한 추상 구문 트리(AST)를 형성합니다. 구조는 다음과 같습니다.

- 추상 표현식(Abstract Expression)은 구문 트리의 모든 노드가 공통으로 구현하는 인터페이스입니다. 해석을 수행하는 `interpret(Context)` 메서드를 선언합니다.
- 종단 표현식(Terminal Expression)은 문법에서 더 이상 쪼갤 수 없는 기본 요소입니다. 숫자, 변수, 상수 값을 비교하는 `key > value` 같은 표현식이 예입니다.
- 비종단 표현식(Non-Terminal Expression)은 다른 표현식을 자식 노드로 가지는 복합 요소입니다. `interpret()`가 호출되면 자식의 `interpret()`를 재귀적으로 호출해 결과를 조합합니다. 사칙연산이나 `And`, `Or` 같은 논리 연산이 예입니다.
- 콘텍스트(Context)는 해석에 필요한 전역 정보나 실행 환경 데이터를 담아 전달합니다. 변수 이름과 값을 매핑한 `Map<String, Long>`이 예입니다.
- 클라이언트는 입력된 표현식 문자열을 파싱해 구상 표현식으로 구문 트리를 조립하고, 최상위 노드의 `interpret()`를 호출해 결과를 얻습니다.

## 구현 방법

시스템 모니터링의 경고 규칙 엔진 예시입니다. 개발자가 `key1 > 100 && key2 < 30 || key3 < 100 && key4 > 80` 같은 문자열로 규칙을 정의하면, 이를 구문 트리로 파싱한 뒤 실시간 통계 데이터(`Map<String, Long> stats`)를 넘겨 경고 여부를 판별합니다. 문법은 `&&`가 `||`보다 먼저 묶이는 비교식의 조합이며, 비교 연산자는 `>`, `<` 두 가지입니다.

`Expression`은 추상 표현식입니다. 콘텍스트인 `stats`를 받아 규칙의 참과 거짓을 평가합니다.

{% include code-header.html %}
```java
public interface Expression {
    boolean interpret(Map<String, Long> stats);
}
```

`GreaterExpression`, `LessExpression`은 종단 표현식입니다. `key > value` 같은 비교식 하나를 맡으며, 더 이상 하위 표현식으로 쪼개지 않습니다. 생성자가 문자열을 `키 연산자 값` 세 토큰으로 나누어 검증합니다. 통계에 키가 없으면 `false`를 반환합니다. 두 클래스는 비교 연산자만 다릅니다.

{% include code-header.html %}
```java
public class GreaterExpression implements Expression {
    private final String key;
    private final long value;

    public GreaterExpression(String strExpression) {
        String[] elements = strExpression.trim().split("\\s+");
        if (elements.length != 3 || !elements[1].equals(">")) {
            throw new IllegalArgumentException("Expression is invalid: " + strExpression);
        }
        this.key = elements[0];
        this.value = Long.parseLong(elements[2]);
    }

    @Override
    public boolean interpret(Map<String, Long> stats) {
        if (!stats.containsKey(key)) {
            return false;
        }
        return stats.get(key) > value;
    }
}

public class LessExpression implements Expression {
    private final String key;
    private final long value;

    public LessExpression(String strExpression) {
        String[] elements = strExpression.trim().split("\\s+");
        if (elements.length != 3 || !elements[1].equals("<")) {
            throw new IllegalArgumentException("Expression is invalid: " + strExpression);
        }
        this.key = elements[0];
        this.value = Long.parseLong(elements[2]);
    }

    @Override
    public boolean interpret(Map<String, Long> stats) {
        if (!stats.containsKey(key)) {
            return false;
        }
        return stats.get(key) < value;
    }
}
```

`AndExpression`과 `OrExpression`은 비종단 표현식입니다. 하위 표현식을 리스트로 가지고, `interpret()`에서 자식의 `interpret()`를 재귀적으로 호출해 결과를 조합합니다.

`AndExpression`은 `&&`로 문자열을 나누어 비교식마다 알맞은 종단 표현식을 만들고, `OrExpression`은 `||`로 나누어 조각마다 `AndExpression`을 만듭니다. 우선순위가 높은 `&&`가 트리의 아래쪽에 놓이므로 `&&`가 `||`보다 먼저 평가됩니다.

{% include code-header.html %}
```java
public class AndExpression implements Expression {
    private final List<Expression> expressions = new ArrayList<>();

    public AndExpression(String strAndExpression) {
        for (String strExpr : strAndExpression.split("&&")) {
            if (strExpr.contains(">")) {
                expressions.add(new GreaterExpression(strExpr));
            } else if (strExpr.contains("<")) {
                expressions.add(new LessExpression(strExpr));
            } else {
                throw new IllegalArgumentException("Expression is invalid: " + strAndExpression);
            }
        }
    }

    @Override
    public boolean interpret(Map<String, Long> stats) {
        for (Expression expr : expressions) {
            if (!expr.interpret(stats)) {
                return false;
            }
        }
        return true;
    }
}

public class OrExpression implements Expression {
    private final List<Expression> expressions = new ArrayList<>();

    public OrExpression(String strOrExpression) {
        for (String andExpr : strOrExpression.split("\\|\\|")) {
            expressions.add(new AndExpression(andExpr));
        }
    }

    @Override
    public boolean interpret(Map<String, Long> stats) {
        for (Expression expr : expressions) {
            if (expr.interpret(stats)) {
                return true;
            }
        }
        return false;
    }
}
```

`AlertRuleInterpreter`는 외부에 노출되는 진입점입니다. 규칙 문자열을 받아 가장 낮은 우선순위의 `||`를 루트로 삼아 구문 트리를 조립하고, `interpret()`은 루트에게 평가를 위임합니다. 구조에서 말한 클라이언트의 파싱과 조립 역할을 이 클래스가 맡습니다.

{% include code-header.html %}
```java
public class AlertRuleInterpreter {
    private final Expression expression;

    public AlertRuleInterpreter(String ruleExpression) {
        this.expression = new OrExpression(ruleExpression);
    }

    public boolean interpret(Map<String, Long> stats) {
        return expression.interpret(stats);
    }
}
```

사용하는 쪽은 규칙 문자열로 인터프리터를 만들고, 통계 데이터를 넘겨 평가합니다.

{% include code-header.html %}
```java
public class DemoTest {
    public static void main(String[] args) {
        String rule = "key1 > 100 && key2 < 30 || key3 < 100 && key4 > 80";
        AlertRuleInterpreter interpreter = new AlertRuleInterpreter(rule);

        Map<String, Long> stats = new HashMap<>();
        stats.put("key1", 101L);
        stats.put("key2", 10L);
        stats.put("key3", 12L);
        stats.put("key4", 88L);

        boolean alert = interpreter.interpret(stats); // true
    }
}
```

`key1 > 100 && key2 < 30`이 참이므로 `OrExpression`은 첫 `AndExpression`에서 바로 `true`를 반환합니다.

`>=`나 `<=` 같은 새 비교 연산자는 `GteExpression`, `LteExpression` 클래스를 추가하고 `AndExpression`의 분기에 연결하면 됩니다. 이때 `>=`가 `>`를 포함하므로 `contains(">")` 분기보다 먼저 검사해야 합니다. 규칙 문자열을 파싱하는 로직이 `AndExpression`과 `OrExpression` 생성자에 들어 있어서, 문법이 복잡해지면 파싱을 별도 클래스로 분리해야 합니다.

# 장단점

인터프리터 패턴의 장단점은 다음과 같습니다.

**장점**

- **개방-폐쇄 원칙(OCP)과 단일 책임 원칙(SRP) 준수**: 새로운 연산자나 문법 요소는 기존 코드를 수정하지 않고 새 표현식 클래스만 추가하면 됩니다. 각 표현식은 문법의 작은 단위 하나만 책임집니다.
- **문법의 직관적인 클래스화**: 복잡한 조건식을 작은 단위 객체로 나누므로 재사용하기 쉽고 단위별로 테스트하기 좋습니다.

**단점**

- **복잡한 문법에서의 클래스 증가**: 문법 규칙이 늘어날수록 정의해야 하는 표현식 클래스가 많아져 시스템이 비대해집니다.
- **해석 성능 부담**: 구문 트리를 재귀적으로 순회하며 평가하므로 중첩이 깊으면 호출 스택 부담과 실행 속도 저하가 생길 수 있습니다.
- **과도한 설계의 위험**: 단순한 사칙연산 수준의 처리는 스택 기반의 간단한 반복문으로도 충분히 해결할 수 있으므로, 무조건 패턴을 적용하면 오버엔지니어링이 됩니다.

---

정리하면, 인터프리터는 단순한 문법으로 표현된 규칙이나 수식을 런타임에 해석해야 할 때 문법 규칙을 표현식 클래스로 분리하는 패턴입니다. 문법이 복잡하거나 자주 확장된다면 피하고, 사칙연산 수준의 처리라면 스택 기반 반복문으로 충분한지 먼저 따져 봅니다.

# 참고 자료

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
