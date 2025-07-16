---
title: 플로이드-워셜(Floyd-Warshall)

categories:
  - Algorithm

toc: true
toc_sticky: true
published: true
 
date: 2025-06-08
last_modified_at: 2025-06-08
---

플로이드-워셜은 가중치가 모두 양수인 가중 그래프에서 모든 노드 간의 최단 경로의 가중치의 합을 $O(n^3)$의 시간 복잡도로 찾는 알고리즘이다. 다이나믹 프로그래밍 기법을 사용한 알고리즘이고, 인접 행렬을 활용하여 각 노드 간 최소 비용을 계산한다.

# 이론

1에서 N까지 번호가 매겨진 V를 꼭짓점으로 갖는 그래프가 있을 때, i에서 j로 집합 `{1, 2, ..., k}`의 꼭짓점들을 경유지로 거쳐가는 최단 경로를 반환하는 함수인 `shortestPath(i, j, k)`를 생각한다. 

함수가 주어졌을 때, 목표는 `{1, 2, ..., N}`에 있는 꼭짓점만을 이용하여 모든 꼭짓점 i에서 모든 꼭짓점 j로 가는 최단 경로를 찾는 것이다.

각각의 꼭짓점 쌍에 대해서 `shortestPath(i, j, k)`는 다음 중 한 가지에 속한다.

1. k를 통과하지 않는 경로
2. k를 통과하는 경로

여기서 k를 통과하는 경로의 경우 i에서 j로 직접 가는 경우와 i에서 k의 경유지를 거치고, k의 경유지에서 j로 가는 경우를 합친 다음 비교하여 최소 가중치를 선택하면 될 것이다. 즉 `k`는 중간 경유지가 되는 것이고, `i`와 `j`는 각각 출발지와 목적지가 되는 것이다.

이를 의사 코드로 보면 다음과 같다.

```c
let dist // 모든 요소의 값이 INF인 |V| x |V| 배열
for each edge (u,v)
   dist[u][v] ← w(u,v)  // 변 (u,v)의 가중치
for each vertex v
   dist[v][v] ← 0  // 자기 자신의 가중치는 0으로 책정
for k from 1 to |V|
   for i from 1 to |V|
      for j from 1 to |V|
		     dist[i][j] ← min(dist[i][j], dist[i][k] + dist[k][j])
```

# 알고리즘 구현

