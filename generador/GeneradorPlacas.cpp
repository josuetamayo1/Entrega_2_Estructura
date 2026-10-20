#include "GeneradorPlacas.h"
#include <cstdlib>   

std::string GeneradorPlacas::generar() const {
    std::string placa = "";

    
    for (int i = 0; i < 3; i++) {
        placa += (char)('A' + rand() % 26);
    }

    
    for (int i = 0; i < 3; i++) {
        placa += (char)('0' + rand() % 10);
    }

    return placa;
}