#include "Parqueadero.h"
#include <cstdlib>   // rand

Parqueadero::Parqueadero() : ocupadas(0) {
    // Los 20 strings nacen vacíos, o sea todas las celdas libres.
}

bool Parqueadero::celdaValida(int celda) const {
    return celda >= 1 && celda <= NUM_CELDAS;
}

bool Parqueadero::estaLleno() const  { return ocupadas == NUM_CELDAS; }
bool Parqueadero::estaVacio() const  { return ocupadas == 0; }

bool Parqueadero::placaDentro(const std::string& placa) const {
    if (placa.empty()) return false;  // "" coincidiría con las celdas libres
    for (int i = 0; i < NUM_CELDAS; i++) {
        if (celdas[i] == placa) return true;
    }
    return false;
}

std::string Parqueadero::placaEn(int celda) const {
    if (!celdaValida(celda)) return "";
    return celdas[celda - 1];
}

int Parqueadero::elegirCeldaLibre() const {
    int libres = NUM_CELDAS - ocupadas;
    if (libres == 0) return 0;

    int k = rand() % libres;          // "la k-ésima celda libre"
    for (int i = 0; i < NUM_CELDAS; i++) {
        if (celdas[i].empty()) {
            if (k == 0) return i + 1;
            k--;
        }
    }
    return 0;  // no debería llegar aquí
}

int Parqueadero::elegirCeldaOcupada() const {
    if (ocupadas == 0) return 0;

    int k = rand() % ocupadas;        // "la k-ésima celda ocupada"
    for (int i = 0; i < NUM_CELDAS; i++) {
        if (!celdas[i].empty()) {
            if (k == 0) return i + 1;
            k--;
        }
    }
    return 0;
}

bool Parqueadero::ocupar(int celda, const std::string& placa) {
    if (!celdaValida(celda) || placa.empty()) return false;
    if (!celdas[celda - 1].empty()) return false;   // ya estaba ocupada
    celdas[celda - 1] = placa;
    ocupadas++;
    return true;
}

bool Parqueadero::liberar(int celda) {
    if (!celdaValida(celda)) return false;
    if (celdas[celda - 1].empty()) return false;    // ya estaba libre
    celdas[celda - 1] = "";
    ocupadas--;
    return true;
}
