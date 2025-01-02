package call.interfaces;

import com.sun.jna.Callback;

import call.QMBCall;
import queues.QMBDSPQueue;

public interface ReceiveCallFnQMB extends Callback {
    int m_ReceiveCallFnQMB(QMBCall p_CallInfo, QMBDSPQueue p_Queue);
}
