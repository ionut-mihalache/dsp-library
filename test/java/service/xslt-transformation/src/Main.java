import com.sun.jna.Library;
import com.sun.jna.Native;

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP)Native.load("dsp", LibDSP.class);

    void hello();
    void install();

    int getValue();
}

public class Main {
    public static void main(String[] args) {
        // LibDSP.INSTANCE.hello();
        // System.out.println("Hello World!");

        LibDSP.INSTANCE.install();

        while(true) {
            System.out.println(LibDSP.INSTANCE.getValue());
            try {
                Thread.sleep(2500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
};
