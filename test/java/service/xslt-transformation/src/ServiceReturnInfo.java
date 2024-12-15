import java.util.List;

import com.sun.jna.Callback;
import com.sun.jna.Structure;

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

public class ServiceReturnInfo extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_ResponseQueue", "m_QMBQueue", "m_SendReturnFnQMB");

    public ConnectResponseQueue m_ResponseQueue;
    public QMBDSPQueue m_QMBQueue;
    public SendReturnFnQMB m_SendReturnFnQMB;

    public ServiceReturnInfo() {
        super();
    }

    @Override
    protected List<String> getFieldOrder() {
        return FIELDS;
    }
}
