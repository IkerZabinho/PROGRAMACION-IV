#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "estructuras.h"
#include "sqlite3.h"

// Crear tabla de usuarios
void crearTablaUsuarios(sqlite3 *db) {
    char *sql = "CREATE TABLE IF NOT EXISTS Usuarios ("
                "id_usuario INTEGER PRIMARY KEY AUTOINCREMENT,"
                "nombre TEXT,"
                "apellidos TEXT,"
                "nombre_usuario TEXT UNIQUE,"
                "contrasena TEXT,"
                "tipo INTEGER);";

    char *error = 0;
    if (sqlite3_exec(db, sql, 0, 0, &error) != SQLITE_OK) {
        printf("Error al crear tabla: %s\n", error);
        sqlite3_free(error);
    } else {
        printf("Tabla Usuarios lista.\n");
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
        printf("Usuario guardado correctamente\n");
    }
}

// Callback login
int callbackLogin(void *data, int argc, char **argv, char **colName) {
    int *encontrado = (int*)data;
    *encontrado = 1;
    return 0;
}

// Comprobar login
int comprobarLogin(sqlite3 *db, char *user, char *pass) {
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

// Funciones de menú
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
}

void menuPrincipal(sqlite3 *db) {
    int opcion;
    do {
        printf("\n=== MENÚ PRINCIPAL ===\n");
        printf("1. Iniciar sesión\n2. Registrarse\n3. Ver usuarios (solo prueba)\n4. Salir\nElige: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: iniciarSesion(db); break;
            case 2: registrarUsuario(db); break;
            case 3: mostrarUsuarios(db); break;
            case 4: printf("Saliendo...\n"); break;
            default: printf("Opción inválida.\n");
        }
    } while(opcion != 4);
}