package sources.Programming;

import java.util.Scanner;

class Hotel {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int loop = sc.nextInt();

        for (int i = 0; i < loop; i++) {
            int num = 1;
            int h = sc.nextInt();
            int w = sc.nextInt();
            int n = sc.nextInt();

            while (h < n) {
                n = n - h;
                num++;
            }
            System.out.println((100 * n) + num);
        }

        sc.close();
    }
}