#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10
#define DEFAULT_BUFLEN 512

SOCKET server_socket;
std::vector<std::string> orders;

int main() {
    using namespace std;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Failed. Error Code: " << WSAGetLastError() << "\n";
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cout << "Could not create socket: " << WSAGetLastError() << "\n";
        return 2;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        cout << "Bind failed with error code: " << WSAGetLastError() << "\n";
        return 3;
    }

    listen(server_socket, MAX_CLIENTS);

    fd_set readfds;
    SOCKET client_socket[MAX_CLIENTS] = {};

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET s = client_socket[i];
            if (s > 0) {
                FD_SET(s, &readfds);
            }
        }

        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
            cout << "select function call failed with error code : " << WSAGetLastError() << "\n";
            return 4;
        }

        SOCKET new_socket;
        sockaddr_in address;
        int addrlen = sizeof(sockaddr_in);
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
                cout << "accept function error\n";
                return 5;
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 0; i < orders.size(); i++)
            {
                cout << "Order #" << i + 1 << ": " << orders[i] << "\n";
                send(new_socket, orders[i].c_str(), orders[i].size(), 0);
            }

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets at index %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            SOCKET s = client_socket[i];
            if (FD_ISSET(s, &readfds))
            {
                getpeername(s, (sockaddr*)&address, (int*)&addrlen);

                char client_message[DEFAULT_BUFLEN];
                int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
                client_message[client_message_length] = '\0';

                string check_exit = client_message;
                if (check_exit == "off")
                {
                    cout << "Client #" << i << " is off\n";
                    client_socket[i] = 0;
                }

                string temp = client_message;
                orders.push_back(temp);

                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_socket[i] != 0) {
                        send(client_socket[i], client_message, client_message_length, 0);
                    }
                }
            }
        }
    }

    WSACleanup();
    return 0;
}




