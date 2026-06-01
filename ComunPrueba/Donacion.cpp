#include "Clases.h"
#include <iostream>
#include <cstdio> // Para sprintf

using namespace std;
using namespace GestionONG;

// Constructores base por si acaso no los tenías definidos
Donacion::Donacion(int id, int idU, TipoDonacion t, string f)
    : id_donacion(id), id_usuario(idU), tipoDonacion(t), fecha(f) {}

Ropa::Ropa(int id, int idD, float k) : id_ropa(id), id_donacion(idD), kilos(k) {}
Dinero::Dinero(int id, int idD, float cant) : id_dinero(id), id_donacion(idD), cantidad(cant) {}
Comida::Comida(int id, TipoComida t, float k, int idD) : id_comida(id), tipo_comida(t), kilos(k), id_donacion(idD) {}


// MÉTODOS ESTÁTICOS DE GESTIÓN DE DONACIONES

int Donacion::insertarDonacionRopa(sqlite3 *db, const Ropa& r, int id_donante) {
    char sql[400];
    char *error = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // Reemplazamos el '2' por el enum ROPAD
    sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo, fecha) VALUES (%d, %d, date('now'));", 
            id_donante, (int)ROPAD);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        cout << "Error en Donaciones (Ropa): " << error << endl;
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    int id_padre = (int)sqlite3_last_insert_rowid(db);

    sprintf(sql, "INSERT INTO Ropa (id_donacion, kilos) VALUES (%d, %.2f);", 
            id_padre, r.kilos);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        cout << "Error en tabla Ropa: " << error << endl;
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return 0;
}

int Donacion::insertarDonacionDinero(sqlite3 *db, const Dinero& d, int id_donante) {
    char sql[400];
    char *error = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // Reemplazamos el '3' por el enum DINERO
    sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo, fecha) VALUES (%d, %d, date('now'));", 
            id_donante, (int)DINERO);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    int id_padre = (int)sqlite3_last_insert_rowid(db);

    sprintf(sql, "INSERT INTO Dinero (id_donacion, cantidad) VALUES (%d, %.2f);", 
            id_padre, d.cantidad);
    
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return 0;
}

int Donacion::insertarDonacionComidaDB(sqlite3 *db, const Donacion& d, const Comida& c) {
    char sql[400];
    char *error = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // Reemplazamos el '1' por el enum COMIDAD
    sprintf(sql, "INSERT INTO Donaciones (id_donante, tipo, fecha) VALUES (%d, %d, date('now'));", 
            d.id_usuario, (int)COMIDAD);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        cout << "[!] Error Donacion (Comida): " << error << endl;
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    int id_padre = (int)sqlite3_last_insert_rowid(db);

    sprintf(sql, "INSERT INTO Comida (id_donacion, tipo_comida, kilos) VALUES (%d, %d, %.2f);",
            id_padre, (int)c.tipo_comida, c.kilos);

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        cout << "[!] Error tabla Comida: " << error << endl;
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
        sqlite3_free(error);
        return -1;
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    return 0;
}