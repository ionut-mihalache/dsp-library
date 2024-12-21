import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
// import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
// import java.nio.charset.StandardCharsets;

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

interface LibDSP extends Library {
    LibDSP INSTANCE = (LibDSP) Native.load("dsp", LibDSP.class);

    Pointer getCallInfo();

    void dspInstall(ServiceConnectInfo p_ConnectInfo, ServiceCallInfo p_CallInfo, String p_StrId, String p_Version);

    void dspReturn();
}

public class Main {
    public static void main(String[] args) {
        // Main main = new Main();
        ServiceConnectInfo connectInfo = new ServiceConnectInfo();
        ServiceCallInfo callInfo = new ServiceCallInfo();
        ServiceReturnInfo returnInfo = new ServiceReturnInfo();

        LibDSP.INSTANCE.dspInstall(connectInfo, callInfo, "xslt-transformation", "v0.0.1");

        while (true) {
            connectInfo.m_ReceiveConnectRequest.receiveConnectRequest(returnInfo, connectInfo);
            System.out.println("Received new connection");

            connectInfo.m_ReceiveDisconnectRequest.receiveDisconnectRequest(connectInfo);
        }

        // returnInfo.m_SendReturnFnQMB.sendQMBReturn(returnInfo.m_QMBQueue, new
        // QMBCall());

        // while (true) {
        // try {
        // QMBCall callData = new QMBCall();
        // callInfo.m_ReceiveCallFnQMB.receiveQMBCall(callData, callInfo.m_QMBQueue);

        // // ByteBuffer callInfoBuffer = ByteBuffer.wrap(callData.m_CallInfo);
        // // String iiaData = StandardCharsets.UTF_8.decode(callInfoBuffer.slice(0,
        // // callData.m_Size)).toString()
        // // .replace("\0", "");

        // byte[] iiaData = Arrays.copyOfRange(callData.m_CallInfo, 0, callData.m_Size);

        // Path xsltPath = Paths.get("transformations/transform_version_v6.xsl");
        // byte[] xsltData = Files.readAllBytes(xsltPath);

        // String result = main.getXmlTransformed(iiaData, xsltData);

        // System.out.println(result);
        // } catch (IOException e) {
        // e.printStackTrace();
        // } catch (Exception e) {
        // e.printStackTrace();
        // }
        // }
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
