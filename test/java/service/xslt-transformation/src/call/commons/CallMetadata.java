package call.commons;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct CallMetadata {
 * uint32_t m_Size;
 * uint32_t m_ConnId;
 * bool m_DataReady;
 * };
 */

@FieldOrder({ "m_Size", "m_ConnId", "m_DataReady" })
public class CallMetadata extends Structure {
    public int m_Size;
    public int m_ConnId;
    public byte m_DataReady;
}
