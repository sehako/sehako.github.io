---
title: 백트래킹

categories:
  - Algorithm

toc: true
toc_sticky: true
published: true
 
date: 2024-12-19
---

완전 탐색은 모든 경우의 수를 탐색한다는 점에서 비효율적일 수 있다. 백트래킹은 이러한 단점을 해결하기 위해 고안된 DFS 기반의 탐색 방법론으로, DFS의 시간을 줄이기 위한 모든 최적화 기법을 포함한다.

다시 말해, 백트래킹은 유망하지 않은 경로를 탐색하지 않도록 가지치기, 메모이제이션, 상태 복원 등의 기법을 활용하여 구현된다. 몇 가지 방법을 소개하면 다음과 같다.

- 가지치기

유망하지 않은 경로를 조기에 배제하여 탐색 공간을 줄이는 방법으로 탐색 중간에 현재 경로가 유망하지 않으면 탐색하지 않는 방법이다. N-Queen 문제에서 모든 경우를 탐색하는 것은 시간이 오래 걸리기 때문에 가능한 경로만 방문하도록 가지치기를 한다.

- 메모이제이션

동일한 경로를 반복 방문하는 것을 방지하기 위해서 이미 계산된 결과를 저장하여 재활용 하는 방법이다. 주로 DP 문제에서 자주 활용된다.

- 상태 복원

탐색 과정에서 변경된 상태를 원래 상태로 복원하여 다음 경로를 탐색할 수 있도록 하는 기법이다. 어떤 경로의 방문 처리 이후 재귀 호출이 끝나면 방문 처리를 해제하는 등의 기법이다.

- 탐색 순서 최적화

탐색 순서를 조정하여 유망한 경로를 우선 탐색하도록 하는 기법이다. 예를 들어, 외판원 순회 문제(TSP)에서는 가중치가 낮은 경로부터 탐색하여 목표를 빠르게 달성하도록 할 수 있다. 이를 통해 가지치기의 효율도 증가한다.


- 비트 마스킹

경로의 방문 처리 등을 비트로 표현하는 방법으로 하나의 메모리 공간에서 비트를 이용하여 방문 처리를 하고, 접근도 기본 타입 변수에 대한 접근이므로 속도나 메모리 효율에서 큰 이점을 가질 수 있다.

**N - Queen**

백트래킹을 설명하기 위해서는 N-Queen 문제를 빼놓을 수 없다. 이 문제는 DFS로 해결하는 문제인데, DFS 그대로 구현하면 시간 초과가 발생한다. 그 이유는 해당 퀸을 놓을 수 없는 경우에도 우선 놓은 다음에 다음 경우의 수를 탐색하기 때문이다. 따라서 퀸을 놓는 위치에 대해서 다른 퀸과의 경로를 살펴서 가능하면 해당 위치에 퀸을 놓고, 아니면 다음 위치로 넘어가도록 구현하는 것이다.

{% include code-header.html %}
```java
static void dfs(int row) {
    if (row == N) {  // 모든 퀸을 배치 완료
        count++;
        return;
    }
    for (int col = 0; col < N; col++) {
        if (isPossible(row, col)) {
            check[row] = col;
            right[(row - col) + N] = true;
            left[col + row] = true;
            
            dfs(row + 1);  // 다음 행으로 이동

            // 상태 복원
            check[row] = 0;
            right[(row - col) + N] = false;
            left[col + row] = false;
        }
    }
}
// 가지치기를 위한 메소드
// 다른 퀸의 경로를 계산해서 겹치면 false를, 겹치지 않으면 true를 반환
static boolean isPossible(int row, int col) {
    for (int i = row - 1; i >= 0; i--) if (check[i] == col) return false;
    
    if (right[(row - col) + N]) return false;
    if (left[col + row]) return false;
    
    return true;
}
```

이런 가지치기를 통해 퀸을 놓을 수 있는 경우를 효율적으로 도출해낼 수 있다.

---

간단하게 백 트래킹에 대해서 정리해보았다. 사실 다양한 기법들을 혼용해서 사용하는 경우도 있기 때문에 DFS의 불필요한 경로 탐색을 줄이기 위해 사용하는 모든 방법을 백트래킹이라고만 이해하면 될 것 같다. 예를 들어, 유망하지 않은 경로를 쳐내는 가지치기와 현재 퀸의 위치를 기록하는 메모이제이션을 결합하여 N-Queen 문제에서 불필요한 경로를 탐색하지 않으면서, 동일한 상태를 중복 계산하지 않도록 최적화 한 것 처럼 말이다. 이러한 기법 혼용은 복잡한 문제에서 백트래킹의 효율성을 극대화한다.
