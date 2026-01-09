---
title: 벨만-포드 (Bellman-Ford)

categories:
  - Algorithm

toc: true
toc_sticky: true
published: true

date: 2025-08-12
last_modified_at: 2025-08-12
---

다익스트라 알고리즘과 같이 가중 유향 그래프에서 최단 경로 문제를 푸는 알고리즘이다. 하지만 다익스트라 알고리즘이 모든 가중치가 양수여야 하는 것과는 다르게 벨만-포드 알고리즘은 가중치가 음수여도 최단 경로를 도출할 수 있다. V가 정점, E가 간선이라고 가정했을 때, 벨만-포드 알고리즘의 시간 복잡도는 $O(\vert V \vert \vert E \vert)$이다.

# 이론

벨만포드 알고리즘의 동작 과정은 다음과 같다.

1. 모든 정점까지의 거리를 무한대로 초기화하되, 출발 정점의 초기값을 0으로 설정한다.
2. 정점의 개수(V) - 1 번 만큼 반복을 진행한다.
   1. 모든 간선을 순회하며 거리를 갱신
   2. 기존 값보다 더 작은 값으로 업데이트 된다면 거리를 갱신
3. 음의 사이클을 확인하기 위해서 한 번 더 거리를 갱신(V번)하여 업데이트 되는 지 확인한다.
   1. 업데이트 된다면 음의 사이클이 존재하는 것
4. 최종적으로 구한 경로들이 출발점에서의 최단 경로가 된다.

하나씩 살펴보도록 하자.

**V - 1 번의 반복**

5개의 정점을 가진 그래프로부터 1번 정점에서 출발하여 다른 경로로의 최단 거리를 계산한다고 가정해보자. 이 경우에는 도착지까지의 간선의 개수가 최대 4개로 구성되어 최단 경로가 이루어질 수 있기 때문에 정점의 개수 V개에서 1번을 뺀 V - 1번의 반복으로 출발지에서 모든 정점까지의 최단 경로를 구할 수 있다.

**음수 사이클**

음수 사이클이라는 것은 어떤 정점에서 해당 정점으로의 사이클이 구성되고, 해당 사이클의 전체적인 가중치의 합이 음수라는 의미이다. 따라서 음의 사이클이 존재한다면 해당 그래프는 최단 경로를 정의할 수 없다. 그 이유는 그래프를 계속해서 순회한다면 사이클이 존재하는 정점의 최단 경로는 계속해서 최소값을 갱신할 것이고, 결과적으로 음의 무한대로 수렴하게 될 것이기 때문이다.

# 알고리즘 구현

벨만-포드 알고리즘 구현을 위해서 백준에 있는 다음 문제를 풀어볼 것이다.

**타임머신 (11657)**

- **문제**

N개의 도시가 있다. 그리고 한 도시에서 출발하여 다른 도시에 도착하는 버스가 M개 있다. 각 버스는 A, B, C로 나타낼 수 있는데, A는 시작도시, B는 도착도시, C는 버스를 타고 이동하는데 걸리는 시간이다. 시간 C가 양수가 아닌 경우가 있다. C = 0인 경우는 순간 이동을 하는 경우, C < 0인 경우는 타임머신으로 시간을 되돌아가는 경우이다.

1번 도시에서 출발해서 나머지 도시로 가는 가장 빠른 시간을 구하는 프로그램을 작성하시오.

- **입력**

첫째 줄에 도시의 개수 N (1 ≤ N ≤ 500), 버스 노선의 개수 M (1 ≤ M ≤ 6,000)이 주어진다. 둘째 줄부터 M개의 줄에는 버스 노선의 정보 A, B, C (1 ≤ A, B ≤ N, -10,000 ≤ C ≤ 10,000)가 주어진다.

- **출력**

만약 1번 도시에서 출발해 어떤 도시로 가는 과정에서 시간을 무한히 오래 전으로 되돌릴 수 있다면 첫째 줄에 -1을 출력한다. 그렇지 않다면 N-1개 줄에 걸쳐 각 줄에 1번 도시에서 출발해 2번 도시, 3번 도시, ..., N번 도시로 가는 가장 빠른 시간을 순서대로 출력한다. 만약 해당 도시로 가는 경로가 없다면 대신 -1을 출력한다.

