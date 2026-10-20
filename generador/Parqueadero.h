#ifndef PARQUEADERO_H
#define PARQUEADERO_H

#include <string>

class Parqueadero {
public:
    static const int NUM_CELDAS = 30;  

    Parqueadero();

    
    bool estaLleno() const;
    bool estaVacio() const;
    bool placaDentro(const std::string& placa) const;
    std::string placaEn(int celda) const;   
    int elegirCeldaLibre() const;           
    int elegirCeldaOcupada() const;        

    
    bool ocupar(int celda, const std::string& placa);
    bool liberar(int celda);

private:
    std::string celdas[NUM_CELDAS];  
    int ocupadas;                    

    bool celdaValida(int celda) const;
};

#endif
