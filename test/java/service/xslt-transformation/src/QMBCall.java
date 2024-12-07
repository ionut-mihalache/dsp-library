import java.util.List;

import com.sun.jna.Structure;

/**
 *
 * struct QMBCall {
        uint8_t m_CallInfo[QMB];
        uint32_t m_Size;
        bool m_DataReady;
    };
 */

public class QMBCall extends Structure {
    public static final List<String> FIELDS = createFieldsOrder("m_CallInfo", "m_Size", "m_DataReady");

    public byte[] m_CallInfo;
    public int m_Size;
    public byte m_DataReady;

    public QMBCall() {
        super();
        m_CallInfo = new byte[Constants.QMB];
        m_Size = 0;
        m_DataReady = 0;
    }

    @Override
    protected List<String> getFieldOrder() {
        return FIELDS;
    }
}
