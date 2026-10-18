#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    Config();                                   // valores por defecto: 127.0.0.1 y 8080
    bool cargar(const std::string& archivo);    // lee el archivo; false si no existe

    std::string getIp() const;
    int getPuerto() const;

private:
    std::string ip;
    int puerto;

    static std::string recortar(const std::string& s);   // quita espacios de los extremos
};

#endif