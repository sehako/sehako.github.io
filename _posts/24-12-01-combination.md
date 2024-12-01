---
title: 완전탐색 - 조합 (Combination)
excerpt: 완전 탐색 기초

categories:
  - Algorithm

toc: true
toc_sticky: true
 
date: 2024-12-01
---

# 조합

서로 다른 n개의 원소 중 r개를 순서 없이 골라낸 것을 조합이라고 한다. $_nC_r$이라고 하며 수식은 다음과 같다.

$$
_nC_r = \frac {(n!)} {(n-r)! \times r!} (단, n \ge r)
$$

조합은 다음 성질을 가지고 있다. 

## 구현

**반복문**

원소 `{1, 2, 3, 4}`중에서 3개를 포함하는 모든 조합의 수를 생성하는 것을 반복문을 통해 구현하면 다음과 같다. 순열과 마찬가지로 반복문의 중첩 개수가 선택하는 수가 되기 때문에 유연하지 못하다.

```java
int[] arr = {1, 2, 3, 4};
for (int i = 0; i < 4; i++) {
	for (int j = i + 1; j < 4; j++) {
		for (int k = j + 1; k < 4; k++) {
			System.out.printf("[%d, %d, %d]\n", arr[i], arr[j], arr[k]);
		}
	}
}
```

**재귀**

따라서 재귀 함수를 통해 구현한다. 조합은 순열과 다르게 한 번 선택한 수는 아예 고려하면 안되므로 depth를 선택한 인덱스의 다음 수로 설정해줘야 한다.

```java
int[] arr = {1, 2, 3, 4};
// 선택한 원소를 담을 배열
int[] result = new int[3];

static void combination(int index, int depth) {
	// 원소 3개를 선택했을 때
	if (index == 3) {
		System.out.println(Arrays.toString(result));
		return;
	}
	
	for (int i = depth; i < 4; i++) {
		result[index] = arr[i];
		// 이미 선택한 인덱스는 고려할 필요가 없으므로 i + 1
		combination(index + 1, i + 1);
	}
	
}
```

# 중복 조합

서로 다른 n개의 원소 중 r개를 순서 없이 골라낼 때 중복을 허용하는 것을 중복 조합이라고 한다. 수식은 다음과 같다.

$$
_nH_r = _{n + r - 1}C_r = _{n + r - 1}C_{n - 1}
$$

중복 조합의 구현은 선택한 인덱스의 값 자체를 넘겨줘 다음 선택시에도 해당 인덱스 값을 선택하도록 유도하면 된다.

```java
int[] arr = {1, 2, 3, 4};
int[] result = new int[3];
void combinationWithDuplication(int index, int start) {
	if (index == 3) {
		System.out.println(Arrays.toString(result));
		return;
	}
	
	for (int i = start; i < 4; i++) {
		result[index] = arr[i];
		// 현재 선택한 인덱스를 다음 start 값으로 넘겨주어 중복을 유도
		combinationWithDuplication(index + 1, i);
	}
}
```