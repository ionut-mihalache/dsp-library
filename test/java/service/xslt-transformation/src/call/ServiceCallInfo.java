package call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import call.interfaces.ReceiveCallFn;
import call.interfaces.ReceiveCallFnHMB;
import call.interfaces.ReceiveCallFnQMB;

import queues.HMBDSPQueue;
import queues.QMBDSPQueue;
import queues.DSPQueue;

/**
 * struct ServiceCallInfo {
 * struct HMBDSPQueue m_HMBQueue;
 * struct QMBDSPQueue m_QMBQueue;
 * // v0.0.2
 * struct DSPQueue m_Q;
 *
 * int32_t (*m_ReceiveCallFnHMB)(struct HMBCall *, struct HMBDSPQueue *);
 * int32_t (*m_ReceiveCallFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
 *
 * // v0.0.2
 * int32_t (*m_ReceiveCallFn)(struct PopInformation *);
 * };
 */

@FieldOrder({ "m_HMBQueue", "m_QMBQueue", "m_Q", "m_ReceiveCallFnHMB", "m_ReceiveCallFnQMB", "m_ReceiveCallFn" })
public class ServiceCallInfo extends Structure {
    public HMBDSPQueue m_HMBQueue;
    public QMBDSPQueue m_QMBQueue;
    public DSPQueue m_Q;
    public ReceiveCallFnHMB m_ReceiveCallFnHMB;
    public ReceiveCallFnQMB m_ReceiveCallFnQMB;
    public ReceiveCallFn m_ReceiveCallFn;

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
                + m_Q.toString(indentation + 1) + ws
                + "m_ReceiveCallFnHMB: " + m_ReceiveCallFnHMB + ws
                + "m_ReceiveCallFnQMB: " + m_ReceiveCallFnQMB + ws
                + "m_ReceiveCallFn" + m_ReceiveCallFn;
    }
}
