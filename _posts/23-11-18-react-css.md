---
title:  "[React] 리액트와 CSS"
excerpt: " "

categories:
  - React

toc: true
toc_sticky: true
 
date: 2023-11-18
---

CSS는 HTML 요소들을 디자인 할 수 있도록 해주는 코드이다. 리액트에서는 다양한 CSS 활용 방법을 제공한다.

# 리액트와 CSS

css 파일을 가져와 반환 요소에 `className`을 정의하는 것이 가장 단순한 사용법일 것이다.

```js
import './Component.css';

return <div className="css-class"></div>
```

그리고 조건에 따라서 css값을 변경할 수 있다. 다음 코드는 버튼이 클릭되면 텍스트 색상이 바뀌는 코드다.

```js
import React, { useState } from 'react';
import './Component.css';

function Component() {
  const [isRed, setIsRed] = useState(false);
  function clickListener() {
    if(isRed) {
      setIsRed(true)
    }
    else {
      setIsRed(false)
    }
  }

  return (
    <div>
      <h2 style={{color: !isRed && 'red'}}>예시 텍스트</h2>
      <button onClick={clickListener}>버튼</button>
    </div>
  )
}
```

참고로 리액트에서 스타일을 지정할 때는 `{{}}`를 사용하고 키 값은 따옴표를 사용할 필요가 없다.

추가로 css파일 내 `<div>`를 정의하는 기본 클래스가 있고 그 부분이 텍스트가 빨간 색으로 바뀔 때마다 다른 클래스로 바뀌는 기능을 구현하면 다음과 같다.

```js
return (
  <div className={`div-class ${!isRed ? 'isred' : ''}`}>
    <h2 style={{color: !isRed && 'red'}}>예시 텍스트</h2>
    <button onClick={clickListener}>버튼</button>
  </div>
)
```

이 경우 버튼이 클릭되면 `className`의 값이 `div-class isred`가 되고 그 상태에서 버튼을 다시 클릭하면 `div-class`가 된다.

## Styled Component

리액트의 라이브러리 중 [Styled Component](https://styled-components.com/)라는 라이브러리는 js 파일 내 css를 직접 정의할 수 있도록 해준다.

```
npm install styled-components
```

아래 코드는 버튼의 css를 정의한 간단한 예제 코드다.

```js
import styled from 'styled-components';

const Button = styled.button`
  font: inherit;
  padding: 0.5rem 1.5rem;
  border: 1px solid #8b005d;

  &:active {
    background: #ac0e77;
    border-color: #ac0e77;
    box-shadow: 0 0 8px rgba(0, 0, 0, 0.26);
  }
`;
```

이 `styled` 라이브러리는 탬플릿 리터럴로 작동된다. 또한 컴포넌트의 메인 함수 내부가 아닌 바깥에 정의한다.(필수인지 모름) 따라서 변수를 선언하고 `styled.element`이후에 탬플릿 리터럴을 넣고 그곳에 css를 작성한다. 또 기존 css와의 차이점은 클래스 이름이 `&`로 대체된다는 것이다. 위 코드의 원래 css 파일에서 클래스 이름이 `.button`이었다면 그 부분의 내용이 탬플릿 리터럴 내 첫 3줄에 정의된 부분이다. 따라서 `&:button:active`는 원래 css 파일의 작성 규칙 대로라면 `.button:active`이다.

스타일 컴포넌트로 생성된 변수는 말 그대로 컴포넌트 취급이다.

### props

따라서 `props`를 이용하여 직관적으로 css내 값들을 바꿀 수 있다. 위 버튼 스타일 컴포넌트를 사용하는 컴포넌트가 있다고 가정하고 버튼이 클릭될 때마다 `background`의 값이 바뀌는 코드는 다음과 같다.

```js
import React, { useState } from 'react';
import styled from 'styled-components';

  const Button = styled.button`
  font: inherit;
  padding: 0.5rem 1.5rem;
  border: 1px solid #8b005d;

  &:active {
    background: ${props => (props.isred ? 'red' : 'transparent')};
    border-color: #ac0e77;
    box-shadow: 0 0 8px rgba(0, 0, 0, 0.26);
  }
`;

function Component() {
  const [isRed, setIsRed] = useState(false);
  function clickListener() {
    if(isRed) {
      setIsRed(true)
    }
    else {
      setIsRed(false)
    }
  }

  return (
    <div>
      <h2 style={{color: !isRed && 'red'}}>예시 텍스트</h2>
      <Button isred={isRed}>버튼</Button>
    </div>
  )
}
```

## css 모듈

리액트는 css의 중첩 방지를 위해서 css 모듈을 지원한다. 사용 방법은 아래와 같다. 

```js
import styles from './Button.module.css';

const Button = props => {
  return (
    <button className={styles.button}>
    </button>
  );
};
```

정의된 css 파일이 **Button.css**라면 **Button.module.css**로 변경해야 한다. 그 이후 `style.button` 또는 `style['button']`으로 css 파일에 정의된 값을 불러올 수 있다.