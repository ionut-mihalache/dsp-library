import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct HMBCall {
 * uint8_t m_CallInfo[HMB];
 * uint32_t m_Size;
 * bool m_DataReady;
 * };
 */

@FieldOrder({ "m_CallInfo", "m_Size", "m_DataReady" })
public class HMBCall extends Structure {
    public byte[] m_CallInfo;
    public int m_Size;
    public byte m_DataReady;

    public HMBCall() {
        super();
    }
}
