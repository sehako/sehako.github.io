package sources.Programming;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

class Alphabet_count {
    public static void main(String[] args) throws IOException {
        int arr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        BufferedReader bf = new BufferedReader(new InputStreamReader(System.in));
        String str = bf.readLine();
        bf.close();
        
        for (int i = 0; i < str.length(); i++) {
            int x = (int)str.charAt(i) - 97;

            arr[x] += 1;
        }

        for (int i = 0; i < arr.length; i++) {
            System.out.print(arr[i]);
            System.out.print(' ');
        }
    }
}