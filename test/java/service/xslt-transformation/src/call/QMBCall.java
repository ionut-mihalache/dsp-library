package call;

import call.abstract_classes.Call;
import consts.Constants;

public class QMBCall extends Call {
    public QMBCall() {
        super();
        m_CallInfo = new byte[Constants.QMB];
        m_CallMetadata.m_Size = 0;
        m_CallMetadata.m_ConnId = 0;
        m_CallMetadata.m_DataReady = 0;
    }
}
