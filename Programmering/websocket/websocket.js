const net = require("net");
const crypto = require("crypto");
const fs = require("fs");
const path = require("path");

const WEBSOCKET_MAGIC_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const clients = new Set();

class WebSocketServer {
  constructor() {
    this.clientIdCounter = 0;
  }

  decodeMessage(buffer) {
    const secondByte = buffer[1];
    const length = secondByte & 127;
    const mask = buffer.slice(2, 6);
    const payload = buffer.slice(6, 6 + length);

    let decoded = "";
    for (let i = 0; i < payload.length; i++) {
      decoded += String.fromCharCode(payload[i] ^ mask[i % 4]);
    }
    return decoded;
  }

  encodeMessage(message) {
    const length = Buffer.byteLength(message);
    const buffer = Buffer.alloc(2 + length);
    buffer[0] = 0x81; // FIN bit + text message
    buffer[1] = length;
    buffer.write(message, 2);
    return buffer;
  }

  handleHandshake(data, socket) {
    const request = data.toString();
    if (!request.includes("Upgrade: websocket")) {
      socket.end();
      return;
    }

    const keyMatch = request.match(/Sec-WebSocket-Key: (.+)/);
    if (!keyMatch) {
      socket.end();
      return;
    }

    const acceptKey = crypto
      .createHash("sha1")
      .update(keyMatch[1].trim() + WEBSOCKET_MAGIC_STRING)
      .digest("base64");

    const responseHeaders = [
      "HTTP/1.1 101 Switching Protocols",
      "Upgrade: websocket",
      "Connection: Upgrade",
      `Sec-WebSocket-Accept: ${acceptKey}`,
      "\r\n",
    ].join("\r\n");

    socket.write(responseHeaders);
    clients.add(socket);
    console.log(`Handshake successful with Client ${socket.clientId}`);
  }

  handleMessage(socket, data) {
    if (data.length < 2) return;

    const message = this.decodeMessage(data);
    console.log(`Client ${socket.clientId} sent: ${message}`);

    // Only broadcast button click messages
    if (message === "button_click") {
      const encodedMessage = this.encodeMessage(
        `Client ${socket.clientId} says hello!`
      );
      this.broadcast(socket, encodedMessage);
    }
  }

  broadcast(sender, message) {
    clients.forEach((client) => {
      if (client !== sender) {
        client.write(message);
      }
    });
  }

  start(httpPort = 3000, wsPort = 3001) {
    // HTTP Server
    const httpServer = net.createServer((connection) => {
      connection.on("data", () => {
        const content = fs.readFileSync(
          path.join(__dirname, "index.html"),
          "utf8"
        );

        connection.write(
          "HTTP/1.1 200 OK\r\n" +
            "Content-Type: text/html\r\n" +
            "Content-Length: " +
            content.length +
            "\r\n" +
            "\r\n" +
            content
        );
      });
    });

    httpServer.listen(httpPort, () =>
      console.log(`HTTP server listening on port ${httpPort}`)
    );

    const wsServer = net.createServer((socket) => {
      socket.clientId = ++this.clientIdCounter;
      console.log(`\nNew client connected: Client ${socket.clientId}`);

      socket.once("data", (data) => this.handleHandshake(data, socket));
      socket.on("data", (data) => this.handleMessage(socket, data));

      socket.on("end", () => {
        console.log(`Client ${socket.clientId} disconnected`);
        clients.delete(socket);
      });

      socket.on("error", (err) => {
        console.error(`Error from Client ${socket.clientId}:`, err);
        clients.delete(socket);
      });
    });

    wsServer.listen(wsPort, () => {
      console.log(`WebSocket server listening on port ${wsPort}`);
    });
  }
}

// Start the server
new WebSocketServer().start();
