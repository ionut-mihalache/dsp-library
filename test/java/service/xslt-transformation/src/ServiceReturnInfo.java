import com.sun.jna.Callback;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct ServiceReturnInfo {
 * struct ConnectResponseQueue m_ResponseQueue;
 * struct QMBDSPQueue m_QMBQueue;
 * int32_t (*m_SendReturnFnQMB)(struct QMBDSPQueue *, struct QMBCall *);
 * };
 */

interface SendReturnFnQMB extends Callback {
    int sendQMBReturn(QMBDSPQueue p_Queue, QMBCall p_ReturnInfo);
}

@FieldOrder({ "m_ResponseQueue", "m_QMBQueue", "m_SendReturnFnQMB" })
public class ServiceReturnInfo extends Structure {
    public ConnectResponseQueue m_ResponseQueue;
    public QMBDSPQueue m_QMBQueue;
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
                + "m_SendReturnFnQMB: " + m_SendReturnFnQMB;
    }
}
