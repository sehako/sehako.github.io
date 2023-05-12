package sources.Programming;

import java.util.Scanner;

class Time {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int a = sc.nextInt();
        int b = sc.nextInt();
        int c = sc.nextInt();
        int d = sc.nextInt();
        sc.close();

        System.out.println((a + b + c + d) / 60);
        System.out.println((a + b + c + d) % 60);
    }
}