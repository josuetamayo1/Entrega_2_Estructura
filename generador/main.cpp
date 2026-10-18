#include <cstdlib>
#include <ctime>
#include "Config.h"
#include "Simulador.h"

int main() {
    srand((unsigned)time(NULL));   // UNA sola vez, al inicio del programa

    Config config;
    config.cargar("config.txt");

    Simulador simulador(config);
    simulador.ejecutar();
    return 0;
}