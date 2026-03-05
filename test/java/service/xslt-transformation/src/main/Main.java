package main;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

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

import calling.call_package.DMBCall;
import calling.call_package.EMBCall;
import calling.call_package.GBCall;
import calling.call_package.HGBCall;
import calling.call_package.SMBCall;
import calling.call_package.QMBCall;
import calling.call_package.HMBCall;
import calling.call_package.MBCall;
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
            // synchronized (Main.CONNECT_LOCK) {
            m_ConnectInfo.m_ReceiveDisconnectRequest.receiveDisconnectRequest(m_ConnectInfo);
            // }
        }
    }
}

class ProcessCallThread implements Runnable {
    private final Call m_CallData;
    private final ConcurrentHashMap<Integer, ServiceReturnInfo> m_Connections;

    ProcessCallThread(Call p_CallData, ConcurrentHashMap<Integer, ServiceReturnInfo> p_Connections) {
        m_CallData = p_CallData;
        m_Connections = p_Connections;
    }

    public void run() {
        try {
            byte[] iiaData = Arrays.copyOfRange(m_CallData.getCallInfo(), 0, m_CallData.getMetadata().m_Size);

            Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            String result = mf_GetXmlTransformed(iiaData, xsltData);

            byte[] resByteArr = result.getBytes(StandardCharsets.UTF_8);

            // System.out.println(m_Connections.keySet());

            // ServiceReturnInfo serviceReturnInfo =
            // m_Connections.get(m_CallData.getMetadata().m_ConnId);

            ServiceReturnInfo serviceReturnInfo = m_Connections.get(m_CallData.getMetadata().m_ConnId);
            while (serviceReturnInfo == null) {
                serviceReturnInfo = m_Connections.get(m_CallData.getMetadata().m_ConnId);

                if (serviceReturnInfo == null) {
                    Thread.onSpinWait(); // foarte ieftin
                }
            }

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

            // m_Connections.remove(m_CallData.getMetadata().m_ConnId);
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

class ProcessCallThread1 implements Runnable {
    private final Call m_CallData;
    private final ServiceReturnInfo m_ServiceReturnInfo;

    ProcessCallThread1(Call p_CallData, ServiceReturnInfo p_ServiceReturnInfo) {
        m_CallData = p_CallData;
        m_ServiceReturnInfo = p_ServiceReturnInfo;
    }

    public void run() {
        try {
            byte[] iiaData = Arrays.copyOfRange(m_CallData.getCallInfo(), 0, m_CallData.getMetadata().m_Size);

            Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            String result = mf_GetXmlTransformed(iiaData, xsltData);

            byte[] resByteArr = result.getBytes(StandardCharsets.UTF_8);

            // ServiceReturnInfo serviceReturnInfo =
            // m_Connections.get(m_CallData.getMetadata().m_ConnId);

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

class ProcessCallThread2 implements Runnable {
    private final ServiceCallInfo m_CallInfo;
    private final ServiceReturnInfo m_ServiceReturnInfo;

    ProcessCallThread2(ServiceCallInfo p_CallInfo, ServiceReturnInfo p_ServiceReturnInfo) {
        m_CallInfo = p_CallInfo;
        m_ServiceReturnInfo = p_ServiceReturnInfo;
    }

    public void run() {
        try {
            Call callData = switch (Main.QTYPE) {
                case Constants.SMBQ -> new SMBCall();
                case Constants.EMBQ -> new EMBCall();
                case Constants.QMBQ -> new QMBCall();
                case Constants.HMBQ -> new HMBCall();
                case Constants.MBQ -> new MBCall();
                case Constants.DMBQ -> new DMBCall();
                case Constants.HGBQ -> new HGBCall();
                case Constants.GBQ -> new GBCall();
                default -> {
                    System.err.println("Return queue type not recognized!");
                    yield null;
                }
            };

            LibDSP.INSTANCE.receiveCall(callData.getPointer(), m_CallInfo);

            callData.read();

            byte[] iiaData = Arrays.copyOfRange(callData.getCallInfo(), 0, callData.getMetadata().m_Size);

            Path xsltPath = Paths.get("transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            String result = mf_GetXmlTransformed(iiaData, xsltData);

            byte[] resByteArr = result.getBytes(StandardCharsets.UTF_8);

            // ServiceReturnInfo serviceReturnInfo =
            // m_Connections.get(m_CallData.getMetadata().m_ConnId);

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
            returnData.getMetadata().m_ConnId = callData.getMetadata().m_ConnId;

            returnData.write();
            // nativeMem.write(0, returnData.getPointer().getByteArray(0, (int)
            // returnDataSegmentSize), 0,
            // (int) returnDataSegmentSize);
            // synchronized (Main.RETURN_LOCK) {
            LibDSP.INSTANCE.sendReturn(m_ServiceReturnInfo, returnData.getPointer());
            // }

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
    private static final int THREAD_POOL_SIZE = Runtime.getRuntime().availableProcessors();
    public static int PAYLOAD_SIZE;
    public static int QTYPE;

    private static int sm_GetQType(String p_Arg) {
        switch (p_Arg.toUpperCase()) {
            case "SMB":
                return Constants.SMBQ;
            case "EMB":
                return Constants.EMBQ;
            case "QMB":
                return Constants.QMBQ;
            case "HMB":
                return Constants.HMBQ;
            case "MB":
                return Constants.MBQ;
            case "DMB":
                return Constants.DMBQ;
            case "HGB":
                return Constants.HGBQ;
            case "GB":
                return Constants.GBQ;
            default:
                throw new IllegalArgumentException("Unknown queue type: " + p_Arg);
        }
    }

    private static int sm_GetPayloadSize(String p_Arg) {
        switch (p_Arg.toUpperCase()) {
            case "SMB":
                return Constants.SMB;
            case "EMB":
                return Constants.EMB;
            case "QMB":
                return Constants.QMB;
            case "HMB":
                return Constants.HMB;
            case "MB":
                return Constants.MB;
            case "DMB":
                return Constants.DMB;
            case "HGB":
                return Constants.HGB;
            case "GB":
                return Constants.GB;
            default:
                throw new IllegalArgumentException("Unknown payload type: " + p_Arg);
        }
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            PAYLOAD_SIZE = Constants.SMB;
            QTYPE = Constants.SMBQ;
        } else {
            PAYLOAD_SIZE = sm_GetPayloadSize(args[0]);
            QTYPE = sm_GetQType(args[0]);
        }

        ExecutorService executor = Executors.newFixedThreadPool(THREAD_POOL_SIZE);

        ServiceConnectInfo connectInfo = new ServiceConnectInfo();
        ServiceCallInfo callInfo = new ServiceCallInfo();
        // ConcurrentHashMap<Integer, ServiceReturnInfo> connections = new
        // ConcurrentHashMap<Integer, ServiceReturnInfo>();

        LibDSP.INSTANCE.dspInstall(connectInfo, callInfo, "xslt-transformation", "v0.0.2", QTYPE);

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

                // Pointer nativePtr = returnInfo.m_ResponseQueue.m_Data;
                // if (nativePtr == null) {
                // System.err.println("ConnectThread: m_Data pointer is NULL");
                // continue;
                // }

                // int size = new ConnectResponseInformation().size();

                // byte[] buf = nativePtr.getByteArray(0, size);

                // ConnectResponseInformation responseInfo = new ConnectResponseInformation();
                // responseInfo.getPointer().write(0, buf, 0, size);
                // responseInfo.read();
                // ConnectResponseInformation responseInfo = new ConnectResponseInformation(
                // returnInfo.m_ResponseQueue.m_Data.share(0));

                // int connId = responseInfo.m_Id;
                // connections.put(connId, returnInfo);
                // if (oldReturnInfo != null) {
                // System.err.println("Why is this the case?!");
                // }

                // SMBCall callData = new SMBCall();
                // EMBCall callData = new EMBCall();
                // QMBCall callData = new QMBCall();
                // HMBCall callData = new HMBCall();
                // MBCall callData = new MBCall();

                Call callData = switch (QTYPE) {
                    case Constants.SMBQ -> new SMBCall();
                    case Constants.EMBQ -> new EMBCall();
                    case Constants.QMBQ -> new QMBCall();
                    case Constants.HMBQ -> new HMBCall();
                    case Constants.MBQ -> new MBCall();
                    case Constants.DMBQ -> new DMBCall();
                    case Constants.HGBQ -> new HGBCall();
                    case Constants.GBQ -> new GBCall();
                    default -> {
                        System.err.println("Return queue type not recognized!");
                        yield null;
                    }
                };

                // synchronized (Main.DSP_LOCK) {
                LibDSP.INSTANCE.receiveCall(callData.getPointer(), callInfo);
                // }

                callData.read();

                // executor.submit(new ProcessCallThread(callData, connections));
                executor.submit(new ProcessCallThread1(callData, returnInfo));
                // executor.submit(new ProcessCallThread2(callInfo, returnInfo));
                // ProcessCallThread processCallThread = new ProcessCallThread(callData,
                // connections, connectInfo);
                // ProcessCallThread1 processCallThread = new ProcessCallThread1(callData,
                // returnInfo, connectInfo);
                // processCallThread.setName("CallThread-" + callData.m_Metadata.m_ConnId);
                // processCallThread.start();
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
