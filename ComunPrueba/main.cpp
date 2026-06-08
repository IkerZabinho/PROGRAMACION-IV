#include <iostream>
#include <cstdio>       // Para FILE, fopen, fprintf, fclose si hicieran falta
#include <windows.h>     // Mantenemos la librería de Windows para las tildes y la Ñ
#include "sqlite3.h"
#include "../ComunPrueba/Clases.h"     // Acceso a tus clases (incluye Config con std::string)
#include "../ComunPrueba/interfaz.h"   // Prototipos de los menús e inicio de sesión

using namespace std;
using namespace GestionONG;

// Función para generar el reporte simulado o adaptado a C++ de tu config
void generarReporteResumenCPP(sqlite3* db, const string& nombre_reporte) {
    // Aquí iría la lógica o llamada al reporte si fuera externa,
    // pero ahora usamos directamente el método estático de la clase Config.
}

int main() {
    sqlite3 *db = NULL;
    
    // Configuración para que se vean bien las tildes y la Ñ en Windows
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // 1. CARGAR CONFIGURACIÓN USANDO TU CLASE CONFIGactualizarDatosBeneficiario
    Config miConfig;
    
    // Llamamos a tu función real pasándole la dirección de memoria de tu objeto (&miConfig)
    if (miConfig.cargar_configuracion("config.conf", &miConfig) == 0) { 
        cout << "Error: No se encontró el archivo de configuración." << endl;
        return 1;
    }

    // Acceso directo a la variable pública std::string sin métodos get
    cout << "Iniciando servidor con usuario: " << miConfig.admin_user << endl;

    // 2. ABRIR LA BASE DE DATOS USANDO LA RUTA DEL CONFIG (.c_str() es obligatorio para sqlite3_open)
    if (sqlite3_open(miConfig.db_path.c_str(), &db) != SQLITE_OK) {
        cout << "Error al abrir la BD definida en config: " << miConfig.db_path << endl;
        return 1;
    }

    // Activar claves foráneas (importante para que los DELETE funcionen en cascada)
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, 0);
    cout << "[SISTEMA] Base de datos conectada correctamente en C++." << endl;

    // 3. MANTENIMIENTO AUTOMÁTICO AL ARRANCAR
    // Usamos el nombre exacto que tienes declarado en tu interfaz.h
    GestionONG::crearEventoJuevesRopaAutomatico(db);
//
    // 4. BUCLE PRINCIPAL DEL MENÚ
    int opcion;
    do {
        cout << "\n==============================";
        cout << "\n   SISTEMA DE GESTIÓN ONG (C++)";
        cout << "\n==============================";
        cout << "\n1. Iniciar Sesión";
        cout << "\n2. Registrarse";
        cout << "\n0. Salir";
        cout << "\n------------------------------";
        cout << "\nSeleccione una opción: ";
        
        // Validación de entrada numérica en C++ (evita bucles infinitos si meten letras)
        if (!(cin >> opcion)) {
            cout << "\n[!] Ups, parece que no has introducido un número.";
            cout << "\nPor favor, elige una opción del 0 al 2.\n";
            cin.clear();              // Limpia el estado de error del flag cin
            cin.ignore(10000, '\n');  // Descarta los caracteres erróneos del buffer
            opcion = -1;              // Valor neutro para repetir el bucle
            continue;
        }

        switch (opcion) {
            case 1:
                // Llamamos a iniciarSesion del namespace GestionONG de tu interfaz.cpp
                GestionONG::iniciarSesion(db);
                break;
            case 2:
                // Llamamos a registrarUsuario del namespace GestionONG de tu interfaz.cpp
                GestionONG::registrarUsuario(db);
                break;
            case 0:
                cout << "\nGenerando reporte en " << miConfig.report_name << "..." << endl;
                
                // Llamamos directamente a tu función estática real pasándole el const char* vía .c_str()
                Config::generarReporteResumen(db, miConfig.report_name.c_str());
                
                cout << "\nGracias por usar el sistema. ¡Hasta pronto!\n" << endl;
                break;
            default:
                cout << "\n[?] La opción '" << opcion << "' no existe en el menú. Inténtalo de nuevo.\n" << endl;
                break;
        }

    } while (opcion != 0);

    cout << "Reporte guardado como: " << miConfig.report_name << endl;
    
    // Cerrar la base de datos de manera limpia al finalizar
    sqlite3_close(db);
    return 0;
}