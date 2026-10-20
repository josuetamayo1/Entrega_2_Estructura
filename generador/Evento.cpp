#define _CRT_SECURE_NO_WARNINGS   
#include "Evento.h"
#include <sstream>
#include <ctime>

Evento::Evento(const std::string& placa, int celda)
    : placa(placa), hora(horaActual()), celda(celda) {
}

std::string Evento::horaActual() {
    time_t t = time(NULL);
    struct tm* info = localtime(&t);
    char buf[9];                              
    strftime(buf, sizeof(buf), "%H:%M:%S", info);
    return std::string(buf);
}

std::string Evento::aTexto() const {
    std::ostringstream ss;
    ss << placa << "," << hora << "," << celda << "\n";   
    return ss.str();
}