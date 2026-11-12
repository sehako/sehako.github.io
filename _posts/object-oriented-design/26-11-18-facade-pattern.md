---
title: 퍼사드 패턴 (구조와 전지전능한 객체 문제)
categories:
  - 객체지향 설계
toc: true
toc_sticky: true
published: true
date: 2026-11-18

permalink: "/object-oriented-design/facade-pattern/"
---

[디자인 패턴](/object-oriented-design/design-pattern/) 시리즈의 구조 패턴 편입니다. 이번에는 퍼사드를 다룹니다. 구조와 비디오 변환 프레임워크 예제를 살펴본 뒤, 퍼사드가 비대해졌을 때 생기는 전지전능한 객체 문제와 추가 퍼사드로 이를 나누는 방법을 정리하겠습니다.

# 퍼사드 패턴이란

복잡한 하위 시스템(라이브러리, 프레임워크 등)에 대해서 단순화된 고수준의 단일 창구 인터페이스를 제공하는 구조 디자인 패턴입니다.

주로 다음 상황에서 사용합니다.

- 복잡한 하위 시스템에 대해 제한적이지만 간단한 인터페이스가 필요한 경우

라이브러리나 프레임워크는 시간이 지날수록 범용성을 고려하기 때문에 기능과 설정이 복잡해집니다. 클라이언트 코드가 이러한 하위 시스템에 직접 의존하게 되면 변경 범위가 클라이언트에게도 전파되므로, 이 둘 사이에 퍼사드를 제공하여 클라이언트가 자주 쓰는 기능 중심의 단순한 인터페이스로 분리합니다.

- 하위 시스템을 계층으로 관리하려는 경우

하위 시스템(계층) 간의 통신에 퍼사드를 진입점(창구)으로 제공함으로써, 각 계층의 내부 구현 변경이 다른 계층으로 번지는 영향을 최소화하고 결합도를 낮출 수 있습니다.

# 구조

퍼사드 패턴은 다음과 같은 구조를 가집니다.

- 퍼사드는 하위 시스템의 기능 중 클라이언트에게 필요한 부분만 골라 간단한 인터페이스로 제공합니다.
- 하위 시스템은 수십 개의 클래스로 구성될 수 있으며, 퍼사드의 존재를 모릅니다.
- 클라이언트는 하위 시스템 객체 대신 퍼사드를 호출합니다.
- 퍼사드가 비대해지면 일부 기능을 추가 퍼사드 클래스로 분리할 수 있습니다.

# 구현 예시

복잡한 타사 비디오 변환 프레임워크와의 상호작용을 단순화하는 예시입니다. 프레임워크 코드는 직접 제어할 수 없으므로 단순화할 수 없습니다. 대신 퍼사드 클래스를 만들어 프레임워크의 복잡성을 간단한 인터페이스 뒤에 숨깁니다.

다음은 프레임워크 클래스의 일부입니다. 수정할 수 없다고 가정합니다.

{% include code-header.html %}
```java
// 복잡한 타사 비디오 변환 프레임워크 클래스의 일부
public class VideoFile {
    // ...
}

public interface CompressionCodec {
    // ...
}

public class OggCompressionCodec implements CompressionCodec {
    // ...
}

public class MPEG4CompressionCodec implements CompressionCodec {
    // ...
}

public class CodecFactory {
    public CompressionCodec extract(VideoFile file) {
        // ...
    }
}

public class BitrateReader {
    public static Buffer read(String filename, CompressionCodec codec) {
        // ...
    }

    public static Buffer convert(Buffer buffer, CompressionCodec codec) {
        // ...
    }
}

public class AudioMixer {
    public Buffer fix(Buffer result) {
        // ...
    }
}
```

퍼사드는 이 클래스들을 조합하는 순서와 방법을 알고 있으며, 클라이언트에게는 `convert()` 하나만 노출합니다. 기능성과 단순함을 맞바꾸는 것이 아니라 서로 보완하도록 필요한 기능만 제공합니다.

{% include code-header.html %}
```java
// 퍼사드: 프레임워크의 복잡성을 간단한 인터페이스 뒤에 숨긴다.
public class VideoConverter {
    public File convert(String filename, String format) {
        VideoFile file = new VideoFile(filename);
        CompressionCodec sourceCodec = new CodecFactory().extract(file);

        CompressionCodec destinationCodec;
        if (format.equals("mp4")) {
            destinationCodec = new MPEG4CompressionCodec();
        } else {
            destinationCodec = new OggCompressionCodec();
        }

        Buffer buffer = BitrateReader.read(filename, sourceCodec);
        Buffer result = BitrateReader.convert(buffer, destinationCodec);
        result = new AudioMixer().fix(result);
        return new File(result);
    }
}
```