## 입력 및 그래프 구성

입력값에 따라서 그래프를 구성해야 한다. 간선에 대한 정보는 간단하게 2차원 배열을 통해 구현할 것이다. 주어진 M값에 따른 M크기의 2차원 배열에는 3개의 요소가 저장되는데, 0번 인덱스부터 순서대로 출발지, 도착지, 소요 시간을 저장할 것이다.

```java
public static void main(String[] args) throws IOException {
	BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
	StringTokenizer st;

	st = new StringTokenizer(br.readLine(), " ");
	int N = stoi(st.nextToken());
	int M = stoi(st.nextToken());

	// 출발지, 도착지, 소요 시간을 주어진 간선의 개수만큼 저장하는 2차원 배열
	int[][] graph = new int[M][3];
	int from, to, weight;
	for (int r = 0; r < M; r++) {
		st = new StringTokenizer(br.readLine(), " ");

		from = stoi(st.nextToken());
		to = stoi(st.nextToken());
		weight = stoi(st.nextToken());

		graph[r][0] = from;
		graph[r][1] = to;
		graph[r][2] = weight;
	}
}

private static int stoi(String number) {
	return Integer.parseInt(number);
}
```

한 가지 눈여겨 볼 것은 그래프를 구성할 때 일반적인 그래프 탐색 알고리즘은 출발지를 이용하여 해당 인덱스에 저장하는 방식이지만 벨만-포드 알고리즘에서는 간선의 개수만큼의 그래프 정보만을 저장한다는 것을 볼 수 있다.

## 최단 경로 구하기

벨만-포드 알고리즘을 구현할 차례다.

```java
private static long[] getDist(int N, int M, int start, int[][] graph) {
	// 모든 정점의 최단 거리를 구하기 위한 배열 생성
	long[] dist = new long[N + 1];

	// 전체 배열을 INF로 초기화
	Arrays.fill(dist, INF);

	// 벨만-포드 알고리즘에 따라서 시작 정점의 최단 거리는 0으로 설정
	dist[start] = 0;

	for (int i = 0; i < N - 1; i++) {
		for (int r = 0; r < M; r++) {
			int[] edge = graph[r];

			// 시작 정점의 최소 가중치 배열이 무한대면 아직 방문하지 않은 정점이란 의미
			if (dist[edge[0]] == INF) continue;
			// 목적지 정점의 최소 소요 시간보다 시작 정점의 최소 소요 시간이 더 많은 경우
			if (dist[edge[1]] <= dist[edge[0]] + edge[2]) continue;

			// 최소 소요 시간 갱신
			dist[edge[1]] = dist[edge[0]] + edge[2];

			// 시작 정점의 최소 소요시간이 무한대인지만 확인 후 다음과 같이 최소값 갱신도 가능
			// dist[edge[1]] = Math.min(dist[edge[1]], dist[edge[0]] + edge[2]);
		}
	}

	// 음수 사이클 확인
	for (int i = 0; i < M; i++) {
		int[] edge = graph[i];

		if (dist[edge[0]] == INF) continue;
		if (dist[edge[1]] <= dist[edge[0]] + edge[2]) continue;

		// 음수 사이클이 확인되면 null을 리턴
		// 최단 경로를 도출할 수 없다는 의미
		return null;
	}

	// 음수 사이클이 확인되지 않으면 최단 경로 반환
	return dist;
}
```

`boolean`을 반환하는 식으로 음의 사이클을 확인하는 코드가 많았지만, 나는 `null`을 활용하면 좀 더 깔끔하지 않을까 해서 그렇게 구현해봤다. 최단 경로를 `long` 배열로 설정한 이유는 다음과 같다.

> 500개 vertex, 6,000개의 edge가 -10,000 일 경우 최솟값 계산이기에 -30억이 나올 수 있습니다. 양수로는 최대 60,000,000까지만 가능해서 INF값 설정을 작게 잡아도 되지만, dist를 저장하는 배열은 long long으로 선언해야 INT 음수 최댓값 범위 바깥을 커버할 수 있습니다.

## 최소 경로 출력

메서드의 결과에 따라서 -1을 출력하거나, 출발지인 1번 정점을 제외한 나머지 정점의 최단 경로를 출력하도록 하자.

