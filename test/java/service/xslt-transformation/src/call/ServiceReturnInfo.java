package call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import call.interfaces.SendReturnFn;
import connect.ConnectResponseInformation;
import queues.ConnectResponseQueue;
import queues.DSPQueue;

/**
 * struct ServiceReturnInfo {
 * struct ConnectResponseQueue m_ResponseQueue;
 * struct ConnectResponseInformation m_ConnectResponseInformation;
 * 
 * struct DSPQueue m_Q;
 * 
 * int32_t (*m_SendReturnFn)(struct PushInformation *);
 * };
 */

@FieldOrder({ "m_ResponseQueue", "m_ConnectResponseInformation", "m_Q", "m_SendReturnFn" })
public class ServiceReturnInfo extends Structure {
    public ConnectResponseQueue m_ResponseQueue;
    public ConnectResponseInformation m_ConnectResponseInformation;
    public DSPQueue m_Q;
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
                + m_ConnectResponseInformation.toString(indentation + 1) + ws
                + m_Q.toString(indentation + 1) + ws
                + "m_SendReturnFn: " + m_SendReturnFn;
    }
}
