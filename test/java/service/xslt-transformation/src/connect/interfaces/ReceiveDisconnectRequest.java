package connect.interfaces;

import com.sun.jna.Callback;

import connect.ServiceConnectInfo;

public interface ReceiveDisconnectRequest extends Callback {
    int receiveDisconnectRequest(ServiceConnectInfo p_ConnectInfo);
}
