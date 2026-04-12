#include <stdio.h>
#include <windows.h>
#include "sqlite3.h"
#include "funciones.h"
#include "estructuras.h"

// Declaraciones de funciones
//void crearTablas(sqlite3 *db);   //  CAMBIO IMPORTANTE
//void menuPrincipal(sqlite3 *db, int tipo);

int main() {
    sqlite3 *db;
    int tipo = 0;

    // UTF-8 para ñ y tildes
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // Abrir base de datos
    if (sqlite3_open("asociacion.db", &db) != SQLITE_OK) {
        printf("Error al abrir la base de datos\n");
        return 1;
    }

    printf("Base de datos abierta correctamente\n");

    //Crear Tablas bailña danak
    //crearTablas(db);

    // 
    //menuPrincipal(db, tipo);
    //MENU
    int opcion;
    do {
        printf("\n=== BIENVENIDO ===\n");
        printf("1. Iniciar Sesión\n");
        printf("2. Registrarse\n");
        printf("0. Salir\n");
        printf("Seleccione: ");
        if (scanf("%d", &opcion) != 1) {
            printf("[!] Error: Introduce un número válido.\n");
            while (getchar() != '\n'); // Limpiar el búfer para evitar bucle infinito
            opcion = -1; // Forzamos que continúe el bucle
            continue;
        }

      switch (opcion) {
            case 1:
                iniciarSesion(db);
                break;
            case 2:
                registrarUsuario(db);
                break;
            case 0:
                printf("Saliendo del programa...\n");
                break;
            default:
                printf("[!] Error: '%d' no es una opción válida.\n", opcion);
                break;
        }

    } while (opcion != 0);
    // Cerrar base de datos
    sqlite3_close(db);

    printf("Programa finalizado\n");

    return 0;
}
////////////////////////////////////
//EJEKUTAU AHAL IZATEKO HAU JARRI TERMINALIEN

//gcc main.c funciones.c sqlite3.c -o asociacion.exe -lsqlite3
//.\asociacion.exe