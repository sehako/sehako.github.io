package sources.Programming;

import java.util.Scanner;

class OX {
    public static void main(String[] args) {
        int result = 0;
        Scanner sc = new Scanner(System.in);
        int x = sc.nextInt();
        sc.nextLine();
        for (int i = 0; i < x; i++) {
            String str = sc.nextLine();
            result = 0;
            int score = 1;
            for (int j = 0; j < str.length(); j++) {
                if (str.charAt(j) == 'O') {
                    result += score;
                    score++;
                }
                else {
                    score = 1;
                }
            }
            System.out.println(result);
        }
        sc.close();
    }
}