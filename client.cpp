#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")
/*
   Roadmap
   1. initialize winsock
   2. create socket
   3. connect to the server
   4. send/receive messages
   5. close the socket
*/

using namespace std;

bool initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// XOR-based encryption and decryption function
void encryptDecrypt(char* data, int length, const char* key, int keyLength) {
    for (int i = 0; i < length; i++) {
        data[i] ^= key[i % keyLength];
    }
}

void SendMsg(SOCKET s) {
    cout << "Enter your chat name: " << endl;
    string name;
    getline(cin, name);
    string message;

    const char* encryptionKey = "my_secret_key"; // Encryption key
    int keyLength = strlen(encryptionKey);

    while (true) {
        getline(cin, message);
        if (message == "quit") {
            cout << "Stopping the application!" << endl;
            break;
        }
        string msg = name + " : " + message;

        // Convert the message to a C-style string for encryption
        char* buffer = new char[msg.length() + 1];
        strcpy_s(buffer, msg.length() + 1, msg.c_str()); // Safe version of strcpy

        // Encrypt the message
        encryptDecrypt(buffer, msg.length(), encryptionKey, keyLength);

        // Send the encrypted message
        int bytesent = send(s, buffer, msg.length(), 0);
        delete[] buffer; // Clean up allocated memory

        if (bytesent == SOCKET_ERROR) {
            cout << "Error in sending message!" << endl;
            break;
        }
    }
}

void ReceiveMsg(SOCKET s) {
    char buffer[4096];
    int recvlength;

    const char* encryptionKey = "my_secret_key"; // Encryption key
    int keyLength = strlen(encryptionKey);

    while (true) {
        recvlength = recv(s, buffer, sizeof(buffer) - 1, 0); // Leave room for null terminator
        if (recvlength <= 0) {
            cout << "Disconnected from the server!" << endl;
            break;
        }
        else {
            buffer[recvlength] = '\0'; // Null-terminate the buffer

            // Decrypt the received message
            encryptDecrypt(buffer, recvlength, encryptionKey, keyLength);

            // Display the decrypted message
            cout << string(buffer) << endl;
        }
    }
}

int main() {
    if (!initialize()) {
        cout << "Winsock initialization failed!" << endl;
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        cout << "Invalid socket created!" << endl;
        return 1;
    }

    int port = 12345;
    string serveraddress = "127.0.0.1";
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));

    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Not able to connect to server!" << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }
    cout << "Successfully connected to the server!" << endl;

    thread senderthread(SendMsg, s);
    thread receiverthread(ReceiveMsg, s);

    senderthread.join();
    receiverthread.join();

    closesocket(s);
    WSACleanup();

    return 0;
}
