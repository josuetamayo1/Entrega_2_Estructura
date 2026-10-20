#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    Config();                                   
    bool cargar(const std::string& archivo);    

    std::string getIp() const;
    int getPuerto() const;

private:
    std::string ip;
    int puerto;

    static std::string recortar(const std::string& s);   
};

#endif