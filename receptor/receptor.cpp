#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")  

#define EXPORTAR extern "C" __declspec(dllexport)


static const bool ENVIAR_ACK = true;

static const size_t MAX_COLA = 1000;

static SOCKET g_servidor = INVALID_SOCKET;
static std::thread g_hilo;
static std::atomic<bool> g_corriendo(false);
static std::mutex g_mutex;              
static std::queue<std::string> g_cola;


static void encolar(std::string linea) {
    while (!linea.empty() && linea.back() == '\r') {
        linea.pop_back();
    }
    if (linea.empty()) return;

    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_cola.size() >= MAX_COLA) {
        g_cola.pop();
    }
    g_cola.push(std::move(linea));
}


static void atender_cliente(SOCKET cliente) {
    std::string pendiente;  
    char buffer[1024];

    while (g_corriendo) {
        fd_set lectura;
        FD_ZERO(&lectura);
        FD_SET(cliente, &lectura);
        timeval espera;
        espera.tv_sec = 0;
        espera.tv_usec = 200000; 

        int r = select(0, &lectura, nullptr, nullptr, &espera);
        if (r == SOCKET_ERROR) break;
        if (r == 0) continue;  

        int n = recv(cliente, buffer, (int)sizeof(buffer), 0);
        if (n <= 0) break;  

        pendiente.append(buffer, n);

        size_t pos;
        while ((pos = pendiente.find('\n')) != std::string::npos) {
            encolar(pendiente.substr(0, pos));
            pendiente.erase(0, pos + 1);
        }

        if (pendiente.size() > 4096) {
            pendiente.clear();
        }

        if (ENVIAR_ACK) {
            send(cliente, "Recibido", 8, 0);
        }
    }
    closesocket(cliente);
}


static void bucle_servidor() {
    while (g_corriendo) {
        fd_set lectura;
        FD_ZERO(&lectura);
        FD_SET(g_servidor, &lectura);
        timeval espera;
        espera.tv_sec = 0;
        espera.tv_usec = 200000; 

        int r = select(0, &lectura, nullptr, nullptr, &espera);
        if (r == SOCKET_ERROR) break;
        if (r == 0) continue; 

        SOCKET cliente = accept(g_servidor, nullptr, nullptr);
        if (cliente == INVALID_SOCKET) continue;

        atender_cliente(cliente);
    }
}


EXPORTAR int iniciar_receptor(int puerto) {
    if (g_corriendo) return 1;  
    if (puerto < 1 || puerto > 65535) return 0;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 0;

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        WSACleanup();
        return 0;
    }

    sockaddr_in direccion;
    std::memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons((u_short)puerto);

    if (bind(s, (sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR ||
        listen(s, 3) == SOCKET_ERROR) {
        closesocket(s);
        WSACleanup();
        return 0;
    }

    {
     
        std::lock_guard<std::mutex> lock(g_mutex);
        std::queue<std::string>().swap(g_cola);
    }

    g_servidor = s;
    g_corriendo = true;
    g_hilo = std::thread(bucle_servidor);
    return 1;
}


EXPORTAR int obtener_mensaje(char* buffer, int tamano) {
    if (buffer == nullptr || tamano <= 0) return 0;

    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_cola.empty()) return 0;  

    const std::string& mensaje = g_cola.front();
    size_t n = std::min(mensaje.size(), (size_t)(tamano - 1));
    std::memcpy(buffer, mensaje.data(), n);
    buffer[n] = '\0';  
    return 1;
}


EXPORTAR void detener_receptor() {
    if (!g_corriendo) return;

    g_corriendo = false;  
    if (g_hilo.joinable()) {
        g_hilo.join();
    }
    if (g_servidor != INVALID_SOCKET) {
        closesocket(g_servidor);
        g_servidor = INVALID_SOCKET;
    }
    WSACleanup();
}