#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "Parqueadero.h"
#include "GeneradorPlacas.h"
using namespace std;

int main() {
    srand(time(NULL));   // UNA sola vez, al inicio del programa

    const int TOTAL_PLACAS = 40;
    Parqueadero p;
    GeneradorPlacas gen;
    int salidas = 0;

    for (int i = 1; i <= TOTAL_PLACAS; i++) {
        // Si no hay espacio, sale una placa al azar para dar cupo
        if (p.estaLleno()) {
            int s = p.elegirCeldaOcupada();
            cout << "Sale  " << p.placaEn(s) << " de la celda " << s << endl;
            p.liberar(s);
            salidas++;
        }

        // Generar una placa que NO esté ya dentro
        string placa;
        do {
            placa = gen.generar();
        } while (p.placaDentro(placa));

        int c = p.elegirCeldaLibre();
        p.ocupar(c, placa);
        cout << "Entra " << placa << " en la celda " << c << endl;
    }

    cout << endl;
    cout << "Placas que entraron: " << TOTAL_PLACAS << endl;
    cout << "Salidas por falta de cupo: " << salidas << endl;
    return 0;
}