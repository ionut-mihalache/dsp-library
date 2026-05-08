package connect;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import connect.interfaces.ReceiveConnectRequest;
import connect.interfaces.ReceiveDisconnectRequest;
import queues.ConnectQueue;
import queues.DisconnectQueue;

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

@FieldOrder({ "m_ConnectQ", "m_DisconnectQ", "m_Connections", "m_ConnectLock", "m_ReceiveConnectRequest",
        "m_ReceiveDisconnectRequest" })
public class ServiceConnectInfo extends Structure {
    public ConnectQueue m_ConnectQ;
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
                + m_ConnectQ.toString(indentation + 1) + ws
                + m_DisconnectQ.toString(indentation + 1) + ws
                + "m_Connections: " + m_Connections + ws
                + "m_ConnectLock: " + m_ConnectLock + ws
                + "m_ReceiveConnectRequest: " + m_ReceiveConnectRequest + ws
                + "m_ReceiveDisconnectRequest: " + m_ReceiveDisconnectRequest;
    }
}
