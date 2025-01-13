package queues;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import queues.commons.DSPQueueMetadata;

/**
 * struct HMBDSPQueue {
 * struct HMBCall *m_Data;
 * pthread_cond_t *m_FullCond;
 * pthread_cond_t *m_EmptyCond;
 * pthread_mutex_t *m_Lock;
 * uint32_t *m_PushIdxPtr;
 * uint32_t *m_PopIdxPtr;
 * uint32_t *m_Size;
 * };
 */

@FieldOrder({ "m_Metadata", "m_Data" })
public class HMBDSPQueue extends Structure {
    public DSPQueueMetadata m_Metadata;
    public Pointer m_Data;

    public HMBDSPQueue() {
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

        return "HMBDSPQueue" + ws
                + "m_Data: " + m_Data + ws
                + "m_Lock: " + m_Metadata.m_Lock + ws
                + "m_FullCond: " + m_Metadata.m_FullCond + ws
                + "m_EmptyCond: " + m_Metadata.m_EmptyCond + ws
                + "m_PushIdxPtr: " + m_Metadata.m_PushIdxPtr + ws
                + "m_PopIdxPtr: " + m_Metadata.m_PopIdxPtr + ws
                + "m_Size: " + m_Metadata.m_Size;
    }
}
