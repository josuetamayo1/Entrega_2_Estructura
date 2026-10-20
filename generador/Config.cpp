#include "Config.h"
#include <fstream>
#include <iostream>
#include <cstdlib>   
#include <direct.h>  

Config::Config() : ip("127.0.0.1"), puerto(8080) {
}

std::string Config::getIp() const { return ip; }
int Config::getPuerto() const { return puerto; }

std::string Config::recortar(const std::string& s) {
    const std::string espacios = " \t\r\n";
    std::string::size_type ini = s.find_first_not_of(espacios);
    if (ini == std::string::npos) return "";
    std::string::size_type fin = s.find_last_not_of(espacios);
    return s.substr(ini, fin - ini + 1);
}

bool Config::cargar(const std::string& archivo) {
    std::ifstream entrada(archivo.c_str());
    if (!entrada.is_open()) {
        char carpeta[512];
        std::cout << "[CONFIG] No se encontro " << archivo;
        if (_getcwd(carpeta, sizeof(carpeta)) != NULL) {
            std::cout << " en la carpeta " << carpeta;
        }
        std::cout << ". Se usan los valores por defecto." << std::endl;
        return false;
    }

    std::string linea;
    bool primera = true;
    while (std::getline(entrada, linea)) {
        
        if (primera) {
            primera = false;
            if (linea.size() >= 3 && linea.compare(0, 3, "\xEF\xBB\xBF") == 0) {
                linea.erase(0, 3);
            }
        }

        linea = recortar(linea);
        if (linea.empty() || linea[0] == '#') continue;  

        std::string::size_type igual = linea.find('=');
        if (igual == std::string::npos) {
            std::cout << "[CONFIG] Linea ignorada (falta '='): " << linea << std::endl;
            continue;
        }

        std::string clave = recortar(linea.substr(0, igual));
        std::string valor = recortar(linea.substr(igual + 1));

        if (clave == "ip")          ip = valor;
        else if (clave == "puerto") puerto = atoi(valor.c_str());
        else std::cout << "[CONFIG] Clave desconocida, se ignora: " << clave << std::endl;
    }

    std::cout << "[CONFIG] Configuracion cargada de " << archivo << std::endl;
    return true;
}