package calling.return_package;

import calling.abstract_classes.Call;
import consts.Constants;

public class HMBReturn extends Call {
    public HMBReturn() {
        super();
        m_CallInfo = new byte[Constants.HMB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
