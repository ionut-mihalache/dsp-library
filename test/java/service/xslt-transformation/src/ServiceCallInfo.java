import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Structure;

interface CallFnCallback extends Callback {
    int receiveCall(DSPQueue m_Queue);
}

public class ServiceCallInfo extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_Queue", "m_CallFn");

    public DSPQueue m_Queue;
    public CallFnCallback m_CallFn;

    public ServiceCallInfo() {
        super();
    }

    public CallFnCallback getCallFn() {
        return m_CallFn;
    }

    public DSPQueue getQueue() {
        return m_Queue;
    }

    public void setCallFn(CallFnCallback p_CallFn) {
        m_CallFn = p_CallFn;
    }

    public void setQueue(DSPQueue p_Queue) {
        m_Queue = p_Queue;
    }

    @Override
    protected List<String> getFieldOrder() {
        return FIELDS;
    }

    @Override
    public String toString() {
        return "CallInfo: " + m_CallFn.toString() + " " + m_Queue;
    }
}
