package uds;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.StandardProtocolFamily;
import java.net.UnixDomainSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
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

class Constants {
    public static final int SMB = 1 << 16;
    public static final int EMB = 1 << 17;
    public static final int QMB = 1 << 18;
    public static final int HMB = 1 << 19;
    public static final int MB = 1 << 20;
    public static final int DMB = 1 << 21;
    public static final int HGB = 1 << 29;
    public static final int GB = 1 << 30;

    public static final int RETURNQ_NAME_MAX_SIZE = 256;

    public static final int SMBQ = 0;
    public static final int EMBQ = 1;
    public static final int QMBQ = 2;
    public static final int HMBQ = 3;
    public static final int MBQ = 4;
    public static final int DMBQ = 5;
    public static final int HGBQ = 6;
    public static final int GBQ = 7;
}

class ProcessCallThread implements Runnable {
    private SocketChannel m_Client;
    private ByteBuffer m_Buf;

    ProcessCallThread(SocketChannel p_Client, ByteBuffer p_Buf) {
        m_Client = p_Client;
        m_Buf = p_Buf;
    }

    public void run() {
        try {
            m_Buf.flip();

            int xmlLength = m_Buf.getInt();
            byte[] xmlBytes = new byte[xmlLength];
            m_Buf.get(xmlBytes);

            // System.out.println("Received: " + new String(m_Buf.array(), 0,
            // m_Buf.limit()));

            Path xsltPath = Paths.get("../../transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            ByteBuffer response = ByteBuffer.allocate(Constants.EMB);
            String result = mf_GetXmlTransformed(xmlBytes, xsltData);

            response.putInt(result.length());
            response.put(result.getBytes(StandardCharsets.UTF_8));

            while (response.hasRemaining()) {
                response.put((byte) 0);
            }

            response.flip();
            while (response.hasRemaining()) {
                m_Client.write(response);
            }

            m_Client.close();
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
    private SocketChannel m_Client;

    ProcessCallThread1(SocketChannel p_Client) {
        m_Client = p_Client;
    }

    public void run() {
        try {
            ByteBuffer buf = ByteBuffer.allocate(Constants.EMB);

            while (buf.hasRemaining()) {
                m_Client.read(buf);
            }

            buf.flip();

            int xmlLength = buf.getInt();
            byte[] xmlBytes = new byte[xmlLength];
            buf.get(xmlBytes);

            // System.out.println("Received: " + new String(m_Buf.array(), 0,
            // m_Buf.limit()));

            Path xsltPath = Paths.get("../../transformations/transform_version_v7.xsl");
            byte[] xsltData = Files.readAllBytes(xsltPath);

            ByteBuffer response = ByteBuffer.allocate(Constants.EMB);
            String result = mf_GetXmlTransformed(xmlBytes, xsltData);

            response.putInt(result.length());
            response.put(result.getBytes(StandardCharsets.UTF_8));

            while (response.hasRemaining()) {
                response.put((byte) 0);
            }

            response.flip();
            while (response.hasRemaining()) {
                m_Client.write(response);
            }

            m_Client.close();
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
        ExecutorService executor = Executors.newFixedThreadPool(THREAD_POOL_SIZE);
        Path socketPath = Path.of("/tmp/xslt-uds.sock");

        try {
            Files.deleteIfExists(socketPath);
        } catch (IOException e) {
            e.printStackTrace();
        }

        UnixDomainSocketAddress socketAddr = UnixDomainSocketAddress.of(socketPath);

        try {
            ServerSocketChannel server = ServerSocketChannel.open(StandardProtocolFamily.UNIX);
            server.bind(socketAddr, 1024);
            System.out.println("Server listening on " + socketPath);

            while (!Thread.currentThread().isInterrupted()) {
                SocketChannel client = server.accept();

                ByteBuffer buf = ByteBuffer.allocate(Constants.EMB);

                while (buf.hasRemaining()) {
                    client.read(buf);
                }

                executor.submit(new ProcessCallThread(client, buf));
                // executor.submit(new ProcessCallThread1(client));
                // ProcessCallThread processCallThread = new ProcessCallThread(client, buf);
                // processCallThread.setName("CallThread-" + client.hashCode());
                // processCallThread.start();
            }

            server.close();
        } catch (

        Exception e) {
            e.printStackTrace();
        }
    }
}
