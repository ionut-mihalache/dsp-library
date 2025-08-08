package call.interfaces;

import com.sun.jna.Callback;

import call.HMBCall;
import queues.HMBDSPQueue;

public interface SendReturnFnHMB extends Callback {
    int sendQMBReturn(HMBDSPQueue p_Queue, HMBCall p_ReturnInfo);
}
