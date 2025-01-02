package queues;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * Based on the C library version:
 * struct DSPQueue {
 * pthread_cond_t *m_FullCond;
 * pthread_cond_t *m_EmptyCond;
 * pthread_mutex_t *m_Lock;
 * uint32_t *m_PushIdxPtr;
 * uint32_t *m_PopIdxPtr;
 * char *m_Start;
 * };
 */

@FieldOrder({ "m_FullCond", "m_EmptyCond", "m_Lock", "m_PushIdxPtr", "m_PopIdxPtr", "m_Start" })
public class DSPQueue extends Structure {
    public Pointer m_FullCond;
    public Pointer m_EmptyCond;
    public Pointer m_Lock;
    public Pointer m_PushIdxPtr;
    public Pointer m_PopIdxPtr;
    public Pointer m_Start;

    public DSPQueue() {
        super();
    }
}
