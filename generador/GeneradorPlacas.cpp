#include "GeneradorPlacas.h"
#include <cstdlib>   // rand

std::string GeneradorPlacas::generar() const {
    std::string placa = "";

    // 3 letras mayúsculas: 'A' + un número entre 0 y 25
    for (int i = 0; i < 3; i++) {
        placa += (char)('A' + rand() % 26);
    }

    // 3 dígitos: '0' + un número entre 0 y 9
    for (int i = 0; i < 3; i++) {
        placa += (char)('0' + rand() % 10);
    }

    return placa;
}