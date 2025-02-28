#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <memory>
#include <cstdlib>
#include <sstream>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;

string execute_python(const string& code) {
    ofstream file("temp.py");
    
    // Legger til feilhådntering av klientens kode
    // indents og sånn
    file << "try:\n";
    stringstream ss(code);
    string line;
    while (getline(ss, line)) {
        file << "    " << line << "\n";
    }
    file << "except Exception as e:\n";
    file << "    print(f'Python Error: {str(e)}')\n";
    file << "    exit(1)\n";
    
    file.close();

    // Kjører python koden 
    // lagrer resultatet i en string
    string result;
    FILE* pipe = popen("python3 temp.py 2>&1", "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
    }
    
    remove("temp.py");
    return result;
}

int main() {
    // server som lyttter på port 8080
    http_listener listener(U("http://0.0.0.0:8080/execute"));
    
    // håndterer post-forerspørsler fra klienten
    listener.support(methods::POST, [](http_request request) {
        request.extract_json().then([=](json::value json_data) {
            // henter ut Python-kode fra JSON-forespørselen
            // konverterer og kjører koden
            auto code = json_data[U("code")].as_string();
            string result = execute_python(utility::conversions::to_utf8string(code));
            
            json::value response;
            response[U("result")] = json::value::string(utility::conversions::to_string_t(result));
            
            // setter opp HTTP-respons med CORS-header
            http_response resp(status_codes::OK);
            resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            resp.set_body(response);
            request.reply(resp);
        });
    });

    // Håndterer CORS for sikker kommunikasjon mellom backend og frontend
    // Trenger CORS - Cross-Origin Resource Sharing, 
    // siden frontend og backend kjører på to forskjellige porter
    listener.support(methods::OPTIONS, [](http_request request) {
        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*")); // burde spesifisere * til f.eks. localhost
        response.headers().add(U("Access-Control-Allow-Methods"), U("POST, OPTIONS"));
        response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
        request.reply(response);
    });

    listener.open().wait();
    cout << "Server running at http://localhost:8080/execute\n";
    while (true) this_thread::sleep_for(chrono::seconds(1));
    

    return 0;
}