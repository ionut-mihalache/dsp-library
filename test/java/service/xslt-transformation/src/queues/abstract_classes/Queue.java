package queues.abstract_classes;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

@FieldOrder({ "m_Data", "m_FullCond", "m_EmptyCond", "m_Lock", "m_PushIdxPtr", "m_PopIdxPtr", "m_Size" })
public class Queue extends Structure {
    public Pointer m_Data;
    public Pointer m_FullCond;
    public Pointer m_EmptyCond;
    public Pointer m_Lock;
    public Pointer m_PushIdxPtr;
    public Pointer m_PopIdxPtr;
    public Pointer m_Size;

    public Queue() {
        super();
    }
}
