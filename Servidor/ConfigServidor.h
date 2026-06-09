// ConfigServidor.h
#pragma once
#include <string>
#include <fstream>
#include <sstream>

class ConfigServidor {
private:
    std::string dbPath;
    int puerto;
    std::string adminUser;
    std::string adminPass;
    std::string logNivel;
    bool cargadoDesdeArchivo = false;

public:
    ConfigServidor() {
        // Valores de emergencia — solo si config.conf no existe o falla
        dbPath    = "asociacion.db";
        puerto    = 8888;
        adminUser = "admin";
        adminPass = "12";
        logNivel  = "INFO";

        std::ifstream archivo("config.conf");
        if (archivo.is_open()) {
            std::string linea;
            while (std::getline(archivo, linea)) {
                // Ignorar líneas vacías y comentarios (#)
                if (linea.empty() || linea[0] == '#') continue;

                std::istringstream iss(linea);
                std::string clave, valor;
                if (std::getline(iss, clave, '=') && std::getline(iss, valor)) {
                    // Eliminar espacios en blanco alrededor de clave y valor
                    clave.erase(0, clave.find_first_not_of(" \t"));
                    clave.erase(clave.find_last_not_of(" \t") + 1);
                    valor.erase(0, valor.find_first_not_of(" \t"));
                    valor.erase(valor.find_last_not_of(" \t") + 1);

                    if (clave == "DB_PATH")    dbPath    = valor;
                    if (clave == "PUERTO")     puerto    = std::stoi(valor);
                    if (clave == "ADMIN_USER") adminUser = valor;
                    if (clave == "ADMIN_PASS") adminPass = valor;
                    if (clave == "LOG_NIVEL")  logNivel  = valor;
                }
            }
            archivo.close();
            cargadoDesdeArchivo = true;
        }
    }

    bool seCargo() const { return cargadoDesdeArchivo; }

    std::string getDbPath()    const { return dbPath; }
    int         getPuerto()    const { return puerto; }
    std::string getAdminUser() const { return adminUser; }
    std::string getAdminPass() const { return adminPass; }
    std::string getLogNivel()  const { return logNivel; }
};