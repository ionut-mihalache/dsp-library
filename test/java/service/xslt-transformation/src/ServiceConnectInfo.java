import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct ServiceConnectInfo {
 * struct ConnectQueue m_Queue;
 * struct DisconnectQueue m_DisconnectQ;
 * struct ConnectionInformation *m_Connections;
 * pthread_spinlock_t *m_ConnectLock;
 * int32_t (*m_ReceiveConnectRequest)(struct ServiceReturnInfo *,
 * struct ServiceConnectInfo *);
 * int32_t (*m_ReceiveDisconnectRequest)(struct ServiceConnectInfo *);
 * };
 */

interface ReceiveConnectRequest extends Callback {
    int receiveConnectRequest(ServiceReturnInfo p_ReturnInfo, ServiceConnectInfo p_ConnectInfo);
}

interface ReceiveDisconnectRequest extends Callback {
    int receiveDisconnectRequest(ServiceConnectInfo p_ConnectInfo);
}

@FieldOrder({ "m_Queue", "m_DisconnectQ", "m_Connections", "m_ConnectLock", "m_ReceiveConnectRequest",
        "m_ReceiveDisconnectRequest" })
public class ServiceConnectInfo extends Structure {
    public ConnectQueue m_Queue;
    public DisconnectQueue m_DisconnectQ;
    public Pointer m_Connections;
    public Pointer m_ConnectLock;
    public ReceiveConnectRequest m_ReceiveConnectRequest;
    public ReceiveDisconnectRequest m_ReceiveDisconnectRequest;

    public ServiceConnectInfo() {
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

        return "ConnectInfo" + ws
                + m_Queue.toString(indentation + 1) + ws
                + m_DisconnectQ.toString(indentation + 1) + ws
                + "m_Connections: " + m_Connections + ws
                + "m_ConnectLock: " + m_ConnectLock + ws
                + "m_ReceiveConnectRequest" + m_ReceiveConnectRequest + ws
                + "m_ReceiveDisconnectRequest" + m_ReceiveDisconnectRequest;
    }
}
