---
title:  "[React] 조건에 따른 랜더링"
excerpt: " "

categories:
  - React

toc: true
toc_sticky: true
 
date: 2023-11-17
---

# 조건부 처리

대부분의 웹은 사용자의 행동에 따라서 다른 결과를 반환한다. 온라인 쇼핑몰 같은 곳에서 장바구니를 예시로 본다면, 장바구니에 이미 추가된 물건을 다시 추가하려 할 때 이미 장바구니에 담겼다는 메시지를 보여주는 경우가 있을 것이다. 이 모든 건 조건에 따라서 처리하는 것이다. 물건이 없다면 물건을 추가하고, 물건이 있다면 메세지를 보여주는 식이다. 리액트에서는 이런 조건문을 구현하는 방법이 두 가지 있다.

## return에서 조건 생성

`return`내에서 조건을 작성하여 처리할 수 있다. 다음 코드는 배열이 0일 때, `<p>` 태그를 반환하고 배열이 0이 아니면 `map()`을 이용하여 배열 전체 내용을 다른 컴포넌트로 전달하고 반환하도록 하는 예시다.

```js
function Component() {
    //...
    return (
        {array.length === 0 ? (
                    <p>Not found.</p>
                ) : (array.map(element => {
                    return (
                        <Component 
                        key = {element.id}
                        value = {element.value}
                        />
                    );
                }))}
)
}
```

참 값에 대한 처리만 필요하다면 `&&`를 사용한다.

```js
// ...
return (
{array.length === 0 && <p>No expenses found.</p>}
)
```

## 조건에 따라 변수에 다른 값 대입

리액트에서는 다음과 같은 조건문 작성도 가능하다.

```js
function Component() {
    let value = '';
    // ...
    if(array.length === 0) {
        value = <p>No expenses found.</p>;
    }
    else {
        value = array.map(element => {
            return (
                <Component1
                key = {element.id}
                value = {element.value}
                >
            )
        })
    }

    return (
        <div>{value}<div>
    )
}
```

위 방법으로 조건부 처리를 구현하면 `return`에서 조건부 처리를 구현하는 것보다 가독성이 좋아보인다.

# useState()와 조건부 랜더링

위 방법은 당연히 리액트에서 제대로 작동하지 않는다. 리액트에서는 변수의 변경이 실시간으로 랜더링되지 않는다. 따라서 변수의 변경이 있으면 컴포넌트를 다시 랜더링하는 `useState()`를 사용해야 한다. 최종적으로 `value` 변수를 `useState()` 형식으로 취하고 그 사이에 요구 사항에 맞는 기능에 따른 처리를 구현한다.

```js
import React, { useState } from 'react';

function Component(props) {
    const [value, setValue] = useState(props.value);
    // ...
    if(array.length === 0) {
        value = <p>No expenses found.</p>;
    }
    else {
        value = array.map(element => {
            return (
                <Component1
                key = {element.id}
                value = {element.value}
                >
            )
        })
    }

    return (
        <div>{value}<div>
    )
}
```