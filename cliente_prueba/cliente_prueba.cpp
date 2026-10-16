#define _WINSOCK_DEPRECATED_NO_WARNINGS  // Permite usar inet_addr en Visual Studio

#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // Enlaza la libreria Winsock

#define PORT 8080

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Fallo al inicializar Winsock. Codigo de error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Crear socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Fallo en socket. Codigo de error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // IP del receptor (127.0.0.1 = este mismo PC)
    serv_addr.sin_addr.s_addr = inet_addr("192.168.80.20");
    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Direccion invalida o no soportada" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Esperar a que el receptor este listo antes de conectar
    std::cout << "Presiona Enter para conectar...";
    std::cin.get();

    // Conectar al servidor
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

        texto += "\n";  // El receptor separa los mensajes por \n
        send(sock, texto.c_str(), (int)texto.size(), 0);

        int valread = recv(sock, buffer, 1023, 0);
        if (valread <= 0) {
            std::cout << "El servidor se desconecto\n";
            break;
        }
        buffer[valread] = '\0';
        std::cout << "Respuesta del servidor: " << buffer << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}