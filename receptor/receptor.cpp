#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // Enlaza la libreria Winsock

#define PORT 8080

int main() {
    WSADATA wsaData;
    SOCKET servidor_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int addrlen = sizeof(direccion);
    char buffer[1024] = { 0 };
    const char* mensaje = "Recibido";

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Fallo al inicializar Winsock. Codigo de error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Crear socket
    servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_fd == INVALID_SOCKET) {
        std::cerr << "Fallo en socket. Codigo de error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Configurar direccion
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PORT);

    // Asociar socket a direccion
    if (bind(servidor_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        std::cerr << "Fallo en bind. Codigo de error: " << WSAGetLastError() << std::endl;
        closesocket(servidor_fd);
        WSACleanup();
        return 1;
    }

    // Escuchar conexiones
    if (listen(servidor_fd, 3) == SOCKET_ERROR) {
        std::cerr << "Fallo en listen. Codigo de error: " << WSAGetLastError() << std::endl;
        closesocket(servidor_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Esperando conexion en el puerto " << PORT << "...\n";

    // Aceptar conexion
    nuevo_socket = accept(servidor_fd, (struct sockaddr*)&direccion, &addrlen);
    if (nuevo_socket == INVALID_SOCKET) {
        std::cerr << "Fallo en accept. Codigo de error: " << WSAGetLastError() << std::endl;
        closesocket(servidor_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Cliente conectado\n";

    // Acumulador: guarda lo recibido hasta tener mensajes completos (terminados en \n)
    std::string pendiente;

    while (true) {
        int valread = recv(nuevo_socket, buffer, 1023, 0);
        if (valread <= 0) {
            std::cout << "El cliente se desconecto\n";
            break;
        }
        buffer[valread] = '\0';
        pendiente += buffer;

        // Sacar todos los mensajes completos que haya en el acumulador
        size_t pos;
        while ((pos = pendiente.find('\n')) != std::string::npos) {
            std::string linea = pendiente.substr(0, pos);
            pendiente.erase(0, pos + 1);

            // Separar la linea por comas
            std::vector<std::string> partes;
            std::stringstream ss(linea);
            std::string parte;
            while (std::getline(ss, parte, ',')) {
                partes.push_back(parte);
            }

            // Validar que tenga 3 partes: placa, hora y celda
            if (partes.size() != 3) {
                std::cout << "Mensaje invalido: [" << linea << "]" << std::endl;
                continue;
            }

            std::string placa = partes[0];
            std::string hora = partes[1];
            int celda;
            try {
                celda = std::stoi(partes[2]);
            }
            catch (...) {
                std::cout << "Celda invalida: [" << linea << "]" << std::endl;
                continue;
            }

            std::cout << "Placa: " << placa << " | Hora: " << hora
                << " | Celda: " << celda << std::endl;
        }

        // Respuesta temporal: el cliente de prueba la espera. Se quita mas adelante.
        send(nuevo_socket, mensaje, (int)strlen(mensaje), 0);
    }

    // Cerrar sockets
    closesocket(nuevo_socket);
    closesocket(servidor_fd);
    WSACleanup();

    return 0;
}