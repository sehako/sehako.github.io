package sources.Programming;

import java.util.Scanner;

class Car {
    public static void main(String[] args) {
        int count = 0;
        int temp;
        Scanner sc = new Scanner(System.in);
        int num = sc.nextInt();
        for(int i = 0; i < 5; i++) {
            temp = sc.nextInt();
            if(num == temp) {
                count++;
            }
        }
        sc.close();
        System.out.println(count);
    }
}