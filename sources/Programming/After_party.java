package sources.Programming;

import java.util.Scanner;

class After_party {
    public static void main(String[] args) {
        int num = 0;
        int arr[] = {0, 0, 0, 0, 0};
        Scanner sc = new Scanner(System.in);
        int x = sc.nextInt();
        int y = sc.nextInt();
        num = x * y;
        for (int i = 0; i < arr.length; i++) {
            arr[i] = sc.nextInt();
            arr[i] = arr[i] - num;
        }
        sc.close();
        for (int i = 0; i < arr.length; i++) {
            System.out.print(arr[i] + " ");
        }
    }
}