플로이드-워셜 알고리즘을 구현 하기 위해 백준에 있는 [문제](https://www.acmicpc.net/problem/11404)를 풀어볼 것이다. 문제는 다음과 같다.

> n(2 ≤ n ≤ 100)개의 도시가 있다. 그리고 한 도시에서 출발하여 다른 도시에 도착하는 m(1 ≤ m ≤ 100,000)개의 버스가 있다. 각 버스는 한 번 사용할 때 필요한 비용이 있다.
> 
> 
> 모든 도시의 쌍 (A, B)에 대해서 도시 A에서 B로 가는데 필요한 비용의 최솟값을 구하는 프로그램을 작성하시오.
> 
> 첫째 줄에 도시의 개수 n이 주어지고 둘째 줄에는 버스의 개수 m이 주어진다. 그리고 셋째 줄부터 m+2줄까지 다음과 같은 버스의 정보가 주어진다. 먼저 처음에는 그 버스의 출발 도시의 번호가 주어진다. 버스의 정보는 버스의 시작 도시 a, 도착 도시 b, 한 번 타는데 필요한 비용 c로 이루어져 있다. 시작 도시와 도착 도시가 같은 경우는 없다. 비용은 100,000보다 작거나 같은 자연수이다.
> 
> 시작 도시와 도착 도시를 연결하는 노선은 하나가 아닐 수 있다.
> 

`BufferedReader` 와 `StringTokenizer`를 활용하여 입력을 받을 것이다. 여기서 최대 비용이 100억이 될 수 있으므로 비용에 관한 부분은 `long` 타입으로 설정해줘야 한다. 

```java
import java.util.*;
import java.io.*;

public class Main {

	private static final long INF = 10_000_000_001L;
	private static long[][] cost;
	
    public static void main(String[] args) throws IOException {
    	BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
    	StringTokenizer st;
    	
    	int n = stoi(br.readLine());
    	int m = stoi(br.readLine());
    	
		}
		
    private static int stoi(String number) {
    	return Integer.parseInt(number);
    }
    
    private static long stol(String number) {
    	return Long.parseLong(number);
    }
}
```

## 가격 정보 초기화

가격은 각 도시의 최대 가격으로 하여 $n \times n$ 배열로 설정한다. 그리고 초기에는 전체 배열을 앞서 정의한 불변 변수인 `INF`로 설정한다.

```java
cost = new long[n][n];

for (int i = 0; i < n; i++) {
	Arrays.fill(cost[i], INF);
	cost[i][i] = 0L;
}
```

또한 출발지와 도착지가 같은 경우는 요금이 무조건 0이 될 수 밖에 없기 때문에 전체 배열을 `INF`로 갱신하는 동시에 출발지와 도착지가 같은 경우도 0으로 설정한다.

## 노선 정보 갱신

이제 입력값에 따라서 노선 정보를 갱신하도록 하자. 노선 정보는 각각 출발도시, 도착도시, 요금 이렇게 나누어진다. 이때 요금은 `long` 타입으로 변환시켰다.

```java
for (int i = 0; i < m; i++) {
	st = new StringTokenizer(br.readLine(), " ");
	int from = stoi(st.nextToken()) - 1;
	int to = stoi(st.nextToken()) - 1;
	long weight = stol(st.nextToken());
	
	cost[from][to] = Math.min(cost[from][to], weight);
}
```

여기서 `cost` 배열을 갱신할 때 현재 배열에 저장된 값과 새로 입력된 요금의 최소값을 비교하게 되는데, 그 이유는 같은 출발지 - 목적지를 가진 도시일지라도 요금이 다른 경우가 있기 때문이다. 앞서 문제에서 설명한 시작 도시와 도착 도시의 노선은 하나가 아닐 수 있다는 것에 유의하도록 하자.

## 최소 요금 구하기

이제 한 도시에서 다른 도시로 이동하는 최소 요금을 구하기 위해서 플로이드-워셜 알고리즘을 활용하게 된다. 이는 앞서 이론 부분에서 확인했듯이 각 도시를 지나쳐서 가는 경우와 현재 저장된 최소값을 비교해가면서 모든 도시를 대상으로 모든 도시를 경유지로 설정하여 계산하면 된다.

```java
private static void getDist(int n) {
	for (int k = 0; k < n; k++) {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				// 현재 설정된 최소값과 i에서 k, k에서 j로 가는 것의 합을 비교
				cost[i][j] = Math.min(cost[i][j], cost[i][k] + cost[k][j]);
			}
		}
	}
}
```

## 전체 코드

문제 풀이에 대한 전체 코드를 살펴보도록 하자.

{% include code-header.html %}
```java
import java.util.*;
import java.io.*;

public class Main {

	private static final long INF = 10_000_000_001L;
	private static long[][] cost;
	
    public static void main(String[] args) throws IOException {
    	BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
    	StringTokenizer st;
    	
    	int n = stoi(br.readLine());
    	int m = stoi(br.readLine());
    	
    	cost = new long[n][n];
    	
    	for (int i = 0; i < n; i++) {
    		Arrays.fill(cost[i], INF);
    		cost[i][i] = 0L;
    	}
    	
    	for (int i = 0; i < m; i++) {
    		st = new StringTokenizer(br.readLine(), " ");
    		int from = stoi(st.nextToken()) - 1;
    		int to = stoi(st.nextToken()) - 1;
    		long weight = stol(st.nextToken());
    		
    		cost[from][to] = Math.min(cost[from][to], weight);
    	}
    	
    	getDist(n);
    	
    	StringBuilder sb = new StringBuilder();
    	for (long[] arr : cost) {
    		for (long num : arr) {
    			sb.append(num == INF ? 0 : num).append(' ');
    		}
    		sb.append('\n');
    	}
    	
    	System.out.println(sb);
    }
    
    private static void getDist(int n) {
    	for (int k = 0; k < n; k++) {
    		for (int i = 0; i < n; i++) {
    			for (int j = 0; j < n; j++) {
    				cost[i][j] = Math.min(cost[i][j], cost[i][k] + cost[k][j]);
    			}
    		}
    	}
    }
    
    private static int stoi(String number) {
    	return Integer.parseInt(number);
    }
    
    private static long stol(String number) {
    	return Long.parseLong(number);
    }
    
}
```

---

알고리즘은 항상 해야지 해야지 하다가 안하게 되는 것 같다. 개발 분야는 꾸준히 하다보면 뭐라도 되겠지 싶은데 알고리즘은 그런 느낌이 안들어서 그런가… 

암튼 플로이드-워셜을 한 두번 접했을 때 원리에 대한 이해가 잘 안됐었는데, 이번에 공부를 하게 되면서 어느정도 이해를 하게 된 것 같다..

# 참고 자료

[**플로이드-워셜 알고리즘**](https://ko.wikipedia.org/wiki/%ED%94%8C%EB%A1%9C%EC%9D%B4%EB%93%9C-%EC%9B%8C%EC%85%9C_%EC%95%8C%EA%B3%A0%EB%A6%AC%EC%A6%98)

[**알고리즘 - 플로이드-워셜(Floyd-Warshall) 알고리즘**](https://chanhuiseok.github.io/posts/algo-50/)

[**[Java]플로이드-워셜 알고리즘(Floyd-Warshall Algorithm)**](https://sskl660.tistory.com/61)