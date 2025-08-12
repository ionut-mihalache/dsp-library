package main;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;

import org.w3c.dom.Document;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

import call.QMBCall;
import call.ServiceCallInfo;
import call.ServiceReturnInfo;
import connect.ServiceConnectInfo;

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP) Native.load("dsp", LibDSP.class);

    void dspInstall(ServiceConnectInfo p_ConnectInfo, ServiceCallInfo p_CallInfo, String p_StrId, String p_Version,
            int p_CallQType);

    void receiveCall(Pointer p_CallData, ServiceCallInfo p_CallInfo);

    void sendReturn(ServiceReturnInfo p_ReturnInfo, Pointer p_ReturnData);
}

class ConnectThread extends Thread {
    private ServiceConnectInfo m_ConnectInfo;
    private HashMap<Integer, ServiceReturnInfo> m_Connections;

    ConnectThread(ServiceConnectInfo p_ConnectInfo, HashMap<Integer, ServiceReturnInfo> p_Connections) {
        m_ConnectInfo = p_ConnectInfo;
        m_Connections = p_Connections;
    }

    public void run() {
        while (true) {
            ServiceReturnInfo returnInfo = new ServiceReturnInfo();

            m_ConnectInfo.m_ReceiveConnectRequest.receiveConnectRequest(returnInfo, m_ConnectInfo);

            int connId = returnInfo.m_ResponseQueue.m_Data.getInt(512);
            m_Connections.put(connId, returnInfo);
        }
    }
}

class DisconnectThread extends Thread {
    private ServiceConnectInfo m_ConnectInfo;

    DisconnectThread(ServiceConnectInfo p_ConnectInfo) {
        m_ConnectInfo = p_ConnectInfo;
    }

    public void run() {
        while (true) {
            m_ConnectInfo.m_ReceiveDisconnectRequest.receiveDisconnectRequest(m_ConnectInfo);
        }
    }
}

class ProcessCallThread extends Thread {
    private QMBCall m_CallData;
    private HashMap<Integer, ServiceReturnInfo> m_Connections;

    ProcessCallThread(QMBCall p_CallData, HashMap<Integer, ServiceReturnInfo> p_Connections) {
        m_CallData = p_CallData;
        m_Connections = p_Connections;
    }

    public void run() {
        try {
            System.out.println(m_CallData);
            byte[] iiaData = Arrays.copyOfRange(m_CallData.m_CallInfo, 0,
                    m_CallData.m_Metadata.m_Size);
            System.out.println("Data size is: " + m_CallData.m_Metadata.m_Size);

            Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            String result = mf_GetXmlTransformed(iiaData, xsltData);

            byte[] resByteArr = result.getBytes();

            QMBCall returnData = new QMBCall();
            System.arraycopy(resByteArr, 0, returnData.m_CallInfo, 0, resByteArr.length);
            returnData.m_Metadata.m_Size = resByteArr.length;
            returnData.m_Metadata.m_ConnId = m_CallData.m_Metadata.m_ConnId;

            returnData.write();
            LibDSP.INSTANCE.sendReturn(m_Connections.get(m_CallData.m_Metadata.m_ConnId), returnData.getPointer());

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Transform a xml file by means of a xslt file
     *
     * @param xmlBytes  The content of the xml file
     * @param xsltBytes The content of the xslt file
     * @return A xml useful to compute the hash code
     */
    private String mf_GetXmlTransformed(byte[] xmlBytes, byte[] xsltBytes) throws Exception {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(true);

        DocumentBuilder db = dbf.newDocumentBuilder();
        Document document = db.parse(new ByteArrayInputStream(xmlBytes));

        System.setProperty("javax.xml.transform.TransformerFactory", "net.sf.saxon.TransformerFactoryImpl");
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        Transformer transformer = transformerFactory.newTransformer(
                new StreamSource(new ByteArrayInputStream(xsltBytes)));
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        transformer.transform(new DOMSource(document), new StreamResult(output));

        return output.toString();
    }
}

public class Main {
    public static void main(String[] args) {
        ServiceConnectInfo connectInfo = new ServiceConnectInfo();
        ServiceCallInfo callInfo = new ServiceCallInfo();
        HashMap<Integer, ServiceReturnInfo> connections = new HashMap<Integer, ServiceReturnInfo>();

        LibDSP.INSTANCE.dspInstall(connectInfo, callInfo, "xslt-transformation", "v0.0.1", 2);

        ConnectThread connectThread = new ConnectThread(connectInfo, connections);
        DisconnectThread disconnectThread = new DisconnectThread(connectInfo);

        connectThread.start();
        disconnectThread.start();

        while (true) {
            try {
                QMBCall callData = new QMBCall();

                LibDSP.INSTANCE.receiveCall(callData.getPointer(), callInfo);

                callData.read();

                ProcessCallThread processCallThread = new ProcessCallThread(callData,
                        connections);
                processCallThread.start();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
};
