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
    // Write code to temporary file
    ofstream file("temp.py");
    if (!file) {
        cerr << "Failed to create temporary Python file" << endl;
        return "Error: Failed to create temporary file";
    }
    
    // Add error handling wrapper to the Python code
    file << "try:\n";
    // Indent the user's code
    stringstream ss(code);
    string line;
    while (getline(ss, line)) {
        file << "    " << line << "\n";
    }
    file << "except Exception as e:\n";
    file << "    print(f'Python Error: {str(e)}')\n";
    file << "    exit(1)\n";
    
    file.close();

    // Execute Python code in container
    FILE* pipe = popen("python3 temp.py 2>&1", "r");
    if (!pipe) {
        cerr << "Failed to execute Python code" << endl;
        return "Error: Failed to execute Python code";
    }

    char buffer[128];
    string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int status = pclose(pipe);
    remove("temp.py");  // Clean up temp file

    if (status != 0) {
        cerr << "Python execution failed with status: " << status << endl;
        // Return the actual error message instead of just the status
        return result;
    }

    return result;
}

void handle_post(http_request request) {
    request.extract_json().then([=](json::value json_data) {
        auto code = json_data[U("code")].as_string();
        string result = execute_python(utility::conversions::to_utf8string(code));
        
        json::value response;
        response[U("result")] = json::value::string(utility::conversions::to_string_t(result));
        
        http_response http_resp(status_codes::OK);
        http_resp.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        http_resp.set_body(response);
        
        request.reply(http_resp);
    }).wait();
}

int main() {
    http_listener listener(U("http://0.0.0.0:8080/execute"));
    
    // Update POST handler to include CORS headers
    listener.support(methods::POST, [](http_request request) {
        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.headers().add(U("Access-Control-Allow-Methods"), U("POST, OPTIONS"));
        response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
        
        handle_post(request);
    });

    // Update OPTIONS handler
    listener.support(methods::OPTIONS, [](http_request request) {
        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.headers().add(U("Access-Control-Allow-Methods"), U("POST, OPTIONS"));
        response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type"));
        request.reply(response);
    });

    try {
        listener.open().wait();
        cout << "Server running at http://localhost:8080/execute" << endl;
        while (true) {
            this_thread::sleep_for(chrono::seconds(1));
        }
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}