package sources.Programming;

import java.util.Scanner;

class Average_recover {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        int r1 = scanner.nextInt();
        int s = scanner.nextInt();
        scanner.close();

        int r2 = (2 * s) - r1;

        System.out.println(r2);
    }
}