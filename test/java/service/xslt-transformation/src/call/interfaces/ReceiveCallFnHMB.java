package call.interfaces;

import com.sun.jna.Callback;

import call.HMBCall;
import queues.HMBDSPQueue;

public interface ReceiveCallFnHMB extends Callback {
    int m_ReceiveCallFnHMB(HMBCall p_CallInfo, HMBDSPQueue p_Queue);
}
