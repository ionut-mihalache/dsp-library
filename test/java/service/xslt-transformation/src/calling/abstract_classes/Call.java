package calling.abstract_classes;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import calling.commons.CallMetadata;

/**
 * struct Call {
 * uint8_t m_CallInfo[SIZE];
 * struct CallMetadata m_CallMetadata;
 * };
 */

@FieldOrder({ "m_CallInfo", "m_Metadata" })
public abstract class Call extends Structure {
    public byte[] m_CallInfo;
    public CallMetadata m_Metadata;
}
