package calling.call_package;

import calling.abstract_classes.Call;
import consts.Constants;

public class MBCall extends Call {
    public MBCall() {
        super();
        m_CallInfo = new byte[Constants.MB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
