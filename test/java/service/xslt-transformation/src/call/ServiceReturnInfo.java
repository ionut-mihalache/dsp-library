package call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import call.interfaces.SendReturnFn;
import call.interfaces.SendReturnFnHMB;
import call.interfaces.SendReturnFnQMB;
import connect.ConnectResponseInformation;
import queues.ConnectResponseQueue;
import queues.DSPQueue;
import queues.QMBDSPQueue;

/**
 * struct ServiceReturnInfo {
 * struct ConnectResponseQueue m_ResponseQueue;
 * struct QMBDSPQueue m_QMBQueue;
 * struct ConnectResponseInformation m_ConnectResponseInformation;
 * 
 * // v0.0.2
 * struct DSPQueue m_Q;
 * 
 * int32_t (*m_SendReturnFnQMB)(struct DSPQueue *, struct QMBCall *);
 * int32_t (*m_SendReturnFnHMB)(struct DSPQueue *, struct HMBCall *);
 * 
 * // v0.0.2
 * int32_t (*m_SendReturnFn)(struct PushInformation *);
 * };
 */

@FieldOrder({ "m_ResponseQueue", "m_QMBQueue", "m_ConnectResponseInformation", "m_Q", "m_SendReturnFnQMB",
        "m_SendReturnFnHMB", "m_SendReturnFn" })
public class ServiceReturnInfo extends Structure {
    public ConnectResponseQueue m_ResponseQueue;
    public QMBDSPQueue m_QMBQueue;
    public ConnectResponseInformation m_ConnectResponseInformation;
    public DSPQueue m_Q;
    public SendReturnFnQMB m_SendReturnFnQMB;
    public SendReturnFnHMB m_SendReturnFnHMB;
    public SendReturnFn m_SendReturnFn;

    public ServiceReturnInfo() {
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

        return "ReturnInfo" + ws
                + m_ResponseQueue.toString(indentation + 1) + ws
                + m_QMBQueue.toString(indentation + 1) + ws
                + m_ConnectResponseInformation.toString(indentation + 1) + ws
                + m_Q.toString(indentation + 1) + ws
                + "m_SendReturnFnQMB: " + m_SendReturnFnQMB + ws
                + "m_SendReturnFnHMB: " + m_SendReturnFnHMB + ws
                + "m_SendReturnFn: " + m_SendReturnFn;
    }
}
