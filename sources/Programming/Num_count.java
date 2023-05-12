package sources.Programming;

import java.util.Scanner;

class Num_count {
    public static void main(String[] args) {
        int arr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        Scanner sc = new Scanner(System.in);
        int a = sc.nextInt();
        int b = sc.nextInt();
        int c = sc.nextInt();
        sc.close();

        int num = a * b * c;
        String str = Integer.toString(num);
        int zero = 0;
        for (int i = 0; i < str.length(); i++) {
            if (str.charAt(i) == '0') {
                zero++;
            }
            else {
                arr[(int)str.charAt(i) - 49] += 1;
            }
        }
        System.out.println(zero);
        for (int i = 0; i < arr.length; i++) {
            System.out.println(arr[i]);
        }
    }
}