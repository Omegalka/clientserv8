#include <iostream>
#include <winsock2.h>
#include <string>
#include <thread>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT "8888"

SOCKET client_socket;



DWORD WINAPI Sender(void* param)
{
    while (true) {
        string order;
        cout << "Enter your order: ";
        getline(cin, order);

        send(client_socket, order.c_str(), order.size(), 0);
    }
}

DWORD WINAPI Receiver(void* param)
{
    while (true) {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';

        cout << "Received message from server: " << response << "\n";
    }
}

int main()
{
    using namespace std;
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed with error: " << iResult << "\n";
        return 1;
    }

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << "\n";
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr;
    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (client_socket == INVALID_SOCKET) {
            cout << "socket failed with error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 3;
        }

        iResult = connect(client_socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
        if (iResult == SOCKET_ERROR) {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) {
        cout << "Unable to connect to server!\n";
        WSACleanup();
        return 4;
    }

    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);

    WSACleanup();
    return 0;
}

