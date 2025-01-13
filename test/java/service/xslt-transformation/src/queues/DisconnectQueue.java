package queues;

import queues.abstract_classes.DSPQueue;

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
public class DisconnectQueue extends DSPQueue {
    public DisconnectQueue() {
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

        return "DisconnectQueue" + ws
                + "m_Data: " + m_Data + ws
                + "m_Lock: " + m_Lock + ws
                + "m_FullCond: " + m_FullCond + ws
                + "m_EmptyCond: " + m_EmptyCond + ws
                + "m_PushIdxPtr: " + m_PushIdxPtr + ws
                + "m_PopIdxPtr: " + m_PopIdxPtr + ws
                + "m_Size: " + m_Size;
    }
}
