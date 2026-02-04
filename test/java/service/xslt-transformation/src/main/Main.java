package main;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;

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

import calling.call_package.SMBCall;
import calling.call_package.ServiceCallInfo;
import calling.interfaces.Call;
import calling.return_package.*;
import connect.ServiceConnectInfo;

import consts.Constants;
import queues.commons.ConnectResponseInformation;

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP) Native.load("dsp", LibDSP.class);

    void dspInstall(ServiceConnectInfo p_ConnectInfo, ServiceCallInfo p_CallInfo, String p_StrId, String p_Version,
            int p_CallQType);

    void receiveCall(Pointer p_CallData, ServiceCallInfo p_CallInfo);

    void sendReturn(ServiceReturnInfo p_ReturnInfo, Pointer p_ReturnData);
}

class ConnectThread extends Thread {
    private final ServiceConnectInfo m_ConnectInfo;
    private final ConcurrentHashMap<Integer, ServiceReturnInfo> m_Connections;

    ConnectThread(ServiceConnectInfo p_ConnectInfo, ConcurrentHashMap<Integer, ServiceReturnInfo> p_Connections) {
        m_ConnectInfo = p_ConnectInfo;
        m_Connections = p_Connections;
    }

    public void run() {
        while (!Thread.currentThread().isInterrupted()) {
            ServiceReturnInfo returnInfo = new ServiceReturnInfo();

            m_ConnectInfo.m_ReceiveConnectRequest.receiveConnectRequest(returnInfo, m_ConnectInfo);

            returnInfo.read();

            Pointer nativePtr = returnInfo.m_ResponseQueue.m_Data;
            if (nativePtr == null) {
                System.err.println("ConnectThread: m_Data pointer is NULL");
                continue;
            }

            int size = new ConnectResponseInformation().size();

            byte[] buf = nativePtr.getByteArray(0, size);

            ConnectResponseInformation responseInfo = new ConnectResponseInformation();
            responseInfo.getPointer().write(0, buf, 0, size);
            responseInfo.read();
            // ConnectResponseInformation responseInfo = new ConnectResponseInformation(
            // returnInfo.m_ResponseQueue.m_Data.share(0));

            int connId = responseInfo.m_Id;
            m_Connections.put(connId, returnInfo);
        }
    }
}

class DisconnectThread extends Thread {
    private final ServiceConnectInfo m_ConnectInfo;

    DisconnectThread(ServiceConnectInfo p_ConnectInfo) {
        m_ConnectInfo = p_ConnectInfo;
    }

    public void run() {
        while (!Thread.currentThread().isInterrupted()) {
            m_ConnectInfo.m_ReceiveDisconnectRequest.receiveDisconnectRequest(m_ConnectInfo);
        }
    }
}

class ProcessCallThread extends Thread {
    private final Call m_CallData;
    private final ConcurrentHashMap<Integer, ServiceReturnInfo> m_Connections;
    private final ServiceConnectInfo m_ConnectInfo;

    ProcessCallThread(Call p_CallData, ConcurrentHashMap<Integer, ServiceReturnInfo> p_Connections, ServiceConnectInfo p_ConnectInfo) {
        m_CallData = p_CallData;
        m_Connections = p_Connections;
        m_ConnectInfo = p_ConnectInfo;
    }

    public void run() {
        try {
            byte[] iiaData = Arrays.copyOfRange(m_CallData.getCallInfo(), 0, m_CallData.getMetadata().m_Size);

            Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            String result = mf_GetXmlTransformed(iiaData, xsltData);

            byte[] resByteArr = result.getBytes(StandardCharsets.UTF_8);

            ServiceReturnInfo serviceReturnInfo = m_Connections.get(m_CallData.getMetadata().m_ConnId);

            // long returnDataSegmentSize = (new SMBReturn()).size();
            // Memory nativeMem = new Memory(returnDataSegmentSize);

            // System.out.println("SMBReturn size: " + returnDataSegmentSize);

            Call returnData = switch (serviceReturnInfo.m_Q.m_Type) {
                case Constants.SMBQ -> new SMBReturn();
                case Constants.EMBQ -> new EMBReturn();
                case Constants.QMBQ -> new QMBReturn();
                case Constants.HMBQ -> new HMBReturn();
                case Constants.MBQ -> new MBReturn();
                case Constants.DMBQ -> new DMBReturn();
                case Constants.HGBQ -> new HGBReturn();
                case Constants.GBQ -> new GBReturn();
                default -> {
                    System.err.println("Return queue type not recognized!");
                    yield null;
                }
            };

            if (returnData == null) {
                System.err.println("Return data type not recognized!");
                return;
            }

            System.arraycopy(resByteArr, 0, returnData.getCallInfo(), 0, resByteArr.length);
            returnData.getMetadata().m_Size = resByteArr.length;
            returnData.getMetadata().m_ConnId = m_CallData.getMetadata().m_ConnId;

            returnData.write();
            // nativeMem.write(0, returnData.getPointer().getByteArray(0, (int)
            // returnDataSegmentSize), 0,
            // (int) returnDataSegmentSize);
            LibDSP.INSTANCE.sendReturn(serviceReturnInfo, returnData.getPointer());

            m_ConnectInfo.m_ReceiveDisconnectRequest.receiveDisconnectRequest(m_ConnectInfo);
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
        Transformer transformer = transformerFactory
                .newTransformer(new StreamSource(new ByteArrayInputStream(xsltBytes)));
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        transformer.transform(new DOMSource(document), new StreamResult(output));

        return output.toString();
    }
}

class ProcessCallThread1 extends Thread {
    private final Call m_CallData;
    private final ServiceReturnInfo m_ServiceReturnInfo;
    private final ServiceConnectInfo m_ConnectInfo;

