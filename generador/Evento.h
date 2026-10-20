#ifndef EVENTO_H
#define EVENTO_H

#include <string>

class Evento {
public:
    Evento(const std::string& placa, int celda);  

    std::string aTexto() const;    

private:
    std::string placa;
    std::string hora;              
    int celda;

    static std::string horaActual();
};

#endif