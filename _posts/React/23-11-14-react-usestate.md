---
title:  "[React] useState"
excerpt: " "

categories:
  - React

toc: true
toc_sticky: true
 
date: 2023-11-14
---

# 실시간 값 변경

어떤 컴포넌트에서 변수의 값을 변경하고 그것을 사용자 화면에 실시간으로 랜더링 할 때 `useState()`를 사용한다.

```js
import React, { useState } from 'react';

function Component(props) {
    // 변수와 변수의 값을 바꿀 함수를 할당
    const [value, method] = useState(props.value);

    const clickHandler = () => {
      method('useState');
    };

    return (
      <button onClick={clickHandler}>Change</button>
      <h2>{value}</h2>
    )
}
```

`useState()`는 현재 변수 값을 저장하는 새로운 변수와 새로운 변수를 변경할 수 있는 메소드를 반환한다. 위 코드는 버튼 클릭 시 전달 받는 값이 useState로 바뀌게 된다. 반환 값에서 변수를 전달받은 값인 `props.value`가 아닌 `value`를 반환해야 한다.

## 여러 값 변경

2개의 값을 `useState()`로 변경하는 경우는 두 번 선언한다.

```js
import React, { useState } from 'react';

const [value, method] = useState('');
const [value1, method1] = useState('');
```

여러 값을 한 번에 받는 `useState()` 문법도 있다.

```js
const [values, method] = useState({
  value: '',
  value1: '',
});
```

이 경우 단일 값 변경 시 기존의 값들은 참조하지 않으므로 변경하지 않는 값도 선언해야 한다. 스프레드 연산자로 기존 값 선언 후 변경 값을 덮어쓰는 방식으로 해결한다.

```js
method({
  ...values,
  value: 'Updated',
});
```

업데이트가 이전 값들에 의존하는 경우 다음과 같이 처리한다. 이런 방식으로 처리하면 리액트가 변경에 대한 정확한 데이터를 보장한다.

```js
method((prevValues) => {
  return {
    ...prevValues,
    value: 'Updated',
  };
});
```