import Interpreter.Interpreter;

public class Main {
    public static void main(String[] args) {
        System.out.println("Welcome to miniSQL!");
        Interpreter interpreter = new Interpreter();
        interpreter.main_loop();
    }
}
