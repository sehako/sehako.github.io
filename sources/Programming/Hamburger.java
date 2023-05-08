package sources.Programming;

import java.util.Scanner;

class Hamburger {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int set = scanner.nextInt();
        for (int i = 0; i < 2; i++) {
            int temp = scanner.nextInt();
            if (temp < set) {
                set = temp;
            }
            else {
                continue;
            }
        }
        int coke = scanner.nextInt();
        int cidar = scanner.nextInt();
        scanner.close();

        if (coke > cidar) {
            System.out.println((set + cidar) - 50);
        }
        else {
            System.out.println((set + coke) - 50);
        }
    }
}