#ifndef EVENTO_H
#define EVENTO_H

#include <string>

class Evento {
public:
    Evento(const std::string& placa, int celda);   // toma la hora actual al crearse

    std::string aTexto() const;    // "ABC123,18:30:05,12\n"

private:
    std::string placa;
    std::string hora;              // "HH:MM:SS"
    int celda;

    static std::string horaActual();
};

#endif