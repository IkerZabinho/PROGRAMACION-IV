#include <stdio.h>
#include <windows.h>
#include "sqlite3.h"

// Declaraciones de funciones
void crearTablas(sqlite3 *db);   // ⚠️ CAMBIO IMPORTANTE
void menuPrincipal(sqlite3 *db);

int main() {
    sqlite3 *db;

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
    crearTablas(db);

    // 
    menuPrincipal(db);

    // Cerrar base de datos
    sqlite3_close(db);

    printf("Programa finalizado\n");

    return 0;
}
////////////////////////////////////
//EJEKUTAU AHAL IZATEKO HAU JARRI TERMINALIEN

//gcc main.c funciones.c sqlite3.c -o asociacion.exe -lsqlite3
//.\asociacion.exe