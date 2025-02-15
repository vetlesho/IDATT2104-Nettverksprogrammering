#include <iostream>     
#include <fstream>     
#include <string>      
#include <sstream>    
#include <unistd.h>    
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <cstring>     

using namespace std;

// reads HTML files and returns their content
string readFile(const string& filename) {
    ifstream file(filename); // open file, and checks if it successful
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/html\r\n\r\n"
               "<h1>404 File Not Found</h1>";
    }

    stringstream buffer; // reads the file to a string
    buffer << file.rdbuf();
    return buffer.str();
}

// handles each client request
void handleRequest(int clientSocket) {
    char buffer[1024]; 
    
    int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1); // lagre data i bufferen
    
    if (bytesRead < 0) {
        cout << "Error - could not read the request." << endl;
        return;
    }

    // konverterer forespørselen fra klienten til en gyldig c-string
    buffer[bytesRead] = '\0';
    string request(buffer); 
    string response;

    if (request.find("GET / HTTP/1.1") != string::npos) {   // sjekker om forespørselen inneholder "GET / HTTP/1.1"
        response = "HTTP/1.1 200 OK\r\n"                    // hvis ja, lager en 200 OK-respons og legger inn innhold i index.html
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


    // sends response to the client, the web browser
    send(clientSocket, response.c_str(), response.length(), 0);

    close(clientSocket);
}

int main() {
    // create a socket for the server
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cout << "Error creating socket" << endl;
        return 1;
    }

    //Set up the server address and port
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;         // IPv4
    serverAddr.sin_port = htons(3000);       // Port 3000 (unused port)
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any IP

    // Bind socket to adress and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cout << "Error binding socket to address and port" << endl;
        return 1;
    }

    // start listening for connections
    if (listen(serverSocket, 10) == -1) {
        cout << "Oops! Couldn't start listening for connections" << endl;
        return 1;
    }

    // accept new request from the user
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // Wait for a new connection
        // Her kobler klienten (nettleser) seg til for å opprette forbindelse
        // Dette er essensielt med TCP, den aksepterer en kllien, og leser dataen deres. 
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket == -1) {
            cout << "Error getting client connection" << endl;
            continue;
        }

        // Handle this client's request
        handleRequest(clientSocket);
    }

    return 0;
}

/* 
How to use this program:
1. Compile it:
   g++ -std=c++11 -o webserver webserver.cpp

2. Run it:
   ./webserver

3. Open your web browser and visit:
   http://localhost:3000
*/