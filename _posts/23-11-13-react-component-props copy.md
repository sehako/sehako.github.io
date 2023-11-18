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
```

```js
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

## map을 이용한 동적 배열 랜더링

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
        key={element.id}
        title={element.title}
        amount={element.amount}/>
    })}
    </div>
  );
}

export default App;
```

### 키 

리액트는 배열에 새로운 요소가 생기면 목록의 마지막에 랜더링 하고 모든 요소를 업데이트 하여 컨텐츠를 반영한다. 모든 목록을 업데이트 하는 것은 비효율적이다. 또한 컴포넌트 내 `useState()`를 사용하는 변수가 있다면, 새로운 요소가 이전의 요소가 가지고 있던 상태를 덮어쓸 수도 있다.

이런 문제에 대한 해결책으로 `key` 매개 변수가 사용된다. `key`값은 HTML요소에 내장된 것을 포함한 모든 컴포넌트에서 사용할 수 매개변수다. 키 값은 항상 고유의 값을 가져야 한다. 키 값이 없다면 `map()` 메소드에서 기본적으로 지원해주는 인덱스 기능을 사용할 수 있지만 특정한 아이템에 대한 인덱스가 항상 같아 버그를 일으킬 수 있기 때문에 권장되는 방법은 아니다. 따라서 키 값은 데이터 내에서 항상 고유의 정보를 갖도록 해야한다. 위 코드의 경우 `id`값이 키 값으로 사용되었다.

추가로 키 값을 사용하면 리액트에서 요소가 위치해야 할 곳도 인식한다.

## props.children

다음 형태로 사용 및 반환되는 컴포넌트가 있다면

```js
  return (
      <Component1>
          <Component2 />
      </Component1>
  )
```

**Component1** 에서 `props.children`을 통해 **Component2**에 접근할 수 있다.

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