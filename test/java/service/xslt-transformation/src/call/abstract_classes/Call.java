package call.abstract_classes;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct Call {
 * uint8_t m_CallInfo[SIZE];
 * uint32_t m_Size;
 * uint32_t m_ConnId;
 * bool m_DataReady;
 * };
 */

@FieldOrder({ "m_CallInfo", "m_Size", "m_ConnId", "m_DataReady" })
public abstract class Call extends Structure {
    public byte[] m_CallInfo;
    public int m_Size;
    public int m_ConnId;
    public byte m_DataReady;
}
