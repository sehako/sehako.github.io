package sources.Programming;

import java.util.Scanner;

class Triangle {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int a = sc.nextInt();
        int b = sc.nextInt();
        int c = sc.nextInt();
        sc.close();

        if (a + b + c == 180) {
            if (a == b && a == c && b == c) {
                System.out.println("Equilateral");
            }
            else if (a == b || a == c || b == c) {
                System.out.println("Isosceles");
            }
            else {
                System.out.println("Scalene");
            }
        } else {
            System.out.println("Error");
        }
    }
}