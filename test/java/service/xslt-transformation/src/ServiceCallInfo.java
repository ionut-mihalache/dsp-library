import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Structure;

/**
    struct ServiceCallInfo {
        struct DSPQueue m_Queue;
        struct QMBDSPQueue m_QMBQueue;
        struct HMBDSPQueue m_HMBQueue;
        int32_t (*m_CallFn)(struct DSPQueue *);
        int32_t (*m_ReceiveCallFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
        int32_t (*m_ReceiveCallFnHMB)(struct HMBCall *, struct HMBDSPQueue *);
    };
 */

interface CallFnCallback extends Callback {
    int receiveCall(DSPQueue p_Queue);
}

interface ReceiveCallFnQMB extends Callback {
    int receiveQMBCall(QMBCall p_CallInfo, QMBDSPQueue p_Queue);
}

interface ReceiveCallFnHMB extends Callback {
    int receiveQMBCall(HMBCall p_CallInfo, HMBDSPQueue p_Queue);
}

public class ServiceCallInfo extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_Queue", "m_QMBQueue", "m_HMBQueue", "m_CallFn", "m_ReceiveCallFnQMB", "m_ReceiveCallFnHMB");

    public DSPQueue m_Queue;
    public QMBDSPQueue m_QMBQueue;
    public HMBDSPQueue m_HMBQueue;
    public CallFnCallback m_CallFn;
    public ReceiveCallFnQMB m_ReceiveCallFnQMB;
    public ReceiveCallFnHMB m_ReceiveCallFnHMB;

    public ServiceCallInfo() {
        super();
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
