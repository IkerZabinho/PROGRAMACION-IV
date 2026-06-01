// ConfigServidor.h
#pragma once
#include <string>
#include <fstream>
#include <sstream>

class ConfigServidor {
private:
    std::string dbPath;
    int puerto;

public:
    ConfigServidor() {
        // Valores por defecto por si el fichero falla
        dbPath = "asociacion.db"; 
        puerto = 8888;
        
        // Intentar leer tu archivo config.conf de la Fase 1
        std::ifstream archivo("config.conf");
        if (archivo.is_open()) {
            std::string linea;
            while (std::getline(archivo, linea)) {
                std::istringstream iss(linea);
                std::string clave, valor;
                if (std::getline(iss, clave, '=') && std::getline(iss, valor)) {
                    if (clave == "DB_PATH" || clave == "ruta_db") dbPath = valor;
                    if (clave == "PUERTO" || clave == "puerto") puerto = std::stoi(valor);
                }
            }
            archivo.close();
        }
    }

    std::string getDbPath() const { return dbPath; }
    int getPuerto() const { return puerto; }
};