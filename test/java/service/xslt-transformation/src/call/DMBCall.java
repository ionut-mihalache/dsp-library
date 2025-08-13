package call;

import call.abstract_classes.Call;
import consts.Constants;

public class DMBCall extends Call {
    public DMBCall() {
        super();
        m_CallInfo = new byte[Constants.DMB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
