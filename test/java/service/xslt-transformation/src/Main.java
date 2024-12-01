import java.util.Arrays;
import java.util.List;

// import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Structure;
import com.sun.jna.Pointer;

// interface CallFnCallback extends Callback {
//     int call();
// }

class ServiceCallInfo extends Structure {
    public Pointer m_CallFn;
    public int m_CallQPushIdx;
    public int m_CallQPopIdx;

    // public CallFnCallback getCallFn() {
    //     return m_CallFn;
    // }

    // public int getCallQPushIdx() {
    //     return m_CallQPushIdx;
    // }

    // public int getCallQPopIdx() {
    //     return m_CallQPopIdx;
    // }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("m_CallFn", "m_CallQPushIdx", "m_CallQPopIdx");
    }

    @Override
    public String toString() {
        return "CallInfo: " + m_CallFn.toString() + " " + m_CallQPushIdx + " " + m_CallQPopIdx;
    }
};

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP)Native.load("dsp", LibDSP.class);

    void dspInstall(ServiceCallInfo p_CallInfo, String p_StrId, String p_Version);
    void dspReturn();
}

public class Main {
    public static void main(String[] args) {
        ServiceCallInfo callInfo = new ServiceCallInfo();

        System.out.println("Hello, hello");
        LibDSP.INSTANCE.dspInstall(callInfo, "xslt-transformation", "v0.0.1");

        System.out.println(callInfo);
    }
};
