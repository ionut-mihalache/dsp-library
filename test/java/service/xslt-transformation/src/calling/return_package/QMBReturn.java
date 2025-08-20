package calling.return_package;

import calling.abstract_classes.Call;
import consts.Constants;

public class QMBReturn extends Call {
    public QMBReturn() {
        super();
        m_CallInfo = new byte[Constants.QMB];
        m_Metadata.m_Size = 0;
        m_Metadata.m_ConnId = 0;
        m_Metadata.m_DataReady = 0;
    }
}
