#define _WINSOCK_DEPRECATED_NO_WARNINGS   // inet_addr es "obsoleta" en Visual Studio
#include "ClienteTCP.h"
#include <iostream>
#include <sstream>
#include <cstring>   // memset

#pragma comment(lib, "ws2_32.lib")   // Visual Studio; en Dev-C++ se usa -lws2_32

static const int PAUSA_REINTENTO_MS = 3000;

ClienteTCP::ClienteTCP(const std::string& ip, int puerto)
    : ip(ip), puerto(puerto), sock(INVALID_SOCKET),
    winsockIniciado(false), configValida(false), conectado(false) {

    WSADATA wsaData;
    int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r != 0) {
        std::cerr << "[ERROR] No se pudo inicializar Winsock (codigo " << r << ")" << std::endl;
        return;
    }
    winsockIniciado = true;

    memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons((u_short)puerto);
    direccion.sin_addr.s_addr = inet_addr(ip.c_str());

    if (puerto < 1 || puerto > 65535) {
        std::cerr << "[ERROR] Puerto invalido: " << puerto
            << " (debe estar entre 1 y 65535)" << std::endl;
    }
    else if (direccion.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "[ERROR] IP invalida: " << ip << std::endl;
    }
    else {
        configValida = true;
    }
}

ClienteTCP::~ClienteTCP() {
    cerrar();
    if (winsockIniciado) WSACleanup();
}

bool ClienteTCP::conectar() {
    if (!winsockIniciado || !configValida) return false;  // el error ya se mostro en el constructor

    cerrar();   // por si quedaba un socket anterior

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[ERROR] No se pudo crear el socket: "
            << describirError(WSAGetLastError()) << std::endl;
        return false;
    }

    if (connect(sock, (sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        std::cerr << "[ERROR] No se pudo conectar a " << ip << ":" << puerto
            << " - " << describirError(WSAGetLastError()) << std::endl;
        cerrar();
        return false;
    }

    conectado = true;
    std::cout << "[OK] Conectado a " << ip << ":" << puerto << std::endl;
    return true;
}

bool ClienteTCP::reconectar() {
    if (!winsockIniciado || !configValida) return false;  // reintentar no serviria de nada

    cerrar();
    int intento = 1;
    while (!conectar()) {
        std::cout << "[RECONECTANDO] Intento " << intento << " fallido. Reintentando en "
            << PAUSA_REINTENTO_MS / 1000 << " s..." << std::endl;
        Sleep(PAUSA_REINTENTO_MS);
        intento++;
    }
    return true;
}

bool ClienteTCP::enviar(const std::string& texto) {
    if (!conectado) return false;

    // Verificar antes de enviar: si el servidor ya cerro, el send "funcionaria"
    // y el mensaje se perderia sin que nos enteremos
    if (!conexionViva()) {
        std::cerr << "[ERROR] El servidor cerro la conexion." << std::endl;
        cerrar();
        return false;
    }

    int total = (int)texto.size();
    int enviados = 0;
    while (enviados < total) {          // send puede enviar menos de lo pedido
        int n = send(sock, texto.c_str() + enviados, total - enviados, 0);
        if (n == SOCKET_ERROR) {
            std::cerr << "[ERROR] Fallo al enviar: "
                << describirError(WSAGetLastError()) << std::endl;
            cerrar();
            return false;
        }
        enviados += n;
    }
    return true;
}

void ClienteTCP::cerrar() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    conectado = false;
}

bool ClienteTCP::estaConectado() const {
    return conectado;
}

bool ClienteTCP::conexionViva() {
    while (true) {
        fd_set lectura;
        FD_ZERO(&lectura);
        FD_SET(sock, &lectura);

        timeval espera;
        espera.tv_sec = 0;      // no esperar: solo preguntar
        espera.tv_usec = 0;

        int r = select(0, &lectura, NULL, NULL, &espera);
        if (r == SOCKET_ERROR) return false;
        if (r == 0) return true;        // nada pendiente: la conexion sigue abierta

        // Hay algo para leer: o son datos del servidor, o es el aviso de cierre
        char descarte[256];
        int n = recv(sock, descarte, sizeof(descarte), 0);
        if (n == 0 || n == SOCKET_ERROR) return false;   // el servidor cerro

        // n > 0: eran datos (por ejemplo una respuesta). Se descartan,
        // porque el contrato no define respuestas, y se vuelve a preguntar
    }
}
std::string ClienteTCP::describirError(int codigo) {
    std::ostringstream ss;
    switch (codigo) {
    case WSAECONNREFUSED: ss << "conexion rechazada (no hay servidor escuchando en ese puerto; esta apagado?)"; break;
    case WSAETIMEDOUT:    ss << "tiempo de espera agotado (revisa la IP o el firewall)"; break;
    case WSAECONNRESET:   ss << "el servidor cerro la conexion de golpe"; break;
    case WSAECONNABORTED: ss << "conexion abortada"; break;
    case WSAENETUNREACH:  ss << "red inalcanzable"; break;
    case WSAEHOSTUNREACH: ss << "no hay ruta hacia el equipo (revisa la IP)"; break;
    default:              ss << "error de Winsock"; break;
    }
    ss << " [codigo " << codigo << "]";
    return ss.str();
}