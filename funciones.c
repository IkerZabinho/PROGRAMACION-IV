#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "estructuras.h"
#include "sqlite3.h"
//hola
// Crear tablas
void crearTablas(sqlite3 *db) {

    char *error = 0;

    char *sql =
    "CREATE TABLE IF NOT EXISTS Usuarios ("
    "id_usuario INTEGER PRIMARY KEY AUTOINCREMENT,"
    "nombre TEXT,"
    "apellidos TEXT,"
    "nombre_usuario TEXT UNIQUE,"
    "contrasena TEXT,"
    "tipo INTEGER);"

    "CREATE TABLE IF NOT EXISTS Donaciones ("
    "id_donacion INTEGER PRIMARY KEY AUTOINCREMENT,"
    "id_usuario INTEGER,"
    "tipo INTEGER);"

    "CREATE TABLE IF NOT EXISTS Eventos ("
    "id_evento INTEGER PRIMARY KEY AUTOINCREMENT,"
    "descripcion TEXT,"
    "tipo INTEGER,"
    "lim_voluntarios INTEGER);";

    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error al crear tablas: %s\n", error);
    } else {
        printf("Tablas listas\n");
    }
}

// Insertar usuario
void insertarUsuario(sqlite3 *db, Usuario u) {
    char sql[500];
    sprintf(sql, "INSERT INTO Usuarios (nombre, apellidos, nombre_usuario, contrasena, tipo) "
                 "VALUES ('%s', '%s', '%s', '%s', %d);",
                 u.nombre, u.apellidos, u.nombre_usuario, u.contrasena, u.tipoUsuario);

    char *error = 0;
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error al insertar usuario: %s\n", error);
        sqlite3_free(error);
    } else {
        printf("Usuario registrado y guardado correctamente\n");
    }
}

// Callback login
int callbackLogin(void *data, int argc, char **argv, char **colName) {
    int *encontrado = (int*)data;
    *encontrado = 1;
    return 0;
}

// Comprobar login
int comprobarLogin(sqlite3 *db, char *user, char *pass, int *tipo) {
    char sql[300];
    int encontrado = 0;
    sprintf(sql, "SELECT * FROM Usuarios WHERE nombre_usuario='%s' AND contrasena='%s';", user, pass);

    char *error = 0;
    sqlite3_exec(db, sql, callbackLogin, &encontrado, &error);

    return encontrado;
}

