package zeromq;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import org.w3c.dom.Document;

import org.zeromq.SocketType;
import org.zeromq.ZContext;
import org.zeromq.ZMQ;

class Constants {
    public static final int SMB = 1 << 16;
    public static final int EMB = 1 << 17;
    public static final int QMB = 1 << 18;
    public static final int HMB = 1 << 19;
    public static final int MB = 1 << 20;
    public static final int DMB = 1 << 21;
    public static final int HGB = 1 << 29;
    public static final int GB = 1 << 30;
}

class ProcessCallThread extends Thread {
    // private final byte[] m_Data;
    private final ZContext m_Context;
    private final int m_ThreadIdx;

    ProcessCallThread(ZContext p_Context, int p_ThreadIdx) {
        // m_Data = p_Data;
        m_Context = p_Context;
        m_ThreadIdx = p_ThreadIdx;
        setName("CallThread" + m_ThreadIdx);
    }

    public void run() {
        try (ZMQ.Socket socket = m_Context.createSocket(SocketType.REP)) {
            // socket.connect("tcp://localhost:5555"); // Connect socket within the thread
            socket.connect("inproc://workers");

            while (!Thread.currentThread().isInterrupted()) {
                byte[] reply = socket.recv(0);

                if (reply.length != Constants.HMB) {
                    throw new RuntimeException("Unexpected size: " + reply.length);
                }

                // Read the first four bytes as a 32-bit big-endian integer
                ByteBuffer buf = ByteBuffer.wrap(reply).order(ByteOrder.BIG_ENDIAN);
                int xmlLength = buf.getInt();

                // Extract the XML portion
                byte[] xmlBytes = new byte[xmlLength];
                buf.get(xmlBytes);

                Path xsltPath = Paths.get("../../transformations/transform_version_v7.xsl");
                byte[] xsltData = Files.readAllBytes(xsltPath);

                ByteBuffer response = ByteBuffer.allocate(Constants.HMB);
                String result = mf_GetXmlTransformed(xmlBytes, xsltData);

                response.putInt(result.length());
                response.put(result.getBytes(StandardCharsets.UTF_8));

                socket.send(response.array(), 0);
            }
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

    public static void main(String args[]) {
        // String ipcPath = "ipc:///tmp/xslt-zmq.sock";
        String ipcPath = "tcp://localhost:5557";

        try (ZContext context = new ZContext()) {
            ZMQ.Socket frontendSocket = context.createSocket(SocketType.ROUTER);
            frontendSocket.bind(ipcPath);

            ZMQ.Socket backendSocket = context.createSocket(SocketType.DEALER);
            backendSocket.bind("inproc://workers");

            for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
                new ProcessCallThread(context, i).start();
            }

            ZMQ.proxy(frontendSocket, backendSocket, null);
            // Socket to talk to clients
            // ZMQ.Socket socket = context.createSocket(SocketType.REP);
            // socket.bind(tcpPath);

            // while (!Thread.currentThread().isInterrupted()) {
            // // Block until a message is received
            // byte[] reply = socket.recv(0);

            // if (reply.length != Constants.HMB) {
            // throw new RuntimeException("Unexpected size: " + reply.length);
            // }

            // // Read the first four bytes as a 32-bit big-endian integer
            // ByteBuffer buf = ByteBuffer.wrap(reply).order(ByteOrder.BIG_ENDIAN);
            // int xmlLength = buf.getInt();

            // // Extract the XML portion
            // byte[] xmlBytes = new byte[xmlLength];
            // buf.get(xmlBytes);

            // // String xml = new String(xmlBytes, StandardCharsets.UTF_8);

            // // Print the message
            // // System.out.println(
            // // "Received: [" + new String(reply, ZMQ.CHARSET) + "]");

            // // Send a response
            // // ProcessCallThread processCallThread = new ProcessCallThread(reply,
            // // context);
            // // processCallThread.setName("CallThread");
            // // processCallThread.start();
            // // String response = "Hello, world!";
            // // socket.send(response.getBytes(ZMQ.CHARSET), 0);

            // Path xsltPath = Paths.get("../../transformations/transform_version_v7.xsl");
            // byte[] xsltData = Files.readAllBytes(xsltPath);

            // ByteBuffer response = ByteBuffer.allocate(Constants.HMB);
            // String result = Main.mf_GetXmlTransformed(xmlBytes, xsltData);

            // response.putInt(result.length());
            // response.put(result.getBytes(StandardCharsets.UTF_8));

            // socket.send(response.array(), 0);
            // }

            // socket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static String mf_GetXmlTransformed(byte[] xmlBytes, byte[] xsltBytes) throws Exception {
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

// java -cp ../../lib/jeromq-0.6.0.jar Main.java
