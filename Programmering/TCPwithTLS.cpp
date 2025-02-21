#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>

using namespace std;

// Oppsett av SSL-kontekst (SSL_CTX) for å håndtere SSL og TLS logikken
// Laster sertifikat (server.crt) og privat nøkkel (server.key)
// hvis en av de feiler, avsluttes programmet
SSL_CTX *initSSL() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        cerr << "Error creating SSL context" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Henter sertifikat og private keye..." << endl;

    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
    {
        cerr << "Error loading certificate or private key" << endl;
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Metode for å lese HTML filene
string readFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/html\r\n\r\n"
               "<h1>404 File Not Found</h1>";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Behandler forespørsler fra klieneten med TLS
void handleRequest(SSL *ssl) {
    char buffer[1024];
    int bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        cout << "Feil ved lesing av forespørsel." << endl;
        return;
    }
    buffer[bytesRead] = '\0';
    string request(buffer);
    string response;

    if (request.find("GET / HTTP/1.1") != string::npos) {
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n\r\n" +
                   readFile("./sider/index.html");
    }
    else if (request.find("GET /page1 HTTP/1.1") != string::npos) {
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n\r\n" +
                   readFile("./sider/page1.html");
    }
    else if (request.find("GET /page2 HTTP/1.1") != string::npos) {
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n\r\n" +
                   readFile("./sider/page2.html");
    }
    else {
        response = "HTTP/1.1 404 Not Found\r\n"
                   "Content-Type: text/html\r\n\r\n"
                   "<h1>404 Page Not Found</h1>";
    }
    SSL_write(ssl, response.c_str(), response.length());
}

// Oppretter TCP-server med TLS
// initialiserer først OpenSSL og nødvendige biblioteker for å bruke TLS
int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = initSSL();
    
    // setter opp en socket som venter på at klienter kobler seg til
    // binder den til port 4433 med bind() og lytter for nye tilkoblinger med listen()
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Feil ved opprettelse av socket." << endl;
        return 1;
    }

    int port = 4433;
    const char* portEnv = getenv("PORT");
    if (portEnv) {
        port = atoi(portEnv);
    }
    cout << "Starter server på port " << port << "..." << endl;
    cout << "https://localhost:4433" << endl;

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Feil ved binding av socket." << endl;
        return 1;
    }
    if (listen(serverSocket, 10) == -1) {
        cerr << "Kunne ikke starte lytting." << endl;
        return 1;
    }

    // venter på nye tilkoblinger, når de gjør det opprettes det en ny SSL tilkobling
    // deretter gjøres en TLS handshake
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        char clientIP[INET_ADDRSTRLEN];
        
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        cout << "Ny tilkobling fra: " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
        if (clientSocket == -1) {
            cerr << "Feil ved klientforbindelse." << endl;
            continue;
        }

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, clientSocket);
        

        if (SSL_accept(ssl) <= 0){
            unsigned long err = ERR_get_error();
            if (ERR_peek_error()) {
                const char *reason = ERR_reason_error_string(err);
                if (reason && strstr(reason, "unexpected eof")) {
                    // Normal case - client closed connection during handshake
                    std::cout << "Info: Client closed connection during handshake (normal behavior)" << std::endl;
                } else {
                    // Other SSL errors that might be more serious
                    std::cerr << "SSL Handshake failed: " << ERR_error_string(err, nullptr) << std::endl;
                    ERR_print_errors_fp(stderr);
                }
            }
        }

        handleRequest(ssl);     // for å svare klienten
        SSL_shutdown(ssl);      // avslutter SSL-forbindelsen
        SSL_free(ssl);          
        close(clientSocket);    
    }

    SSL_CTX_free(ctx);
    return 0;
}

/**
 * Bygger filen:
 * Press Cmd + Shift + B to build using the tasks.json configuration
 *
 * Kompilere:
 * g++ -std=c++11 -o TCPwithTLS TCPwithTLS.cpp -lssl -lcrypto
 *
 * Kan hende må bruke dette:
clang++ -std=c++17 -g TCPwithTLS.cpp -o TCPwithTLS \
-I/opt/homebrew/opt/openssl@3/include \
-L/opt/homebrew/opt/openssl@3/lib \
-lssl -lcrypto
 *
 * ./TCPwithTLS
 *
 * https://localhost:4433
 */
