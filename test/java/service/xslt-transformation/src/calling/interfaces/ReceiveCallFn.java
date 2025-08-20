package calling.interfaces;

import com.sun.jna.Callback;
import com.sun.jna.Pointer;

import queues.DSPQueue;

public interface ReceiveCallFn extends Callback {
    int f_ReceiveCall(Pointer p_CallInfo, DSPQueue p_Queue);
}
