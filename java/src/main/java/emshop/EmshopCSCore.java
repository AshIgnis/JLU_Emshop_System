package emshop;
import java.io.*;
import java.net.*;

public class EmshopCSCore {
    // ================== 服务器端 ==================
    public static class Server {
        private int port;
        private ServerSocket serverSocket;
        private boolean running = false;

        public Server(int port) { this.port = port; }

        public void start() throws IOException {
            serverSocket = new ServerSocket(port);
            running = true;
            System.out.println("Server started on port " + port);
            while (running) {
                Socket clientSocket = serverSocket.accept();
                new Thread(new ClientHandler(clientSocket)).start();
            }
        }

        public void stop() throws IOException {
            running = false;
            if (serverSocket != null) serverSocket.close();
        }

        // 处理客户端连接
        private static class ClientHandler implements Runnable {
            private Socket socket;
            public ClientHandler(Socket socket) { this.socket = socket; }
            @Override
            public void run() {
                try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                     PrintWriter out = new PrintWriter(socket.getOutputStream(), true)) {
                    String request;
                    while ((request = in.readLine()) != null) {
                        // 协议解析与业务分发
                        String response = EmshopServer.dispatch(request);
                        out.println(response);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    try { socket.close(); } catch (IOException ignored) {}
                }
            }
        }
    }

    // ================== 客户端 ==================
    public static class Client {
        private String host;
        private int port;
        private Socket socket;

        public Client(String host, int port) {
            this.host = host;
            this.port = port;
        }

        public void start() throws IOException {
            socket = new Socket(host, port);
            try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                 PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                 BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in))) {
                String userInput;
                System.out.println("Connected to server. Type your request:");
                while ((userInput = stdIn.readLine()) != null) {
                    out.println(userInput);
                    String response = in.readLine();
                    System.out.println("Server: " + response);
                }
            } finally {
                socket.close();
            }
        }
    }
}
