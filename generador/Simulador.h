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
    void ejecutar();      

private:
    Parqueadero parqueadero;
    GeneradorPlacas generador;
    ClienteTCP cliente;

    bool decidirEntrada() const;             
    bool procesarEntrada();                    
    bool procesarSalida();
    bool enviar(const std::string& texto);     
    void esperarIntervalo() const;
};

#endif