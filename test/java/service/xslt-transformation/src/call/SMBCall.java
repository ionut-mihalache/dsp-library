package call;

import call.abstract_classes.Call;
import consts.Constants;

public class SMBCall extends Call {
    public SMBCall() {
        super();
        m_CallInfo = new byte[Constants.SMB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
