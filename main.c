#include <stdio.h>
#include "estructuras.h"
#include "sqlite3.h"

// Declaraciones de funciones de funciones.c
void crearTablaUsuarios(sqlite3 *db);
void menuPrincipal(sqlite3 *db);

int main() {
    sqlite3 *db;

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

//gcc main.c funciones.c -o asociacion.exe -lsqlite3
//.\asociacion.exe