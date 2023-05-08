package sources.Programming;

import java.util.Scanner;

class Sound {
    public static void main(String[] args) {
        int[] num = new int[8];
        Scanner sc = new Scanner(System.in);
        for(int i = 0; i < 8; i++) {
            num[i] = sc.nextInt();
        }
        sc.close();

        String result = "";
        for(int i = 0; i < num.length - 1; i++) {
            if(num[i] == num[i + 1] - 1) {
                result = "ascending";
            }
            else if(num[i] - 1 == num[i + 1]) {
                result = "descending";
            }
            else {
                result = "mixed";
                break;
            }
        }

        System.out.println(result);
    }
}