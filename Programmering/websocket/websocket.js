const net = require("net");
const crypto = require("crypto"); // Brukes for å generere en sikker nøkkel for Websocket handshake
const fs = require("fs");
const path = require("path");

const clients = new Set();
const WEBSOCKET_MAGIC_STRING = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // rfc6455 standard

class WebSocketServer {
  constructor() {
    this.clientIdCounter = 0;
  }

  decodeMessage(buffer) {
    const secondByte = buffer[1]; 
    const length = secondByte & 127;              // lengde på melding
    const mask = buffer.slice(2, 6);              // maskere melding, 4bytes
    const payload = buffer.slice(6, 6 + length);

    // XOR-operasjon for å dekryptere meldingen byte for byte
    let decoded = "";
    for (let i = 0; i < payload.length; i++) {
      decoded += String.fromCharCode(payload[i] ^ mask[i % 4]);
    }
    return decoded;
  }

  encodeMessage(message) {
    const length = Buffer.byteLength(message);
    const buffer = Buffer.alloc(2 + length);
    buffer[0] = 0x81;         // 0x81 signaliserer at dette er en tekstmelding
    buffer[1] = length;
    buffer.write(message, 2); // Skriver innholdet til bufferen
    return buffer;
  }

  // Når klient kobler til Websocket serveren, sender den en HTTP forespørsel,
  // serveren svarer så med en 101 switiching protocols for å oppgradere til Websocket-forbindelse
  handleHandshake(data, socket) {
    const request = data.toString();
    if (!request.includes("Upgrade: websocket")) {
      socket.end();
      return;
    }

    // henter websocket nøkkelen
    const keyMatch = request.match(/Sec-WebSocket-Key: (.+)/);
    if (!keyMatch) {
      socket.end();
      return;
    }

    // hasher den mottatte nøkkelen med RFC6455-standarden stringen
    const acceptKey = crypto
      .createHash("sha1")
      .update(keyMatch[1].trim() + WEBSOCKET_MAGIC_STRING)
      .digest("base64");

    // lager en responsheader til klient
    // og legger til klienten i clients-settet
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

  // håndterer meldingen fra klient, 
  // per nå så er det bare hello-button som godkjennes som melding mellom klinenter
  handleMessage(socket, data) {
    if (data.length < 2) return;

    const message = this.decodeMessage(data);
    console.log(`Client ${socket.clientId} sent: ${message}`);

    // Broadcaster bare hello-button
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

  // starter serveren på port 3000 og websocketen på 3001
  start(httpPort = 3000, wsPort = 3001) {
    // HTTP Server med HTML-siden
    const httpServer = net.createServer((connection) => { // TCP
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

    // WebSocket serveren, inkrementerer clientIdCounter og håndterer handshake 
    const wsServer = net.createServer((socket) => { // TCP
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
