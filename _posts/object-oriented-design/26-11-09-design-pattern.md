---
title: 디자인 패턴이란 무엇이고 어떻게 분류하는가
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-09

permalink: "/object-oriented-design/design-pattern/"
---

디자인 패턴은 객체 지향 소프트웨어를 설계하고 개발하는 과정에서 자주 발생하는 문제에 대한 정형화된 해결책입니다.

현실적으로 디자인 패턴을 몰라도 프로그래밍은 가능합니다. 하지만 디자인 패턴을 배우면 소프트웨어 설계의 일반적인 문제에 대해 시도되고 검증된 해결책을 알게 되고, 팀원과 효율적으로 의사소통할 때 쓸 공통 언어도 얻게 됩니다. 그래서 배울 가치가 충분합니다.

이 글은 디자인 패턴 시리즈의 개요입니다. 패턴 정리 글을 올릴 때마다 아래 [패턴 목록](#패턴-목록)에 링크를 추가합니다.

# 디자인 패턴에 대한 비판

디자인 패턴은 다음과 같은 이유로 비판받기도 합니다.

- 약한 프로그래밍 언어를 위한 엉성한 해결책이다.

패턴이 필요해지는 이유는 추상화 수준이 부족한 프로그래밍 언어나 기술을 선택했기 때문이라는 주장입니다. 이 경우 패턴은 클루지(엉성한 해결책이지만 효과적인 무언가)로 작동합니다. 실제로 전략(Strategy) 패턴은 최신 프로그래밍 언어에서 람다 함수로 구현할 수 있습니다.

- 비효율적인 해결책이다.

많은 사람이 통합된 패턴을 신봉해서, 프로젝트 맥락에 따라 적용하지 않고 문자 그대로 구현합니다.

- 오남용의 여지가 있다.

초보 프로그래머는 패턴을 배운 뒤 간단한 코드로도 해결되는 상황에 패턴을 도입하려고 합니다.

> 망치만 있으면 모든 것이 못처럼 보인다

그래서 이 시리즈에서는 패턴의 구현 방법뿐 아니라 언제 쓰고 언제 쓰지 말아야 하는지, 단점은 무엇인지도 함께 정리합니다.

# 패턴의 분류

디자인 패턴은 복잡성, 상세도, 전체 시스템에 대한 적용 범위에 따라 생성, 구조, 행동 패턴으로 나눕니다.

- 생성 패턴: 기존 코드의 재활용과 유연성을 증가시킵니다.
- 구조 패턴: 구조를 유연하고 효율적으로 유지하면서 객체와 클래스를 더 큰 구조로 조합합니다.
- 행동 패턴: 객체 간 효과적인 의사소통과 책임 할당을 처리합니다.

# 패턴 목록

## 생성 패턴

- [싱글턴 패턴 (Java 구현 방법과 Spring 싱글톤 빈과의 차이)](/object-oriented-design/singleton-pattern/)
- [팩토리 메서드 패턴 (구조와 단순 팩토리와의 차이)](/object-oriented-design/factory-method-pattern/)
- [추상 팩토리 패턴 (구조와 DI 컨테이너와의 차이)](/object-oriented-design/abstract-factory-pattern/)
- [빌더 패턴 (GoF 방식과 자바 플루언트 빌더)](/object-oriented-design/builder-pattern/)
- [프로토타입 패턴 (복사 생성자와 Cloneable)](/object-oriented-design/prototype-pattern/)

## 구조 패턴

- [프록시 패턴 (구조와 스프링 AOP 프록시)](/object-oriented-design/proxy-pattern/)
- [데코레이터 패턴 (구조와 프록시 패턴과의 차이)](/object-oriented-design/decorator-pattern/)

## 행동 패턴

아직 정리한 패턴이 없습니다.

# 참고 자료

[**Refactoring.Guru - 디자인 패턴**](https://refactoring.guru/ko/design-patterns)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
