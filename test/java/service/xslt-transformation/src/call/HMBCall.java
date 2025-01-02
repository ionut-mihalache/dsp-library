package call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import consts.Constants;

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
        m_CallInfo = new byte[Constants.HMB];
        m_Size = 0;
        m_DataReady = 0;
    }
}