// Mostrar usuarios
int callbackMostrar(void *data, int argc, char **argv, char **colName) {
    printf("\n--- Usuario ---\n");
    for (int i = 0; i < argc; i++) {
        // Contraseña ez azaltzeko
        if (strcmp(colName[i], "contrasena") == 0) continue;
        printf("%s: %s\n", colName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

void mostrarUsuarios(sqlite3 *db) {
    char *sql = "SELECT * FROM Usuarios;";
    char *error = 0;
    if (sqlite3_exec(db, sql, callbackMostrar, 0, &error) != SQLITE_OK) {
        printf("Error al mostrar usuarios: %s\n", error);
        sqlite3_free(error);
    }
}


//DONAZIUAK
void donarDinero(sqlite3 *db) {

    int id;
    printf("Tu ID de usuario: ");
    scanf("%d", &id);

    char sql[200];

    sprintf(sql,
        "INSERT INTO Donaciones (id_usuario, tipo) VALUES (%d, 1);",
        id);

    sqlite3_exec(db, sql, 0, 0, 0);

    printf("Donación registrada\n");
}

//EVENTUAK

void crearEvento(sqlite3 *db) {
    char descripcion[100];
    int tipo, limite;

    printf("\n--- Crear Evento ---\n");
    printf("Descripción: ");
    scanf(" %[^\n]", descripcion); // Para leer con espacios

    printf("Tipo de evento (0 = Ropa, 1 = Comida): ");
    scanf("%d", &tipo);

    printf("Límite de voluntarios: ");
    scanf("%d", &limite);

    char sql[300];
    sprintf(sql,
        "INSERT INTO Eventos (descripcion, tipo, lim_voluntarios) VALUES ('%s', %d, %d);",
        descripcion, tipo, limite);

    char *error = 0;
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error al crear evento: %s\n", error);
        sqlite3_free(error);
    } else {
        printf("Evento registrado correctamente\n");
    }
}

//ROLAN ARABERA MENUA
void menuUsuario(sqlite3 *db, int tipo) {

    int opcion;

    do {
        printf("\n--- MENU USUARIO ---\n");

        if(tipo == VOLUNTARIO) {
            printf("1. Ver eventos\n");
        }
        else if(tipo == DONANTE) {
            printf("1. Donar dinero\n");
        }
        else if(tipo == BENEFICIARIO) {
            printf("1. Ver ayudas\n");
        }

        printf("0. Salir\n");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                if(tipo == VOLUNTARIO) {
                    printf("Hemen funcion para ver eventos(iteko dao oaindik)\n");
                } else if(tipo == DONANTE) {
                    donarDinero(db);
                } else if(tipo == BENEFICIARIO) {
                    printf("Hemen funcion para ver ayudas(iteko dao oaindik)\n");
                }
                break;
        }

    } while(opcion != 0);
}

// Funciones de menú
//HAU ZABANA GEHIXKI BADAO NERIA ZABANA JARRI BERRIZ!!
/*
void iniciarSesion(sqlite3 *db) {
    char user[50], pass[50];
    printf("\n--- INICIAR SESIÓN ---\n");
    printf("Usuario: ");
    scanf("%s", user);
    printf("Contraseña: ");
    scanf("%s", pass);

    if (comprobarLogin(db, user, pass)) {
        printf("\n¡Bienvenido %s!\n", user);
    } else {
        printf("\nError: Usuario o contraseña incorrectos.\n");
    }
}
    */

    //INICIAR SESION 2.0
void iniciarSesion(sqlite3 *db) {

    char user[50], pass[50];
    int tipo;

    printf("\nUsuario: ");
    scanf("%s", user);

    printf("Contraseña: ");
    scanf("%s", pass);

    if (comprobarLogin(db, user, pass, &tipo)) {
        printf("Bienvenido!\n");
        menuUsuario(db, tipo);
    } else {
        printf("Error login\n");
    }
}


void registrarUsuario(sqlite3 *db) {
    Usuario nuevoUsuario;
    char buffer[50];
    int rolElegido;

    printf("\n--- REGISTRO DE NUEVO USUARIO ---\n");
    printf("Elige tu rol:\n1. Voluntario\n2. Donante\n3. Beneficiario\nRol (1-3): ");
    scanf("%d", &rolElegido);
    nuevoUsuario.tipoUsuario = (TipoUsuario)(rolElegido - 1);

    printf("Nombre: "); scanf("%s", buffer); nuevoUsuario.nombre = strdup(buffer);
    printf("Apellidos: "); scanf("%s", buffer); nuevoUsuario.apellidos = strdup(buffer);
    printf("Nombre de usuario: "); scanf("%s", buffer); nuevoUsuario.nombre_usuario = strdup(buffer);
    printf("Contraseña: "); scanf("%s", buffer); nuevoUsuario.contrasena = strdup(buffer);

    insertarUsuario(db, nuevoUsuario);
    printf("Registro completado. Accediendo al menú...\n");
    menuUsuario(db, nuevoUsuario.tipoUsuario);
}

void menuPrincipal(sqlite3 *db) {//Zaba aldau indot pixkat eventua sortu ahal izateko

    int opcion;

    do {
        printf("\n=== MENU ===\n");
        printf("1. Login\n2. Registro\n3. Ver usuarios\n4. Crear evento\n5. Salir\n");

        scanf("%d", &opcion);

        switch(opcion) {
            case 1: iniciarSesion(db); break;
            case 2: registrarUsuario(db); break;
            case 3: mostrarUsuarios(db); break;
            case 4: crearEvento(db); break;
        }

    } while(opcion != 5);
}