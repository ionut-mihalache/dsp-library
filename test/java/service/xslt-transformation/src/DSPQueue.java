import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

/**
 * Based on the C library version:
    struct Queue {
        pthread_mutex_t *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        char *m_Start;
    };
*/

public class DSPQueue extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_Lock", "m_PushIdxPtr", "m_PopIdxPtr", "m_Start");

    public Pointer m_Lock;
    public Pointer m_PushIdxPtr;
    public Pointer m_PopIdxPtr;
    public Pointer m_Start;

    public DSPQueue() {
        super();
        m_PushIdxPtr = null;
        m_PopIdxPtr = null;
        m_Start = null;
    }

    @Override
    protected List<String> getFieldOrder() {
        return FIELDS;
    }

    @Override
    public String toString() {
        return "DSPQueue: " + m_PushIdxPtr.getInt(0) + " " + m_PopIdxPtr.getInt(0) + " " + m_Start;
    }
}
