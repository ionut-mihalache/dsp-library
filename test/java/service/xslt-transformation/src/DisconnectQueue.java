import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/**
 * struct DisconnectQueue {
 * struct ConnectRequest *m_Data;
 * pthread_cond_t *m_FullCond;
 * pthread_cond_t *m_EmptyCond;
 * pthread_mutex_t *m_Lock;
 * uint32_t *m_PushIdxPtr;
 * uint32_t *m_PopIdxPtr;
 * uint32_t *m_Size;
 * };
 */
public class DisconnectQueue extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_Data", "m_FullCond", "m_EmptyCond", "m_Lock",
            "m_PushIdxPtr", "m_PopIdxPtr", "m_Size");

    public Pointer m_Data;
    public Pointer m_FullCond;
    public Pointer m_EmptyCond;
    public Pointer m_Lock;
    public Pointer m_PushIdxPtr;
    public Pointer m_PopIdxPtr;
    public Pointer m_Size;

    public DisconnectQueue() {
        super();
    }

    @Override
    protected List<String> getFieldOrder() {
        return FIELDS;
    }
}
