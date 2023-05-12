package sources.Programming;

import java.util.Scanner;

class Duck_clock {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int h = sc.nextInt();
        int m = sc.nextInt();
        int s = sc.nextInt();
        int num = sc.nextInt();
        sc.close();

        s += num;
        m += s / 60;
        s %= 60;
        h += m / 60;
        m %= 60;
        h %= 24;

        System.out.print(h + " " + m + " " + s);
    }
}