import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP)Native.load("dsp", LibDSP.class);

    Pointer getCallInfo();
    void dspInstall(ServiceCallInfo p_CallInfo, String p_StrId, String p_Version);
    void dspReturn();
}

public class Main {
    public static void main(String[] args) {
        ServiceCallInfo callInfo = new ServiceCallInfo();

        LibDSP.INSTANCE.dspInstall(callInfo, "xslt-transformation", "v0.0.1");

        while (true) {
            try {
                System.out.println(callInfo);
                System.out.println(callInfo.getCallFn().receiveCall(callInfo.m_Queue));
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
};
