package call.interfaces;

import com.sun.jna.Callback;
import com.sun.jna.Pointer;

import queues.DSPQueue;

public interface SendReturnFn extends Callback {
    int f_SendReturn(DSPQueue p_Queue, Pointer p_ReturnInfo);
}