    ProcessCallThread1(Call p_CallData, ServiceReturnInfo p_ServiceReturnInfo, ServiceConnectInfo p_ConnectInfo) {
        m_CallData = p_CallData;
        m_ServiceReturnInfo = p_ServiceReturnInfo;
        m_ConnectInfo = p_ConnectInfo;
    }

    public void run() {
        try {
            byte[] iiaData = Arrays.copyOfRange(m_CallData.getCallInfo(), 0, m_CallData.getMetadata().m_Size);

            Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            String result = mf_GetXmlTransformed(iiaData, xsltData);

            byte[] resByteArr = result.getBytes(StandardCharsets.UTF_8);

            // ServiceReturnInfo serviceReturnInfo = m_Connections.get(m_CallData.getMetadata().m_ConnId);

            // long returnDataSegmentSize = (new SMBReturn()).size();
            // Memory nativeMem = new Memory(returnDataSegmentSize);

            // System.out.println("SMBReturn size: " + returnDataSegmentSize);

            Call returnData = switch (m_ServiceReturnInfo.m_Q.m_Type) {
                case Constants.SMBQ -> new SMBReturn();
                case Constants.EMBQ -> new EMBReturn();
                case Constants.QMBQ -> new QMBReturn();
                case Constants.HMBQ -> new HMBReturn();
                case Constants.MBQ -> new MBReturn();
                case Constants.DMBQ -> new DMBReturn();
                case Constants.HGBQ -> new HGBReturn();
                case Constants.GBQ -> new GBReturn();
                default -> {
                    System.err.println("Return queue type not recognized!");
                    yield null;
                }
            };

            if (returnData == null) {
                System.err.println("Return data type not recognized!");
                return;
            }

            System.arraycopy(resByteArr, 0, returnData.getCallInfo(), 0, resByteArr.length);
            returnData.getMetadata().m_Size = resByteArr.length;
            returnData.getMetadata().m_ConnId = m_CallData.getMetadata().m_ConnId;

            returnData.write();
            // nativeMem.write(0, returnData.getPointer().getByteArray(0, (int)
            // returnDataSegmentSize), 0,
            // (int) returnDataSegmentSize);
            LibDSP.INSTANCE.sendReturn(m_ServiceReturnInfo, returnData.getPointer());

            // m_ConnectInfo.m_ReceiveDisconnectRequest.receiveDisconnectRequest(m_ConnectInfo);
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
        Transformer transformer = transformerFactory
                .newTransformer(new StreamSource(new ByteArrayInputStream(xsltBytes)));
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        transformer.transform(new DOMSource(document), new StreamResult(output));

        return output.toString();
    }
}


public class Main {
    public static void main(String[] args) {
        ServiceConnectInfo connectInfo = new ServiceConnectInfo();
        ServiceCallInfo callInfo = new ServiceCallInfo();
        // ConcurrentHashMap<Integer, ServiceReturnInfo> connections = new ConcurrentHashMap<Integer, ServiceReturnInfo>();

        LibDSP.INSTANCE.dspInstall(connectInfo, callInfo, "xslt-transformation", "v0.0.2", Constants.SMBQ);

        connectInfo.read();
        callInfo.read();

        // ConnectThread connectThread = new ConnectThread(connectInfo, connections);
        DisconnectThread disconnectThread = new DisconnectThread(connectInfo);

        // connectThread.setName("ConnectThread");
        disconnectThread.setName("DisconnectThread");

        // connectThread.start();
        disconnectThread.start();

        while (!Thread.currentThread().isInterrupted()) {
            try {
                ServiceReturnInfo returnInfo = new ServiceReturnInfo();

                connectInfo.m_ReceiveConnectRequest.receiveConnectRequest(returnInfo, connectInfo);

                returnInfo.read();

                Pointer nativePtr = returnInfo.m_ResponseQueue.m_Data;
                if (nativePtr == null) {
                    System.err.println("ConnectThread: m_Data pointer is NULL");
                    continue;
                }

                int size = new ConnectResponseInformation().size();

                byte[] buf = nativePtr.getByteArray(0, size);

                ConnectResponseInformation responseInfo = new ConnectResponseInformation();
                responseInfo.getPointer().write(0, buf, 0, size);
                responseInfo.read();
                // ConnectResponseInformation responseInfo = new ConnectResponseInformation(
                // returnInfo.m_ResponseQueue.m_Data.share(0));

                // int connId = responseInfo.m_Id;
                // connections.put(connId, returnInfo);

                SMBCall callData = new SMBCall();

                LibDSP.INSTANCE.receiveCall(callData.getPointer(), callInfo);

                callData.read();

                // ProcessCallThread processCallThread = new ProcessCallThread(callData, connections, connectInfo);
                ProcessCallThread1 processCallThread = new ProcessCallThread1(callData, returnInfo, connectInfo);
                processCallThread.setName("CallThread-" + callData.m_Metadata.m_ConnId);
                processCallThread.start();
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
    public static String s_GetXmlTransformed(byte[] xmlBytes, byte[] xsltBytes) throws Exception {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(true);

        DocumentBuilder db = dbf.newDocumentBuilder();
        Document document = db.parse(new ByteArrayInputStream(xmlBytes));

        System.setProperty("javax.xml.transform.TransformerFactory", "net.sf.saxon.TransformerFactoryImpl");
        TransformerFactory transformerFactory = TransformerFactory.newInstance();
        Transformer transformer = transformerFactory
                .newTransformer(new StreamSource(new ByteArrayInputStream(xsltBytes)));
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        transformer.transform(new DOMSource(document), new StreamResult(output));

        return output.toString();
    }
}
