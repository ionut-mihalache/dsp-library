import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/**
 * struct ServiceConnectInfo {
 * struct ConnectQueue m_Queue;
 * struct ConnectionInformation *m_Connections;
 * int32_t (*m_ReceiveConnectRequest)(struct ServiceReturnInfo *,
 * struct ServiceConnectInfo *);
 * };
 */

interface ReceiveConnectRequest extends Callback {
    int receiveConnectRequest(ServiceReturnInfo p_ReturnInfo, ServiceConnectInfo p_ConnectInfo);
}

public class ServiceConnectInfo extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_Queue", "m_Connections", "m_ReceiveConnectRequest");

    public ConnectQueue m_Queue;
    public Pointer m_Connections;
    public ReceiveConnectRequest m_ReceiveConnectRequest;

    public ServiceConnectInfo() {
        super();
    }

    @Override
    protected List<String> getFieldOrder() {
        return FIELDS;
    }
}
