import com.sun.jna.Library;
import com.sun.jna.Native;

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP)Native.load("dsp", LibDSP.class);

    void dspInstall(String p_StrId, String p_Version);
    void dspReturn();
}

public class Main {
    public static void main(String[] args) {
        LibDSP.INSTANCE.dspInstall("xslt-transformation", "v0.0.1");
    }
};
