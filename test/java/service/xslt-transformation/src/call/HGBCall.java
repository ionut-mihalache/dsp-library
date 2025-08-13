package call;

import call.abstract_classes.Call;
import consts.Constants;

public class HGBCall extends Call {
    public HGBCall() {
        super();
        m_CallInfo = new byte[Constants.HGB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
