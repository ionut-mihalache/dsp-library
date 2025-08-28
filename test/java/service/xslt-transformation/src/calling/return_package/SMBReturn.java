package calling.return_package;

import consts.Constants;

import calling.commons.CallMetadata;
import calling.interfaces.Call;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

@FieldOrder({ "m_CallInfo", "m_Metadata" })
public class SMBReturn extends Structure implements Call {
    public byte[] m_CallInfo = new byte[Constants.SMB];
    public CallMetadata m_Metadata;

    public SMBReturn() {
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }

    @Override
    public byte[] getCallInfo() {
        return m_CallInfo;
    }

    @Override
    public CallMetadata getMetadata() {
        return m_Metadata;
    }
}
