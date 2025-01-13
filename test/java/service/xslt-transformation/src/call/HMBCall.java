package call;

import call.abstract_classes.Call;
import consts.Constants;

public class HMBCall extends Call {
    public HMBCall() {
        super();
        m_CallInfo = new byte[Constants.HMB];
        m_CallMetadata.m_Size = 0;
        m_CallMetadata.m_ConnId = 0;
        m_CallMetadata.m_DataReady = 0;
    }
}
