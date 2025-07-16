---
title: 서로소 집합 (Disjoint Set & Union-Find)

categories:
  - Algorithm

toc: true
toc_sticky: true
published: true
 
date: 2024-12-27
---

# 서로소 집합

서로소 집합은 두 집합간 교집합이 존재하지 않는 집합을 뜻한다. 이 두 서로소 집합을 이용한 연산으로는 Union-Find 알고리즘이 있으며, 다음과 같은 문제에서 유용하게 사용된다.

- 네트워크 연결 문제
- 최소 신장 트리(크루스칼)
- 동일 그룹 판별

# Union-Find

Union-Find에서는 집합에 속한 하나의 특정 멤버를 통해 각 집합들을 구분하는데, 이를 대표자라고 한다. 이때 서로소 집합을 표현하기 위해서 연결 리스트와 배열을 사용한다. 하지만 연결 리스트를 이용하여 서로소 집합을 구현하면 탐색 및 다양한 최적화를 사용하지 못하므로 트리로 구현하는 것이 사실상 정석이다. Union-Find 알고리즘에서 중요한 연산은 다음과 같다.

- `make()`: 서로소 집합 초기화

{% include code-header.html %}
```java
class DisjointSet {
    private int[] parent;
    private int[] rank;

    public DisjointSet(int size) {
        parent = new int[size];
        rank = new int[size];
        make();
    }

    public void make() {
        // 자신의 부모를 자신으로 하여 대표자가 되도록 함
        for (int i = 0; i < size; i++) {
            parent[i] = i;
            rank[i] = 1;
        }
    }
}
```

`rank` 배열은 각 루트의 트리 높이를 나타내는데, `union()`을 최적화 할 때 사용할 것이다.

- `find()`: `x`가 속한 집합 탐색 (집합 식별 -> 대표자 찾기)

{% include code-header.html %}
```java
class DisjointSet {
    public int find(int x) {
        // 자신이 자신의 부모라면 로트 노드이자 집합의 대표자가 됨
        if (x == parent[x]) {
            return x;
        }
        return parent[x] = find(parents[x]);
    }
}
```

## 경로 압축

거쳐가는 경로 상의 모든 노드가 직접 루트(대표자)를 가리키도록 설정하여 트리의 높이를 줄이는 최적화 기법이다.

{% include code-header.html %}
```java
if (a == parent[a]) {
    return a;
}
return parent[a] = find(parents[a]);
```

- `union(x, y)`: `x`와 `y`의 합집합

{% include code-header.html %}
```java
class DisjointSet {
    public boolean union(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);

        // 랭크 기반 합치기
        if (rootX != rootY) {
            if (rank[rootX] > rank[rootY]) {
                parent[rootY] = rootX;
            } else if (rank[rootX] < rank[rootY]) {
                parent[rootX] = rootY;
            } else {
                parent[rootY] = rootX;
                rank[rootX]++;
            }
            return true;
        }
        // 두 집합의 대표자가 같으면 이미 같은 집합으로 합집합 불가능
        return false;
    }
}
```

## 랭크 기반 합치기

트리의 높이를 랭크로 간주하여 높이가 낮은 쪽의 루트 노드를 높은 쪽의 루트 노드에 연결하는 방식이다. 트리의 균형을 유지하여 높이를 최소화하기 때문에 성능 최적화를 할 수 있다.

{% include code-header.html %}
```java
if (rank[rootX] > rank[rootY]) {
    parent[rootY] = rootX;
} else if (rank[rootX] < rank[rootY]) {
    parent[rootX] = rootY;
} else {
    parent[rootY] = rootX;
    rank[rootX]++;
}
```

## 시간 복잡도

이 두 최적화 기법 덕분에 시간 복잡도는 역 아커만 함수가 나온다고 한다. 이는 모든 n에 대해서 5보다 작은 함수로, 사실상의 상수 시간에 서로소 집합을 구성할 수 있다는 뜻이다.

## 전체 코드

{% include code-header.html %}
```java
class DisjointSet {
    private int[] parent;
    private int[] rank;

    public DisjointSet(int size) {
        parent = new int[size];
        rank = new int[size];
        make();
    }

    public void make() {
        // 자신의 부모를 자신으로 하여 대표자가 되도록 함
        for (int i = 0; i < size; i++) {
            parent[i] = i;
            rank[i] = 1;
        }
    }

    public int find(int x) {
        // 자신이 자신의 부모라면 로트 노드이자 집합의 대표자가 됨
        if (x == parent[x]) {
            return x;
        }
        return parent[x] = find(parents[x]);
    }

    public boolean union(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);

        // 랭크 기반 합치기
        if (rootX != rootY) {
            if (rank[rootX] > rank[rootY]) {
                parent[rootY] = rootX;
            } else if (rank[rootX] < rank[rootY]) {
                parent[rootX] = rootY;
            } else {
                parent[rootY] = rootX;
                rank[rootX]++;
            }
            return true;
        }
        // 두 집합의 대표자가 같으면 이미 같은 집합으로 합집합 불가능
        return false;
    }
}
```

