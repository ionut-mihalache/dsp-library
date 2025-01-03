package queues;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct ConnectResponseQueue {
 * struct ConnectResponseInformation *m_Data;
 * pthread_cond_t *m_FullCond;
 * pthread_cond_t *m_EmptyCond;
 * pthread_mutex_t *m_Lock;
 * uint32_t *m_PushIdxPtr;
 * uint32_t *m_PopIdxPtr;
 * uint32_t *m_Size;
 * uint32_t m_MaxSize;
 * };
 */

@FieldOrder({ "m_Data", "m_FullCond", "m_EmptyCond", "m_Lock", "m_PushIdxPtr", "m_PopIdxPtr", "m_Size", "m_MaxSize" })
public class ConnectResponseQueue extends Structure {
    public Pointer m_Data;
    public Pointer m_FullCond;
    public Pointer m_EmptyCond;
    public Pointer m_Lock;
    public Pointer m_PushIdxPtr;
    public Pointer m_PopIdxPtr;
    public Pointer m_Size;
    public int m_MaxSize;

    public ConnectResponseQueue() {
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

        return "ConnectResponseQueue" + ws
                + "m_Data: " + m_Data + ws
                + "m_Lock: " + m_Lock + ws
                + "m_FullCond: " + m_FullCond + ws
                + "m_EmptyCond: " + m_EmptyCond + ws
                + "m_PushIdxPtr: " + m_PushIdxPtr + ws
                + "m_PopIdxPtr: " + m_PopIdxPtr + ws
                + "m_Size: " + m_Size + ws
                + "m_MaxSize: " + m_MaxSize;
    }
}
