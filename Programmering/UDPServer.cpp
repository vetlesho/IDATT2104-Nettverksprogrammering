#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

using namespace std;

// Funksjon for å beregne dot-produktet
double dotProduct(vector<double> &vec1, vector<double> &vec2) {
    if (vec1.size() != vec2.size()) {
        cerr << "Error: Vectors have different sizes!" << endl;
        return 0.0;
    }

    double result = 0.0; // Changed from int to double
    for (size_t i = 0; i < vec1.size(); ++i) {
        result += vec1[i] * vec2[i];
    }

    return result;
}

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[MAXLINE];
    socklen_t clientLen = sizeof(clientAddr);

    // Opprett en UDP-socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Feil: Kunne ikke opprette socket!" << endl;
        return 1;
    }

    // Konfigurer serveradresse
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket til adresse og port
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Feil: Kunne ikke binde socket!" << endl;
        return 1;
    }

    cout << "UDP-server lytter på port " << PORT << "..." << endl;

    while (true) {
        // Motta data fra klient direkte. Ingen handshake eller tilkobling. 
        // Serveren mottar pakker uten å etablere en "session"
        int bytesReceived = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL,
                                     (struct sockaddr *)&clientAddr, &clientLen);
        buffer[bytesReceived] = '\0';

        cout << "\nReceived data: '" << buffer << "'" << endl;

        istringstream iss(buffer);
        string line1, line2;
        getline(iss, line1);
        getline(iss, line2);

        cout << "First vector string: '" << line1 << "'" << endl;
        cout << "Second vector string: '" << line2 << "'" << endl;

        vector<double> vec1, vec2;
        istringstream iss1(line1), iss2(line2);
        double num;

        while (iss1 >> num) {
            vec1.push_back(num);
            cout << "Added to vec1: " << num << endl;
        }
        while (iss2 >> num) {
            vec2.push_back(num);
            cout << "Added to vec2: " << num << endl;
        }

        double result = dotProduct(vec1, vec2);
        cout << "Mottok forespørsel: Dot-produkt = " << result << endl;

        // Send resultat rett tilbake til klienten
        string response = to_string(result);
        sendto(sockfd, response.c_str(), response.length(), 0,
               (const struct sockaddr *)&clientAddr, clientLen); 
    }

    return 0;
}
