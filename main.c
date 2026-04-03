#include <stdio.h>
#include "estructuras.h"
#include "sqlite3.h"
#include <windows.h>

// Declaraciones de funciones de funciones.c
void crearTablaUsuarios(sqlite3 *db);
void menuPrincipal(sqlite3 *db);

int main() {
    sqlite3 *db;

    // Establece la consola en UTF-8 (65001 es el código de UTF-8) para poder visualizar bien las ñ y tildes
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    if (sqlite3_open("asociacion.db", &db) != SQLITE_OK) {
        printf("Error al abrir la base de datos\n");
        return 1;
    }

    printf("Base de datos abierta correctamente\n");

    crearTablaUsuarios(db);
    menuPrincipal(db);

    sqlite3_close(db);
    return 0;
}
////////////////////////////////////
//EJEKUTAU AHAL IZATEKO HAU JARRI TERMINALIEN

//gcc main.c funciones.c sqlite3.c -o asociacion.exe -lsqlite3
//.\asociacion.exe