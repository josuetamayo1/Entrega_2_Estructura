#include "Simulador.h"
#include "Evento.h"
#include <iostream>
#include <cstdlib>   // rand

static const int PROB_ENTRADA = 50;        // % de probabilidad de entrada (si no esta lleno ni vacio)
static const int INTERVALO_MIN_MS = 2000;
static const int INTERVALO_MAX_MS = 5000;

Simulador::Simulador(const Config& config)
    : cliente(config.getIp(), config.getPuerto()) {
}

void Simulador::ejecutar() {
    std::cout << "[GENERADOR] Iniciado. Se enviara un evento cada "
        << INTERVALO_MIN_MS / 1000 << " a " << INTERVALO_MAX_MS / 1000
        << " segundos. Cierra la ventana para terminar." << std::endl;

    while (true) {
        bool ok = decidirEntrada() ? procesarEntrada() : procesarSalida();
        if (!ok) {
            std::cerr << "[GENERADOR] No se puede continuar: revisa la IP y el puerto en config.txt." << std::endl;
            return;
        }
        esperarIntervalo();
    }
}

bool Simulador::decidirEntrada() const {
    if (parqueadero.estaLleno()) return false;    // lleno: tiene que ser salida
    if (parqueadero.estaVacio()) return true;     // vacio: tiene que ser entrada
    return (rand() % 100) < PROB_ENTRADA;
}

bool Simulador::procesarEntrada() {
    // La placa no puede estar ya dentro: el receptor la tomaria como una salida
    std::string placa;
    do {
        placa = generador.generar();
    } while (parqueadero.placaDentro(placa));

    int celda = parqueadero.elegirCeldaLibre();
    Evento evento(placa, celda);
    std::string texto = evento.aTexto();

    std::cout << "ENTRADA " << texto;
    if (!enviar(texto)) return false;

    parqueadero.ocupar(celda, placa);             // solo si el envio tuvo exito
    return true;
}

bool Simulador::procesarSalida() {
    int celda = parqueadero.elegirCeldaOcupada();
    std::string placa = parqueadero.placaEn(celda);
    Evento evento(placa, celda);                  // misma placa y misma celda que ocupaba
    std::string texto = evento.aTexto();

    std::cout << "SALIDA  " << texto;
    if (!enviar(texto)) return false;

    parqueadero.liberar(celda);                   // solo si el envio tuvo exito
    return true;
}

bool Simulador::enviar(const std::string& texto) {
    while (!cliente.enviar(texto)) {
        if (!cliente.reconectar()) return false;
    }
    return true;
}

void Simulador::esperarIntervalo() const {
    int espera = INTERVALO_MIN_MS + rand() % (INTERVALO_MAX_MS - INTERVALO_MIN_MS + 1);
    Sleep(espera);
}