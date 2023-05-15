package sources.Programming;

import java.util.Scanner;

class Pythagoras {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int x, y, z;

        while(true) {
            int temp = 0;
            x = sc.nextInt();
            y = sc.nextInt();
            z = sc.nextInt();

            if (x > y) {
                if (x > z) {
                    temp = z;
                    z = x;
                    x = temp;
                }
            }
            else {
                if (y > z) {
                    temp = z;
                    z = y;
                    y = temp;
                }
            }

            if (x == 0 && y == 0 && z == 0) {
                break;
            }
            else {
                if ((x * x) + (y * y) == (z * z)) {
                    System.out.println("right");
                } else {
                    System.out.println("wrong");
                }
            }
        }
        sc.close();
    }
}