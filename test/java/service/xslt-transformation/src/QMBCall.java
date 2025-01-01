import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct QMBCall {
 * uint8_t m_CallInfo[QMB];
 * uint32_t m_Size;
 * bool m_DataReady;
 * };
 */

@FieldOrder({ "m_CallInfo", "m_Size", "m_DataReady" })
public class QMBCall extends Structure {
    public byte[] m_CallInfo;
    public int m_Size;
    public byte m_DataReady;

    public QMBCall() {
        super();
        m_CallInfo = new byte[Constants.QMB];
        m_Size = 0;
        m_DataReady = 0;
    }
}
