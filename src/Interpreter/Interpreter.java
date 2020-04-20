package Interpreter;

import java.util.Scanner;

public class Interpreter {
    public void main_loop() {
        Scanner in = new Scanner(System.in);
        while (true) {
            System.out.print("MiniSQL> ");
            String input_cmd = in.nextLine();
            parse(input_cmd);
        }
    }
    void parse(String cmd) {
        if (cmd.equals("exit") || cmd.equals("quit") || cmd.equals("q")) {
            System.out.println("Bye!");
            System.exit(0);
        }
        System.out.println(cmd);
    }
}
