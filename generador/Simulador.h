#ifndef SIMULADOR_H
#define SIMULADOR_H

#include <string>
#include "Parqueadero.h"
#include "GeneradorPlacas.h"
#include "ClienteTCP.h"
#include "Config.h"

class Simulador {
public:
    Simulador(const Config& config);
    void ejecutar();      // ciclo infinito: un evento cada 2 a 5 segundos

private:
    Parqueadero parqueadero;
    GeneradorPlacas generador;
    ClienteTCP cliente;

    bool decidirEntrada() const;               // true = entrada, false = salida
    bool procesarEntrada();                    // false solo si la configuracion es invalida
    bool procesarSalida();
    bool enviar(const std::string& texto);     // reintenta el mismo mensaje hasta lograrlo
    void esperarIntervalo() const;
};

#endif