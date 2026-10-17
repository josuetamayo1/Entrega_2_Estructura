#ifndef CLIENTETCP_H
#define CLIENTETCP_H

#include <winsock2.h>
#include <string>

class ClienteTCP {
public:
    ClienteTCP(const std::string& ip, int puerto);
    ~ClienteTCP();

    bool conectar();                        // un solo intento; false si falla
    bool reconectar();                      // reintenta cada 3 s hasta lograrlo;
    // false solo si la IP o el puerto son invalidos
    bool enviar(const std::string& texto);  // false si no se pudo enviar (conexion perdida)
    void cerrar();
    bool estaConectado() const;

private:
    std::string ip;
    int puerto;
    SOCKET sock;
    sockaddr_in direccion;
    bool winsockIniciado;
    bool configValida;
    bool conectado;

    bool conexionViva();
    static std::string describirError(int codigo);

    // Un ClienteTCP no se puede copiar: dos copias cerrarian el mismo socket
    ClienteTCP(const ClienteTCP&);
    ClienteTCP& operator=(const ClienteTCP&);
};

#endif