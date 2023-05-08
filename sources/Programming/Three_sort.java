package sources.Programming;

import java.util.Arrays;
import java.util.Scanner;

class Three_sort {
    public static void main(String[] args) {
        int arr[] = {0, 0, 0};

        Scanner scanner = new Scanner(System.in);
        arr[0] = scanner.nextInt();
        arr[1] = scanner.nextInt();
        arr[2] = scanner.nextInt();
        scanner.close();
        Arrays.sort(arr);

        for(int i = 0; i < arr.length; i++) {
            System.out.print(arr[i] + " ");
        }
    }
}