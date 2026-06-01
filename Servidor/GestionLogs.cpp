// GestionLogs.cpp
#include "GestionLogs.h"
#include <iostream>
#include <fstream>
#include <ctime>

void registrarLog(const std::string& mensaje) {
    // Abrimos en modo Append (ios::app) para no borrar los logs anteriores
    std::ofstream logFile("servidor.log", std::ios::app);
    
    // Capturamos la hora actual del sistema
    std::time_t ahora = std::time(nullptr);
    char bufferHora[26];
    
    // Usamos ctime_s o ctime dependiendo del compilador para asegurar compatibilidad
#if defined(_MSC_VER)
    ctime_s(bufferHora, sizeof(bufferHora), &ahora);
#else
    std::string strHora = std::ctime(&ahora);
    snprintf(bufferHora, sizeof(bufferHora), "%s", strHora.c_str());
#endif

    // Quitamos el salto de línea que ctime mete automáticamente al final
    std::string stringHora(bufferHora);
    if (!stringHora.empty() && stringHora.back() == '\n') {
        stringHora.pop_back();
    }

    // Escribimos en el archivo con formato profesional
    if (logFile.is_open()) {
        logFile << "[" << stringHora << "] " << mensaje << std::endl;
        logFile.close();
    }

    // También lo sacamos por la pantalla del servidor para monitorizarlo en vivo
    std::cout << "[LOG] " << mensaje << std::endl;
}