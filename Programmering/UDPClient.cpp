#include <iostream>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>
#include <vector>
#include <unistd.h>

#define PORT 8080
#define MAXLINE 1024

using namespace std;

int main() {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[MAXLINE];
 
    // Lager en socket for klientsiden, denne skal koble til serveren
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        cerr << "Error: Could not create socket!" << endl;
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));     // Nullstiller serverens adresse
    serverAddr.sin_family = AF_INET;                // Angir at vi bruker IPv4
    serverAddr.sin_port = htons(PORT);              // Setter portnummeret, definert som 8080
    serverAddr.sin_addr.s_addr = INADDR_ANY;        // Sender til hvilken som helst adresse

    while (true) { 
        cout << "Enter first vector (numbers separated by space, or 'q' to quit): ";
        string inputA;
        getline(cin, inputA);
    
        if (inputA == "q") break;

        cout << "Enter second vector (same length): ";
        string inputB;
        getline(cin, inputB);

        string message = inputA + "\n" + inputB; // slår sammen input som en melding til serveren

        // Sender pakken via UDP til serveren. Dette blir sendt uten å opprette en forbindelse. 
        // Pakken blir sendt rett til serveren uten handshake eller feilkontroll.
        if (sendto(sockfd, message.c_str(), message.length(), 0,                
                  (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            cerr << "Error: Failed to send message" << endl;                        
            continue;
        }

        // Svar fra serveren
        // Venter på melding fra serveren med recvfrom()
        socklen_t serverLen = sizeof(serverAddr);
        int bytesReceived = recvfrom(sockfd, buffer, MAXLINE, 0,                
                                   (struct sockaddr*)&serverAddr, &serverLen);
        
        if (bytesReceived < 0) {
            cerr << "Error: Failed to receive response" << endl;
            continue;
        }

        buffer[bytesReceived] = '\0';
        cout << "Dot product result: " << buffer << endl << endl;
    }

    // Clean up
    close(sockfd);
    return 0;
}