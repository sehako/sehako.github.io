package sources.Programming;

import java.util.Scanner;

class Duck_clock {
    public static void main(String[] args) {
        int num_m = 0;
        int num_h = 0;
        Scanner sc = new Scanner(System.in);
        int h = sc.nextInt();
        int m = sc.nextInt();
        int s = sc.nextInt();
        int num = sc.nextInt();
        sc.close();

        while(num >= 60) {
            num -= 60;
            num_m++;
        }

        while(num_m >= 60) {
            num_m -= 60;
            num_h++;
        }

        h += num_h;
        m += num_m;
        s += num;

        if(s >= 60) {
            m++;
            s -= 60;
        }
        if(m >= 60) {
            h++;
            m -= 60;
        }
        if(h >= 24) {
            h -= 24;
        }

        System.out.print(h + " " + m + " " + s);
    }
}