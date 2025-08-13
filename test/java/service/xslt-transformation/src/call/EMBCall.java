package call;

import call.abstract_classes.Call;
import consts.Constants;

public class EMBCall extends Call {
    public EMBCall() {
        super();
        m_CallInfo = new byte[Constants.EMB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
