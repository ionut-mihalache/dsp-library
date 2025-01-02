package connect.interfaces;

import com.sun.jna.Callback;

import call.ServiceReturnInfo;
import connect.ServiceConnectInfo;

public interface ReceiveConnectRequest extends Callback {
    int receiveConnectRequest(ServiceReturnInfo p_ReturnInfo, ServiceConnectInfo p_ConnectInfo);
}
