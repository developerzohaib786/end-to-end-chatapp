#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>

using namespace std;

#pragma comment(lib,"ws2_32.lib") 

// Roadmap
// Step1: initialize winsock library
// Step2: create the socket 
// Step3: get ip and port on which it runs (local host!) 
// Step4: bind the ip or port with the socket 
// Step5: we will start listening on the socket 
// Step6: accept the blocking call 
// Step7: recieve and send 
// Step8: close the socket 
// Step9: we will cleanup the winsock libraray

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients) {
    // send and receive messages from client
    cout << "Client connected!" << endl;
    char buffer[4096];
    while (1) {
        int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesrecvd <= 0) {
            cout << "Client disconnected!" << endl;
            break;
        }

        string message(buffer, bytesrecvd);
        cout << "message from client : " << message << endl;
        for (auto client : clients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.length(), 0);

            }
        }
    }

    auto it = find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }
    closesocket(clientSocket);
}

int main() {
    if (!initialize()) {
        cout << "Winsock initialization failed!" << endl;
        return 1;
    }
    cout << "Server program!" << endl;

    // implementing step2, step3, and step4: creating a socket , getting ip and port on which it runs (local host!) and binding the ip or port with the socket 
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket is not created!" << endl;
        return 1;
    }
    // creating address structure 
    int port = 12345;
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    // converting the ipaddress (0.0.0.0) put it inside the sin_family in binary form
    if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
        cout << "Setting address structure failed!" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    // bind code
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Bind code failed!" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    // step5: listening to the socket
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listening to the socket failed!" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;

    }
    cout << "Server has started listening on the port: " << port << endl;
    vector<SOCKET> clients;
    while (1) {
        //  step6: accepting the listening from the client
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "ClientSocket is invalid!";
        }
        clients.push_back(clientSocket);
        thread tl(InteractWithClient, clientSocket, std::ref(clients));
        tl.detach();
    }


    closesocket(listenSocket);

    WSACleanup();
    return 0;
}