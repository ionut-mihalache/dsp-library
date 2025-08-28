package calling.interfaces;

import com.sun.jna.Pointer;

import calling.commons.CallMetadata;

/**
 * struct Call {
 * uint8_t m_CallInfo[SIZE];
 * struct CallMetadata m_CallMetadata;
 * };
 */

public interface Call {
    void read();

    void write();

    Pointer getPointer();

    byte[] getCallInfo();

    CallMetadata getMetadata();
}
