package sources.Programming;

import java.util.Scanner;

class Snack {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int x = sc.nextInt();
        int y = sc.nextInt();
        int z = sc.nextInt();
        sc.close();

        if(x * y > z) {
            System.out.println((x * y) - z);
        }
        else {
            System.out.println(0);
        }
    }
}