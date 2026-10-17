#ifndef PARQUEADERO_H
#define PARQUEADERO_H

#include <string>

class Parqueadero {
public:
    static const int NUM_CELDAS = 30;  

    Parqueadero();

    // Consultas (no modifican nada)
    bool estaLleno() const;
    bool estaVacio() const;
    bool placaDentro(const std::string& placa) const;
    std::string placaEn(int celda) const;   // "" si la celda está libre o es inválida
    int elegirCeldaLibre() const;           // celda libre al azar, 0 si está lleno
    int elegirCeldaOcupada() const;         // celda ocupada al azar, 0 si está vacío

    // Modificaciones (devuelven false si la operación no es válida)
    bool ocupar(int celda, const std::string& placa);
    bool liberar(int celda);

private:
    std::string celdas[NUM_CELDAS];  // "" = libre
    int ocupadas;                    // contador para lleno/vacío en O(1)

    bool celdaValida(int celda) const;
};

#endif
