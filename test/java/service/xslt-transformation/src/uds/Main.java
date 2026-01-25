package uds;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.net.StandardProtocolFamily;
import java.net.UnixDomainSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
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

public class Main {
    public static void main(String args[]) {
        Path socketPath = Path.of("/tmp/xslt.sock");
        UnixDomainSocketAddress socketAddr = UnixDomainSocketAddress.of(socketPath);

        try (
                ServerSocketChannel server = ServerSocketChannel.open(StandardProtocolFamily.UNIX)) {
            server.bind(socketAddr);

            while (!Thread.currentThread().interrupted()) {
                System.out.println("Server listening on " + socketPath);

                try (SocketChannel client = server.accept()) {
                    ByteBuffer buf = ByteBuffer.allocate(1024);
                    client.read(buf);
                    buf.flip();

                    System.out.println("Received: " + new String(buf.array(), 0, buf.limit()));

                    Path xsltPath = Paths.get("../../transformations/transform_version_v7.xsl");
                    byte[] xsltData = Files.readAllBytes(xsltPath);

                    String result = Main.mf_GetXmlTransformed("TODO".getBytes(), xsltData);

                    client.write(ByteBuffer.wrap(result.getBytes()));
                }
            }
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
