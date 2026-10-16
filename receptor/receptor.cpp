#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define EXPORTAR extern "C" __declspec(dllexport)

static std::queue<std::string> cola_mensajes;
static std::mutex mutex_cola;

static std::atomic<bool> activo(false);
static SOCKET servidor_fd = INVALID_SOCKET;
static std::atomic<SOCKET> cliente_fd(INVALID_SOCKET);
static std::thread* hilo = nullptr;

static void encolar_mensaje(const std::string& msg) {
    std::lock_guard<std::mutex> candado(mutex_cola);
    cola_mensajes.push(msg);
}

// Valida "placa,hora,celda"; si es correcto lo mete en la cola
static void procesar_linea(const std::string& linea) {
    std::vector<std::string> partes;
    std::stringstream ss(linea);
    std::string parte;
    while (std::getline(ss, parte, ',')) {
        partes.push_back(parte);
    }
    if (partes.size() != 3) {
        return;
    }

    int celda;
    try {
        celda = std::stoi(partes[2]);
    }
    catch (...) {
        return;
    }

    encolar_mensaje(partes[0] + "," + partes[1] + "," + std::to_string(celda));
}

static void hilo_recepcion() {
    char buffer[1024];

    while (activo) {
        SOCKET s = accept(servidor_fd, nullptr, nullptr);
        if (s == INVALID_SOCKET) {
            if (!activo) break;
            continue;
        }
        cliente_fd = s;

        std::string pendiente;
        while (activo) {
            int valread = recv(s, buffer, sizeof(buffer) - 1, 0);
            if (valread <= 0) {
                break;
            }
            buffer[valread] = '\0';
            pendiente += buffer;

            size_t pos;
            while ((pos = pendiente.find('\n')) != std::string::npos) {
                std::string linea = pendiente.substr(0, pos);
                pendiente.erase(0, pos + 1);
                if (!linea.empty() && linea.back() == '\r') {
                    linea.pop_back();
                }
                procesar_linea(linea);
            }
        }

        cliente_fd = INVALID_SOCKET;
        closesocket(s);
    }
}

EXPORTAR int iniciar_receptor(int puerto) {
    if (activo) {
        return 0;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 0;
    }

    servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_fd == INVALID_SOCKET) {
        WSACleanup();
        return 0;
    }

    sockaddr_in direccion{};
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons((u_short)puerto);

    if (bind(servidor_fd, (sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR ||
        listen(servidor_fd, 3) == SOCKET_ERROR) {
        closesocket(servidor_fd);
        servidor_fd = INVALID_SOCKET;
        WSACleanup();
        return 0;
    }

    activo = true;
    hilo = new std::thread(hilo_recepcion);
    return 1;
}

EXPORTAR int obtener_mensaje(char* buffer, int tamano) {
    if (buffer == nullptr || tamano <= 0) {
        return 0;
    }

    std::lock_guard<std::mutex> candado(mutex_cola);
    if (cola_mensajes.empty()) {
        return 0;
    }

    const std::string& msg = cola_mensajes.front();
    size_t n = msg.size();
    if (n > (size_t)(tamano - 1)) {
        n = (size_t)(tamano - 1);
    }
    memcpy(buffer, msg.c_str(), n);
    buffer[n] = '\0';

    cola_mensajes.pop();
    return 1;
}

EXPORTAR void detener_receptor() {
    if (!activo) {
        return;
    }
    activo = false;

    // Cerrar los sockets desbloquea accept() y recv() para que el hilo termine
    closesocket(servidor_fd);
    servidor_fd = INVALID_SOCKET;

    SOCKET c = cliente_fd;
    if (c != INVALID_SOCKET) {
        shutdown(c, SD_BOTH);
    }

    if (hilo != nullptr) {
        hilo->join();
        delete hilo;
        hilo = nullptr;
    }

    {
        std::lock_guard<std::mutex> candado(mutex_cola);
        std::queue<std::string> vacia;
        std::swap(cola_mensajes, vacia);
    }

    WSACleanup();
}