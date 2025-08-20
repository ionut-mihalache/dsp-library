package calling.call_package;

import calling.abstract_classes.Call;
import consts.Constants;

public class HMBCall extends Call {
    public HMBCall() {
        super();
        m_CallInfo = new byte[Constants.HMB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
