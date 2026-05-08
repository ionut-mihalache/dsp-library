package calling.call_package;

import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import calling.interfaces.ReceiveCallFn;
import queues.DSPQueue;

/**
 * struct ServiceCallInfo {
 * struct DSPQueue m_Q;
 * 
 * int32_t (*m_ReceiveCallFn)(struct PopInformation *);
 * };
 */

@FieldOrder({ "m_Q", "m_ReceiveCallFn" })
public class ServiceCallInfo extends Structure {
    public DSPQueue m_Q;
    public ReceiveCallFn m_ReceiveCallFn;

    public ServiceCallInfo() {
        super();
    }

    @Override
    public String toString() {
        return this.toString(1);
    }

    public String toString(int indentation) {
        String ws = "\n";
        for (int i = 0; i < indentation; ++i) {
            ws += "\t";
        }

        return "CallInfo" + ws
                + m_Q.toString(indentation + 1) + ws
                + "m_ReceiveCallFn" + m_ReceiveCallFn;
    }
}
