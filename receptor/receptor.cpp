// ============================================================================
//  receptor_dll.cpp  -  Libreria dinamica (DLL) del receptor de placas
//
//  Funciones exportadas (contrato del equipo):
//    int  iniciar_receptor(int puerto);              // 1 = ok, 0 = error
//    int  obtener_mensaje(char* buffer, int tamano); // 1 = hay mensaje, 0 = no hay
//    void detener_receptor();
//
//  Formato de cada mensaje que entrega: "placa,hora,celda"  (ej. ABC123,18:30:05,12)
// ============================================================================

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

#pragma comment(lib, "ws2_32.lib")  // Enlaza la libreria Winsock

// Macro para exportar funciones con nombre "plano" (sin decoracion de C++),
// asi Python las encuentra por su nombre exacto.
#define EXPORTAR extern "C" __declspec(dllexport)

// ===================== Estado interno de la DLL =====================
static std::queue<std::string> cola_mensajes;   // mensajes validos recibidos
static std::mutex mutex_cola;                    // protege la cola

static std::atomic<bool> activo(false);          // true mientras el receptor corre
static SOCKET servidor_fd = INVALID_SOCKET;      // socket que escucha
static std::atomic<SOCKET> cliente_fd(INVALID_SOCKET); // socket del generador conectado

// Se usa un puntero para que, si Python se cierra sin llamar a detener_receptor(),
// no se destruya un hilo activo (eso cerraria el programa con error).
static std::thread* hilo = nullptr;

// ===================== Cola protegida con mutex =====================
static void encolar_mensaje(const std::string& msg) {
    std::lock_guard<std::mutex> candado(mutex_cola);
    cola_mensajes.push(msg);
}

// ===================== Validar una linea completa =====================
// Si tiene el formato placa,hora,celda la mete en la cola; si no, la descarta.
static void procesar_linea(const std::string& linea) {
    std::vector<std::string> partes;
    std::stringstream ss(linea);
    std::string parte;
    while (std::getline(ss, parte, ',')) {
        partes.push_back(parte);
    }
    if (partes.size() != 3) {
        return;  // mensaje invalido
    }

    int celda;
    try {
        celda = std::stoi(partes[2]);
    }
    catch (...) {
        return;  // celda invalida
    }

    encolar_mensaje(partes[0] + "," + partes[1] + "," + std::to_string(celda));
}

// ===================== Hilo de recepcion =====================
// Acepta un generador, recibe sus mensajes y, si se desconecta,
// vuelve a esperar otro. Termina cuando se llama a detener_receptor().
static void hilo_recepcion() {
    char buffer[1024];

    while (activo) {
        SOCKET s = accept(servidor_fd, nullptr, nullptr);
        if (s == INVALID_SOCKET) {
            // Si fallo porque detener_receptor() cerro el socket, salimos
            if (!activo) break;
            continue;
        }
        cliente_fd = s;

        std::string pendiente;  // acumulador de lo recibido
        while (activo) {
            int valread = recv(s, buffer, sizeof(buffer) - 1, 0);
            if (valread <= 0) {
                break;  // el generador se desconecto (o se detuvo el receptor)
            }
            buffer[valread] = '\0';
            pendiente += buffer;

            // Sacar todos los mensajes completos (terminados en \n)
            size_t pos;
            while ((pos = pendiente.find('\n')) != std::string::npos) {
                std::string linea = pendiente.substr(0, pos);
                pendiente.erase(0, pos + 1);
                if (!linea.empty() && linea.back() == '\r') {
                    linea.pop_back();  // por si llega con \r\n
                }
                procesar_linea(linea);
            }
        }

        cliente_fd = INVALID_SOCKET;
        closesocket(s);
    }
}

// ===================== Funciones exportadas =====================

EXPORTAR int iniciar_receptor(int puerto) {
    if (activo) {
        return 0;  // ya estaba iniciado
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
    hilo = new std::thread(hilo_recepcion);  // la recepcion corre en segundo plano
    return 1;                                // y la funcion regresa de inmediato
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
        n = (size_t)(tamano - 1);  // no escribir fuera del buffer de Python
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

    // Cerrar el socket que escucha desbloquea el accept()
    closesocket(servidor_fd);
    servidor_fd = INVALID_SOCKET;

    // Cortar la conexion del generador desbloquea el recv()
    SOCKET c = cliente_fd;
    if (c != INVALID_SOCKET) {
        shutdown(c, SD_BOTH);
    }

    if (hilo != nullptr) {
        hilo->join();  // esperar a que el hilo termine
        delete hilo;
        hilo = nullptr;
    }

    {
        std::lock_guard<std::mutex> candado(mutex_cola);
        std::queue<std::string> vacia;
        std::swap(cola_mensajes, vacia);  // vaciar la cola
    }

    WSACleanup();
}