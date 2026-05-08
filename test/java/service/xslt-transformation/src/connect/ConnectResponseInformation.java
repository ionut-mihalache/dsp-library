package connect;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

// import com.sun.jna.Pointer;
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
@FieldOrder({ "m_ReturnQName", "m_ReturnRequestQName", "m_Id" })
public class ConnectResponseInformation extends Structure {
    public byte[] m_ReturnQName = new byte[Constants.RETURNQ_NAME_MAX_SIZE];
    public byte[] m_ReturnRequestQName = new byte[Constants.RETURNQ_NAME_MAX_SIZE];
    public int m_Id = 0;

    public ConnectResponseInformation() {
        super();
    }

    private String byteArrToString(byte[] p_Arr) {
        ByteBuffer buf = ByteBuffer.wrap(p_Arr);

        return StandardCharsets.UTF_8.decode(buf.slice(0, Constants.RETURNQ_NAME_MAX_SIZE)).toString();
    }

    public String getReturnQName() {
        return this.byteArrToString(m_ReturnQName);
    }

    public String getReturnRequestQName() {
        return this.byteArrToString(m_ReturnRequestQName);
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
                + "m_ReturnQName: " + getReturnQName() + ws
                + "m_ReturnRequestQName: " + getReturnRequestQName() + ws
                + "m_Id: " + m_Id;
    }
}
