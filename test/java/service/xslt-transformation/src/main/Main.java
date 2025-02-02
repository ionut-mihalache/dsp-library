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

import call.QMBCall;
import call.ServiceCallInfo;
import call.ServiceReturnInfo;
import connect.ServiceConnectInfo;

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP) Native.load("dsp", LibDSP.class);

    void dspInstall(ServiceConnectInfo p_ConnectInfo, ServiceCallInfo p_CallInfo, String p_StrId, String p_Version);
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

public class Main {
    public static void main(String[] args) {
        Main main = new Main();
        ServiceConnectInfo connectInfo = new ServiceConnectInfo();
        ServiceCallInfo callInfo = new ServiceCallInfo();
        HashMap<Integer, ServiceReturnInfo> connections = new HashMap<Integer, ServiceReturnInfo>();

        LibDSP.INSTANCE.dspInstall(connectInfo, callInfo, "xslt-transformation", "v0.0.1");

        ConnectThread connectThread = new ConnectThread(connectInfo, connections);
        DisconnectThread disconnectThread = new DisconnectThread(connectInfo);

        connectThread.start();
        disconnectThread.start();

        while (true) {
            try {
                QMBCall callData = new QMBCall();
                callInfo.m_ReceiveCallFnQMB.m_ReceiveCallFnQMB(callData,
                        callInfo.m_QMBQueue);

                byte[] iiaData = Arrays.copyOfRange(callData.m_CallInfo, 0,
                        callData.m_Metadata.m_Size);

                Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
                byte[] xsltData = Files.readAllBytes(xsltPath);

                String result = main.getXmlTransformed(iiaData, xsltData);

                byte[] resByteArr = result.getBytes();

                QMBCall returnData = new QMBCall();
                System.arraycopy(resByteArr, 0, returnData.m_CallInfo, 0, resByteArr.length);
                returnData.m_Metadata.m_Size = resByteArr.length;
                returnData.m_Metadata.m_ConnId = callData.m_Metadata.m_ConnId;

                connections.get(callData.m_Metadata.m_ConnId).m_SendReturnFnQMB
                        .sendQMBReturn(connections.get(callData.m_Metadata.m_ConnId).m_QMBQueue,
                                returnData);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Transform a xml file by means of a xslt file
     *
     * @param xmlBytes  The content of the xml file
     * @param xsltBytes The content of the xslt file
     * @return A xml useful to compute the hash code
     */
    public String getXmlTransformed(byte[] xmlBytes, byte[] xsltBytes) throws Exception {
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
};
