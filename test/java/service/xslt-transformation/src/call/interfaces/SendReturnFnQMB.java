package call.interfaces;

import com.sun.jna.Callback;

import call.QMBCall;
import queues.QMBDSPQueue;

public interface SendReturnFnQMB extends Callback {
    int sendQMBReturn(QMBDSPQueue p_Queue, QMBCall p_ReturnInfo);
}
