package call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import call.interfaces.ReceiveCallFnHMB;
import call.interfaces.ReceiveCallFnQMB;
import queues.HMBDSPQueue;
import queues.QMBDSPQueue;

/**
 * struct ServiceCallInfo {
 * struct HMBDSPQueue m_HMBQueue;
 * struct QMBDSPQueue m_QMBQueue;
 * int32_t (*m_ReceiveCallFnHMB)(struct HMBCall *, struct HMBDSPQueue *);
 * int32_t (*m_ReceiveCallFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
 * };
 */

@FieldOrder({ "m_HMBQueue", "m_QMBQueue", "m_ReceiveCallFnHMB", "m_ReceiveCallFnQMB" })
public class ServiceCallInfo extends Structure {
    public HMBDSPQueue m_HMBQueue;
    public QMBDSPQueue m_QMBQueue;
    public ReceiveCallFnHMB m_ReceiveCallFnHMB;
    public ReceiveCallFnQMB m_ReceiveCallFnQMB;

    public ServiceCallInfo() {
        super();
    }

    @Override
    public String toString() {
        return this.toString(1);
    }

    public String toString(int indentation) {
        String ws = "\n";
        for (int i = 0; i < indentation; ++i) {
            ws += "\t";
        }

        return "CallInfo" + ws
                + m_HMBQueue.toString(indentation + 1) + ws
                + m_QMBQueue.toString(indentation + 1) + ws
                + "m_ReceiveCallFnHMB: " + m_ReceiveCallFnHMB + ws
                + "m_ReceiveCallFnQMB: " + m_ReceiveCallFnQMB;
    }
}
