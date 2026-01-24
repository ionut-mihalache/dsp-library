package uds;

import java.net.StandardProtocolFamily;
import java.net.UnixDomainSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.file.Path;

public class Main {
    public static void main(String args[]) throws Exception {
        Path socketPath = Path.of("/tmp/xslt.sock");
        UnixDomainSocketAddress socketAddr = UnixDomainSocketAddress.of(socketPath);

        try (
                ServerSocketChannel server = ServerSocketChannel.open(StandardProtocolFamily.UNIX)) {
            server.bind(socketAddr);

            System.out.println("Server listening on " + socketPath);

            try (SocketChannel client = server.accept()) {
                ByteBuffer buf = ByteBuffer.allocate(1024);
                client.read(buf);
                buf.flip();

                System.out.println("Received: " + new String(buf.array(), 0, buf.limit()));
            }
        }
    }
}
