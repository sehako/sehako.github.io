package sources.Programming;

import java.util.Scanner;

class Escape {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int x = sc.nextInt();
        int y = sc.nextInt();
        int w = sc.nextInt();
        int h = sc.nextInt();
        sc.close();
        
        int x_dist = w - x;
        int y_dist = h - y;

        if (x_dist > x) {
            if (y_dist > y) {
                if (x > y) {
                    System.out.println(y);
                }
                else {
                    System.out.println(x);
                }
            }
            else {
                if (x < y_dist) {
                    System.out.println(x);
                }
                else {
                    System.out.println(y_dist);
                }
            }
        }
        else {
            if (y_dist > y) {
                if (x_dist > y) {
                    System.out.println(y);
                }
                else {
                    System.out.println(x_dist);
                }
            }
            else {
                if (x_dist < y_dist) {
                    System.out.println(x_dist);
                }
                else {
                    System.out.println(y_dist);
                }
            }
        }
    }
}