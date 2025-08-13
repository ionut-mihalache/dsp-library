package queues;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import queues.commons.DSPQueueMetadata;

/**
 * struct DSPQueue {
 * struct DSPQueueMetadata m_Metadata;
 * void *m_Data;
 * uint32_t m_MaxSize;
 * int m_Type;
 * };
 */

@FieldOrder({ "m_Metadata", "m_Data", "m_MaxSize", "m_Type" })
public class DSPQueue extends Structure {
    public DSPQueueMetadata m_Metadata;
    public Pointer m_Data;
    public int m_MaxSize;
    public int m_Type;

    public DSPQueue() {
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

        return "DSPQueue" + ws
                + "m_Data: " + m_Data + ws
                + "m_Lock: " + m_Metadata.m_Lock + ws
                + "m_FullCond: " + m_Metadata.m_FullCond + ws
                + "m_EmptyCond: " + m_Metadata.m_EmptyCond + ws
                + "m_PushIdxPtr: " + m_Metadata.m_PushIdxPtr + ws
                + "m_PopIdxPtr: " + m_Metadata.m_PopIdxPtr + ws
                + "m_Size: " + m_Metadata.m_Size + ws
                + "m_MaxSize: " + m_MaxSize + ws
                + "m_Type: " + m_Type;
    }
}