```java
StringBuilder sb = new StringBuilder();
long[] result = getDist(N, M, 1, graph);

if (result == null) sb.append(-1);
else {
	for (int i = 2; i <= N; i++) {
		long dist = result[i];
		sb.append(dist == INF ? -1 : dist).append('\n');
	}
}

System.out.println(sb);
```

## 전체 코드

전체 코드는 다음과 같다.

```java
import java.util.*;
import java.io.*;

public class BOJ_11657 {
	private static long INF = 600_000_001;

	public static void main(String[] args) throws IOException {
		BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
		StringTokenizer st;

		st = new StringTokenizer(br.readLine(), " ");
		int N = stoi(st.nextToken());
		int M = stoi(st.nextToken());

		int[][] graph = new int[M][3];
		int from, to, weight;
		for (int r = 0; r < M; r++) {
			st = new StringTokenizer(br.readLine(), " ");

			from = stoi(st.nextToken());
			to = stoi(st.nextToken());
			weight = stoi(st.nextToken());

			graph[r][0] = from;
			graph[r][1] = to;
			graph[r][2] = weight;
		}


		StringBuilder sb = new StringBuilder();
		long[] result = getDist(N, M, 1, graph);

		if (result == null) sb.append(-1);
		else {
			for (int i = 2; i <= N; i++) {
				long dist = result[i];
				sb.append(dist == INF ? -1 : dist).append('\n');
			}
		}

		System.out.println(sb);
	}

	private static long[] getDist(int N, int M, int start, int[][] graph) {
		// 모든 정점의 최단 거리를 구하기 위한 배열 생성
		long[] dist = new long[N + 1];
		// 벨만-포드 알고리즘에 따라서 시작 정점의 최단 거리는 0으로 설정
		Arrays.fill(dist, INF);

		// 출발지 초기화
		dist[start] = 0;

		for (int i = 0; i < N - 1; i++) {
			for (int r = 0; r < M; r++) {
				int[] edge = graph[r];

				// 시작 정점의 최소 가중치 배열이 무한대면 아직 방문하지 않은 정점이란 의미
				if (dist[edge[0]] == INF) continue;
				// 목적지 정점의 최소 소요 시간보다 시작 정점의 최소 소요 시간이 더 많은 경우
				if (dist[edge[1]] <= dist[edge[0]] + edge[2]) continue;

				// 최소 소요 시간 갱신
				dist[edge[1]] = dist[edge[0]] + edge[2];

				// 시작 정점의 최소 소요시간이 무한대인지만 확인 후 다음과 같이 최소값 갱신도 가능
				// dist[edge[1]] = Math.min(dist[edge[1]], dist[edge[0]] + edge[2]);
			}
		}

		// 음수 사이클 확인
		for (int i = 0; i < M; i++) {
			int[] edge = graph[i];

			if (dist[edge[0]] == INF) continue;
			if (dist[edge[1]] <= dist[edge[0]] + edge[2]) continue;

			// 음수 사이클이 확인되면 null을 리턴
			// 최단 경로를 도출할 수 없다는 의미
			return null;
		}

		// 음수 사이클이 확인되지 않으면 최단 경로 반환
		return dist;
	}

	private static int stoi(String number) {
		return Integer.parseInt(number);
	}
}
```

# 참고 자료

[**벨먼-포드 알고리즘**](https://ko.wikipedia.org/wiki/%EB%B2%A8%EB%A8%BC-%ED%8F%AC%EB%93%9C_%EC%95%8C%EA%B3%A0%EB%A6%AC%EC%A6%98)

[**02. 벨만 포드 알고리즘**](https://wikidocs.net/206946)

[**[알고리즘/JAVA] 그래프 최단경로 : Bellman-Ford (벨만-포드)**](https://sjh9708.tistory.com/235)

[**[Java] 벨만 포드(Bellman-Ford Algorithm)**](https://velog.io/@jeongbeom4693/Java-Bellman-Ford-Algorithm)

[**[JAVA] 벨만포드 알고리즘 (Bellman-Ford)**](https://chb2005.tistory.com/79)

[**질문 게시판 - 38% 출력초과 나시는 분**](https://www.acmicpc.net/board/view/141726)
