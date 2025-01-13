package call.abstract_classes;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

/**
 * struct Call {
 * uint8_t m_CallInfo[SIZE];
 * struct CallMetadata m_CallMetadata;
 * };
 */

@FieldOrder({ "m_CallInfo", "m_CallMetadata" })
public abstract class Call extends Structure {
    public byte[] m_CallInfo;
    public CallMetadata m_Metadata;
}
