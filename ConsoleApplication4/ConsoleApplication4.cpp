#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <sstream>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6667"
#define DEFAULT_IP "irc.freenode.net"

using namespace std;

class Client {
    public:
        SOCKET ConnectSocket;
        int msg(string message) {
            message += "\r\n";
            const char* bytes = message.c_str();
            int r = send(this->ConnectSocket, bytes, (int)strlen(bytes), 0);
            if (r == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(this->ConnectSocket);
                WSACleanup();
                return 1;
            }
            return r;
        }
};

void task1(Client c) {
    while (true) {
        string message;
        getline(cin, message);
        c.msg(message);
    }
}

vector<string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;
    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    return res;
}

int __cdecl main()
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    string delimiter = " ";
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(DEFAULT_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    Client c = Client();
    c.ConnectSocket = ConnectSocket;
    c.msg("PASS none");
    c.msg("NICK fulgur");
    c.msg("USER fulgur blah blah fulgur");
    thread t1(task1, c);
    t1.detach();

    do {
        memset(recvbuf, '\0', recvbuflen);
        iResult = recv(ConnectSocket, recvbuf, recvbuflen-1, NULL);
        if (iResult > 0) {
            string msg = recvbuf;
            vector<string> v = split(msg, " ");

            if (msg.substr(0, 4) == "PING") {
                if (msg.substr(0, 4) == "PING") {
                    msg = "PONG" + msg.substr(4);
                    c.msg(msg);
                }
                msg = msg.replace(0, 4, "PONG");
                c.msg(msg);
            }
            else {
                cout << recvbuf << endl;
                if (v[1] != "") {
                    if (v[1] == "NOTICE") {
                        cout << msg.substr(msg.find(" ", msg.find(" ", msg.find(" ") + 1) + 1) + 1) << endl;
                    }
                }
            }
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}