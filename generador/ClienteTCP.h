#ifndef CLIENTETCP_H
#define CLIENTETCP_H

#include <winsock2.h>
#include <string>

class ClienteTCP {
public:
    ClienteTCP(const std::string& ip, int puerto);
    ~ClienteTCP();

    bool conectar();                        
    bool reconectar();                      
    
    bool enviar(const std::string& texto);  
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

    
    ClienteTCP(const ClienteTCP&);
    ClienteTCP& operator=(const ClienteTCP&);
};

#endif