---
title:  "[Algorithm] [Java]유클리드 호제법"
excerpt: " "

categories:
  - Baekjoon

toc: true
toc_sticky: true
 
date: 2023-08-09
---

# 개요

[백준 문제](https://www.acmicpc.net/problem/2609)에 대한 풀이, 반복문을 이용하여 최대공약수와 최소공배수를 구했다.

```java
public class Main {
    public static void main(String[] args) throws IOException {
        int num = 2;
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        StringTokenizer st = new StringTokenizer(br.readLine());
        int A = Integer.parseInt(st.nextToken());
        int B = Integer.parseInt(st.nextToken());
        br.close();

        int min_multi = 0;
        int max_divide = 0;
        int temp = 0;
        if(A > B) temp = B;
        else temp = A;

        while(temp >= num) {
            if(A % num == 0 && B % num == 0) {
                max_divide = num;
            }
            num++;
        }

        if(max_divide == 0) {
            max_divide++;
        }

        min_multi = (A * B) / max_divide;

        System.out.println(max_divide);
        System.out.println(min_multi);
    }
}
```

두 수를 비교하여 작은 수를 `temp` 변수에 대입시켜 반복문에서 1씩 증가시키며 최대로 나눌 수 있는 수를 `max_divide`에 대입시키는 방식이다. 최소 공배수는 아무리 생각해도 답이 나오지 않아 찾아봤더니 두 수의 곱을 최대 공약수로 나누면 최소 공배수가 나온다기에 바로 `min_multi` 변수에 해당 계산값을 대입하여 출력한다. 이 알고리즘의 시간복잡도는 O(N)이다.

재귀를 이용한 유클리드 호제법으로 좀 더 빠르게 구현할 수 있다.

# 유클리드 호제법

```java
public class Euclidean {
    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        StringTokenizer st = new StringTokenizer(br.readLine());
        int a = Integer.parseInt(st.nextToken());
        int b = Integer.parseInt(st.nextToken());
        br.close();

        System.out.println(gcd(a, b));
        System.out.println(gcd_loop(a, b));
        // 최소 공배수
        System.out.println((a * b) / gcd(a, b));
    }

    // 재귀를 통해 구현
    private static int gcd(int x, int y) {
        if(y == 0) return x;
        else return gcd(y, x % y);
    }

    // 반복문을 통해 구현
    private static int gcd_loop(int x, int y) {
        while(y > 0) {
            int temp = x;
            x = y;
            y = temp % y;
        }
        return x;
    }
}
```

이 알고리즘의 시간복잡도는 O(logN)이다.

## 재귀

우선 재귀를 통해 구현한 알고리즘을 차근차근 풀어보기 위해 `x`와 `y`를 8과 26으로 넣어보면 재귀의 과정은 다음과 같을 것이다.

```java
if(26 == 0) return 8;
else return gcd(26, 8); // 8 % 26 = 8

if(8 == 0) return 26;
else return gcd(8, 2); // 26 % 8 = 2

if(2 == 0) return 8;
else return gcd(2, 0);

if(0 == 0) return 2; //재귀 종료
else return gcd(0, 2 % 0);
```

재귀의 가장 중요한 점은 중단시키는 부분을 만들어야 한다는 것이다. 그렇지 않으면 계속해서 함수를 실행하여 스택 오버플로우를 야기한다고 알고 있다. 위 함수에서는 `if` 조건문이 그 역할을 수행한다. 

나머지 연산자는 두 수를 음수로 입력하지 않는 이상 음수로 계산될 일이 없다. 따라서 `if(y == 0)`의 조건이 충족되면 `x`값을 반환하면서 메모리 스택에 쌓인 함수를 순서대로(LIFO) 종료시킨다. 

## 루프

루프로 구현한 알고리즘은 다음과 같은 과정으로 진행된다.

```java
while(26 > 0) {
    int temp = 8;
    x = 26;
    y = 8 % 26; // 8
}

while(8 > 0) {
    int temp = 26;
    x = 8;
    y = 26 % 8; // 2
}

while(2 > 0) {
    int temp = 8;
    x = 2;
    y = 8 % 2; // 0
}

while(0 > 0) {
    // break
}
return 2;
```

마찬가지로 나머지 연산자는 두 양수 사이에서는 음수가 나올 수 없기에 `y > 0`을 반복 조건문으로 내건 것을 볼 수 있다.