---
title:  "[React] 컴포넌트와 props"
excerpt: " "

categories:
  - React

toc: true
toc_sticky: true
 
date: 2023-11-13
---

# 컴포넌트

재사용 가능한 조각이다. 리액트는 수 많은 컴포넌트를 작성하고 불러와 UI를 구성한 뒤 최종 컴포넌트를 **App.js**에 전달하고 **index.js**를 통해 사용자 화면에 최종 랜더링 하는 방식이 기본이다.

컴포넌트는 js의 `function` 키워드로 선언한다. 컴포넌트 내부 함수를 외부에서 사용하고자 한다면 `export`로 컴포넌트를 내보낸다.

```js
function Component() {

}

export default Component;
```

화살표 함수로 컴포넌트를 구성할 수 있다.

```js
const Component = () => {

}

export default Component;
```

# props

컴포넌트가 어떤 값을 전달받아 처리를 수행할 때 사용하는 매개변수다. **App.js**의 데이터를 **Component.js**에서 처리하여 반환하는 컴포넌트는 다음과 같다.

```js
//App.js

import Component from './Component';

const App = () => {
  const a_list = [
    { id: 1, title: '1', amount: 1},
    { id: 2, title: '2', amount: 2},
    { id: 3, title: '3', amount: 3},
    { id: 4, title: '4', amount: 4},
  ];
  return (
    <div>
      <Component items={a_list} />
    </div>
  );
}

export default App;
```

```js
//Component.js
function Component(props) {
    return (
        <div>
            <h2>{props.items[0].title}</h2>
            <div>{props.items[0].amount}</div>
        </div>
    );
}

export default Component;
```

`props` 대신 사용할 변수를 지정할 수도 있다. 이 코드에서는 전달할 데이터가 리스트 형식이므로 **App.js**에서 요소를 하나하나 전달하도록 수정해야 한다.

```js
//App.js
<Component 
title={a_list[0].title}
amount={a_list[0].amount}
/>

//Component.js
function Component({ title, amount }) {
    return (
        <div>
            <h2>{title}</h2>
            <div>{amount}</div>
        </div>
    )
}
```

## map을 이용한 배열 요소 랜더링

위와 같은 방법은 같은 코드를 배열 원소의 개수만큼 반복해야 한다. 리스트 자료형을 대상으로 `map` 메소드를 이용하여 다음과 같이 개선할 수 있다.

```js
//App.js

import Component from './Component';

const App = () => {
  const a_list = [
    { id: 1, title: '1', amount: 1},
    { id: 2, title: '2', amount: 2},
    { id: 3, title: '3', amount: 3},
    { id: 4, title: '4', amount: 4},
  ];
  return (
    <div>
    {a_list.map(element => {
        <Component 
        title={element.title}
        amount={element.amount}/>
    })}
    </div>
  );
}

export default App;
```

## 하위-상위 컴포넌트 간 값 전달

**App.js** -> **Component1.js** -> **Component2.js** 순서로 컴포넌트를 호출한다고 할 때 함수를 이용하여 하위 컴포넌트에서 상위 컴포넌트로 값을 전달할 수 있다.

```js
// App.js
import Component1 from './Component1'

function App() {
  function valuePass(value) {
    // 값 처리
  }

  return (
    <Component1 pass={valuePass} />
  );
}
```

```js
import Component2 from './Component2'

function Component1(props) {
  function valuePass1(value) {
    // 값 처리
    props.pass(value);
  }

  return (
    <Component2 pass1={valuePass1}/>
  );
}

export default Component1;
```

```js
function Component2(props) {
  const num = 0;
  function valuePass2(value) {
    // 값 처리
    props.pass1(num);
  }

  return (
  );
}

export default Component2;
```