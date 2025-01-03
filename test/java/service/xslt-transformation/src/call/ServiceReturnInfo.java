package call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import call.interfaces.SendReturnFnQMB;
import connect.ConnectResponseInformation;
import queues.ConnectResponseQueue;
import queues.QMBDSPQueue;

/**
 * struct ServiceReturnInfo {
 * struct ConnectResponseQueue m_ResponseQueue;
 * struct QMBDSPQueue m_QMBQueue;
 * struct ConnectResponseInformation m_ConnectResponseInformation;
 * int32_t (*m_SendReturnFnQMB)(struct QMBDSPQueue *, struct QMBCall *);
 * };
 */

@FieldOrder({ "m_ResponseQueue", "m_QMBQueue", "m_ConnectResponseInformation", "m_SendReturnFnQMB" })
public class ServiceReturnInfo extends Structure {
    public ConnectResponseQueue m_ResponseQueue;
    public QMBDSPQueue m_QMBQueue;
    public ConnectResponseInformation m_ConnectResponseInformation;
    public SendReturnFnQMB m_SendReturnFnQMB;

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
                + "m_SendReturnFnQMB: " + m_SendReturnFnQMB;
    }
}