애플리케이션 클래스는 프레임워크의 수많은 클래스에 의존하지 않습니다. 프레임워크를 업그레이드하거나 다른 프레임워크로 교체하더라도 퍼사드의 구현만 다시 작성하면 됩니다.

{% include code-header.html %}
```java
// 클라이언트 코드
public class Application {
    public static void main(String[] args) {
        VideoConverter converter = new VideoConverter();
        File mp4 = converter.convert("funny-cats-video.ogg", "mp4");
        mp4.save();
    }
}
```

# 장단점

퍼사드 패턴의 장단점은 다음과 같습니다.

**장점**

- 복잡한 하위 시스템에서 코드를 별도로 분리할 수 있습니다.

**단점**

- 앱의 모든 클래스에 결합된 전지전능한 객체(God Object)가 될 수 있습니다.

# 전지전능한 객체 문제

퍼사드는 클라이언트 코드에게 편의성을 제공하는 목적으로 만들어졌기 때문에, 여러 클라이언트 코드를 고려해서 하나의 퍼사드에 계약을 추가하다 보면 시스템의 모든 클라이언트 코드가 거대한 퍼사드 하나에만 의존하게 됩니다.

이 경우 퍼사드에 작은 변경이 생겼을 때, 그 영향이 시스템 전체로 퍼질 수 있습니다. 따라서 퍼사드를 설계할 때 단일 책임 원칙과 인터페이스 분리 원칙의 관점으로 추가 퍼사드(Additional Facade) 도입을 고려해야 합니다.

예를 들어 영상과 소리를 변환하는 애플리케이션에 `ConverterFacade`를 도입하는 것보다는, 변경 요구사항의 주체(책임)에 따라서 `VideoConverterFacade`, `AudioConverterFacade`를 두고, 클라이언트에서는 자신이 사용하는 기능에 따라서 해당 퍼사드에 의존하게 만드는 것입니다.

---

지금까지 살펴본 내용을 기준으로 퍼사드를 도입할지 판단하는 신호를 표로 나타내면 다음과 같습니다.

| 상황                                                       | 판단          | 근거                                                                    |
| -------------------------------------------------------- | ----------- | --------------------------------------------------------------------- |
| 복잡한 하위 시스템을 자주 쓰는 기능 중심으로 간단하게 사용하고 싶다                   | 쓴다          | 퍼사드가 하위 시스템 객체의 조합 순서와 방법을 숨기고 필요한 기능만 단순한 인터페이스로 제공한다               |
| 라이브러리나 프레임워크를 업그레이드하거나 교체할 가능성이 있다                       | 쓴다          | 클라이언트는 퍼사드에만 의존하므로 퍼사드의 구현만 다시 작성하면 된다                                |
| 하위 시스템(계층) 간 통신 창구가 필요하다                                 | 쓴다          | 퍼사드를 진입점으로 두면 각 계층의 내부 구현 변경이 다른 계층으로 번지는 영향을 줄이고 결합도를 낮출 수 있다        |
| 여러 클라이언트의 요구로 퍼사드에 기능이 계속 쌓이고 있다                         | 추가 퍼사드로 분리한다 | 변경 요구사항의 주체(책임)별로 퍼사드를 나누고 클라이언트가 사용하는 퍼사드에만 의존하게 하여 전지전능한 객체를 막는다   |
| 호환되지 않는 인터페이스를 클라이언트가 기대하는 인터페이스로 맞춰야 한다                 | 어댑터를 검토한다   | 퍼사드는 하위 시스템 앞에 새로운 단순 인터페이스를 제공하고, 어댑터는 기존 인터페이스를 호환되도록 변환한다          |
| 하위 시스템이 이미 단순하거나 클라이언트가 하위 시스템의 모든 기능을 세밀하게 제어해야 한다      | 피한다         | 퍼사드는 필요한 기능만 골라 제공하므로 단순화하는 이점은 없고 계층만 늘어난다                          |

정리하면, 퍼사드는 복잡한 하위 시스템 앞에 단순한 창구를 두어 클라이언트와 하위 시스템 사이의 결합을 끊는 패턴입니다. 다만 창구 하나에 모든 요구를 모으면 그 창구가 새로운 의존의 중심이 되므로, 변경 이유가 다른 기능은 퍼사드를 나눠서 관리해야 합니다.

# 참고 자료

[**퍼사드 패턴 - Refactoring.Guru**](https://refactoring.guru/ko/design-patterns/facade)

[**디자인 패턴의 아름다움**](https://product.kyobobook.co.kr/detail/S000202093794)
