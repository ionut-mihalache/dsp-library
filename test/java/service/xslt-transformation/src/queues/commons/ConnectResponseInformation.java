package queues.commons;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Structure.FieldOrder;

import consts.Constants;

/**
 * struct ConnectResponseInformation {
 * char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
 * char m_ReturnRequestQName[RETURNQ_NAME_MAX_SIZE];
 * uint32_t m_Id;
 * };
 */
@FieldOrder({ "m_ReturnQ", "m_ReturnRequestQName", "m_Id" })
public class ConnectResponseInformation extends Structure {
    public byte[] m_ReturnQ;
    public byte[] m_ReturnRequestQName;
    public int m_Id;

    public ConnectResponseInformation() {
        super();
        m_ReturnQ = new byte[Constants.RETURNQ_NAME_MAX_SIZE];
        m_ReturnRequestQName = new byte[Constants.RETURNQ_NAME_MAX_SIZE];
    }

    public ConnectResponseInformation(Pointer p_P) {
        super(p_P);
        m_ReturnQ = new byte[Constants.RETURNQ_NAME_MAX_SIZE];
        m_ReturnRequestQName = new byte[Constants.RETURNQ_NAME_MAX_SIZE];
        read();
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

        return "ConnectResponseInformation" + ws
                + "m_ReturnQ: " + m_ReturnQ + ws
                + "m_ReturnRequestQName: " + m_ReturnRequestQName + ws
                + "m_Id: " + m_Id;
    }
}
