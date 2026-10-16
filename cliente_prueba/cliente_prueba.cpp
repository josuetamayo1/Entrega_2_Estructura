#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Fallo al inicializar Winsock. Codigo de error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Fallo en socket. Codigo de error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Direccion invalida o no soportada" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Presiona Enter para conectar...";
    std::cin.get();

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Fallo en connect. Codigo de error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Conectado. Formato: placa,hora,celda  (ej. ABC123,18:30:05,12)\n";

    std::string texto;
    while (true) {
        std::cout << "Escribe un mensaje (salir para terminar): ";
        std::getline(std::cin, texto);
        if (texto == "salir") break;

        texto += "\n";
        if (send(sock, texto.c_str(), (int)texto.size(), 0) == SOCKET_ERROR) {
            std::cout << "El servidor se desconecto\n";
            break;
        }
        std::cout << "Enviado\n";
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}